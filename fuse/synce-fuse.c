/*
 * synce-fuse.c — FUSE filesystem for Windows Mobile devices via RAPI
 *
 * Mounts a connected WM device as a directory so file managers
 * (Nautilus, Thunar, etc.) can browse and transfer files.
 *
 * Usage: synce-fuse <mountpoint> [-d DEVNAME]
 *
 * Paths are UTF-8 on the FUSE side, converted to/from WCHAR for RAPI.
 * Windows backslash separators are handled transparently.
 */

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include <synce.h>
#include <rapi2.h>

/* ── Globals ──────────────────────────────────────────────────────────────── */

static IRAPIDesktop  *g_desktop  = NULL;
static IRAPIDevice   *g_device   = NULL;
static IRAPISession  *g_session  = NULL;
static char          *g_devname  = NULL;
static pthread_mutex_t g_lock    = PTHREAD_MUTEX_INITIALIZER;

/* ── Path conversion ──────────────────────────────────────────────────────── */

/* FUSE path "/My Documents/foo.txt" → WM path "\\My Documents\\foo.txt" */
static WCHAR *fuse_to_wm_path(const char *fuse_path)
{
    /* Replace forward slashes with backslashes, prepend nothing (root = "\\") */
    char *tmp = strdup(fuse_path);
    if (!tmp) return NULL;
    for (char *p = tmp; *p; p++)
        if (*p == '/') *p = '\\';
    WCHAR *wpath = wstr_from_utf8(tmp);
    free(tmp);
    return wpath;
}

/* ── Time conversion ──────────────────────────────────────────────────────── */

/* FILETIME (100-ns intervals since 1601-01-01) → Unix time_t */
static time_t filetime_to_unix(FILETIME ft)
{
    uint64_t t = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    /* Offset between 1601-01-01 and 1970-01-01 in 100-ns units */
    t -= (uint64_t)116444736000000000ULL;
    return (time_t)(t / 10000000ULL);
}

static FILETIME unix_to_filetime(time_t t)
{
    uint64_t ft = (uint64_t)t * 10000000ULL + 116444736000000000ULL;
    FILETIME result;
    result.dwLowDateTime  = (DWORD)(ft & 0xFFFFFFFF);
    result.dwHighDateTime = (DWORD)(ft >> 32);
    return result;
}

/* ── FUSE operations ──────────────────────────────────────────────────────── */

static int sf_getattr(const char *path, struct stat *st, struct fuse_file_info *fi)
{
    (void)fi;
    memset(st, 0, sizeof(*st));

    /* Root directory */
    if (strcmp(path, "/") == 0) {
        st->st_mode  = S_IFDIR | 0755;
        st->st_nlink = 2;
        return 0;
    }

    WCHAR *wpath = fuse_to_wm_path(path);
    if (!wpath) return -ENOMEM;

    pthread_mutex_lock(&g_lock);

    DWORD attrs = IRAPISession_CeGetFileAttributes(g_session, wpath);

    if (attrs == (DWORD)-1) {
        free(wpath);
        pthread_mutex_unlock(&g_lock);
        return -ENOENT;
    }

    if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
        st->st_mode  = S_IFDIR | 0755;
        st->st_nlink = 2;
    } else {
        st->st_mode  = S_IFREG | 0644;
        st->st_nlink = 1;

        /* Open file to get size and timestamps */
        HANDLE hf = IRAPISession_CeCreateFile(g_session, wpath,
                        GENERIC_READ, 0, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (hf != INVALID_HANDLE_VALUE && hf != 0) {
            DWORD size_hi = 0;
            DWORD size_lo = IRAPISession_CeGetFileSize(g_session, hf, &size_hi);
            st->st_size = ((off_t)size_hi << 32) | size_lo;

            FILETIME ctime, atime, mtime;
            if (IRAPISession_CeGetFileTime(g_session, hf, &ctime, &atime, &mtime)) {
                st->st_ctime = filetime_to_unix(ctime);
                st->st_atime = filetime_to_unix(atime);
                st->st_mtime = filetime_to_unix(mtime);
            }
            IRAPISession_CeCloseHandle(g_session, hf);
        }
    }

    free(wpath);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int sf_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi,
                      enum fuse_readdir_flags flags)
{
    (void)offset; (void)fi; (void)flags;

    filler(buf, ".",  NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    /* Build wildcard path: "/My Documents" → "\\My Documents\\*.*" */
    size_t plen = strlen(path);
    char *wc_path = malloc(plen + 5);
    if (!wc_path) return -ENOMEM;
    memcpy(wc_path, path, plen);
    /* append \*.* */
    if (plen == 1 && path[0] == '/') {
        strcpy(wc_path, "/*.*");
    } else {
        strcpy(wc_path + plen, "/*.*");
    }

    WCHAR *wwc = fuse_to_wm_path(wc_path);
    free(wc_path);
    if (!wwc) return -ENOMEM;

    pthread_mutex_lock(&g_lock);

    DWORD count = 0;
    CE_FIND_DATA *fdata = NULL;
    BOOL ok = IRAPISession_CeFindAllFiles(g_session, wwc,
                  FAF_ATTRIBUTES | FAF_NAME | FAF_SIZE_LOW | FAF_SIZE_HIGH |
                  FAF_LASTWRITE_TIME | FAF_CREATION_TIME,
                  &count, &fdata);
    free(wwc);

    if (!ok) {
        pthread_mutex_unlock(&g_lock);
        return -EIO;
    }

    for (DWORD i = 0; i < count; i++) {
        char *name = wstr_to_utf8(fdata[i].cFileName);
        if (!name) continue;

        struct stat st;
        memset(&st, 0, sizeof(st));
        if (fdata[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            st.st_mode = S_IFDIR | 0755;
        } else {
            st.st_mode  = S_IFREG | 0644;
            st.st_size  = ((off_t)fdata[i].nFileSizeHigh << 32) | fdata[i].nFileSizeLow;
            st.st_mtime = filetime_to_unix(fdata[i].ftLastWriteTime);
        }
        filler(buf, name, &st, 0, 0);
        free(name);
    }

    IRAPISession_CeRapiFreeBuffer(g_session, fdata);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

/* Open: store the CE file handle in fi->fh */
static int sf_open(const char *path, struct fuse_file_info *fi)
{
    WCHAR *wpath = fuse_to_wm_path(path);
    if (!wpath) return -ENOMEM;

    DWORD access = 0;
    DWORD share  = FILE_SHARE_READ;
    DWORD creat  = OPEN_EXISTING;

    if ((fi->flags & O_ACCMODE) == O_RDONLY) {
        access = GENERIC_READ;
        share  = 0;
    } else if ((fi->flags & O_ACCMODE) == O_WRONLY) {
        access = GENERIC_WRITE;
        share  = 0;
        creat  = OPEN_ALWAYS;
    } else {
        access = GENERIC_READ | GENERIC_WRITE;
        share  = 0;
        creat  = OPEN_ALWAYS;
    }

    if (fi->flags & O_TRUNC) creat = CREATE_ALWAYS;

    pthread_mutex_lock(&g_lock);
    HANDLE h = IRAPISession_CeCreateFile(g_session, wpath, access, share,
                                         NULL, creat, FILE_ATTRIBUTE_NORMAL, 0);
    pthread_mutex_unlock(&g_lock);
    free(wpath);

    if (h == INVALID_HANDLE_VALUE) return -ENOENT;
    fi->fh = (uint64_t)(uintptr_t)h;
    return 0;
}

static int sf_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi)
{
    (void)path;
    HANDLE h = (HANDLE)(uintptr_t)fi->fh;
    DWORD nread = 0;

    pthread_mutex_lock(&g_lock);
    /* Seek to offset */
    IRAPISession_CeSetFilePointer(g_session, h, (LONG)offset, NULL, FILE_BEGIN);
    BOOL ok = IRAPISession_CeReadFile(g_session, h, buf, (DWORD)size, &nread, NULL);
    pthread_mutex_unlock(&g_lock);

    if (!ok) return -EIO;
    return (int)nread;
}

static int sf_write(const char *path, const char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
    (void)path;
    HANDLE h = (HANDLE)(uintptr_t)fi->fh;
    DWORD nwritten = 0;

    pthread_mutex_lock(&g_lock);
    IRAPISession_CeSetFilePointer(g_session, h, (LONG)offset, NULL, FILE_BEGIN);
    BOOL ok = IRAPISession_CeWriteFile(g_session, h, buf, (DWORD)size, &nwritten, NULL);
    pthread_mutex_unlock(&g_lock);

    if (!ok) return -EIO;
    return (int)nwritten;
}

static int sf_release(const char *path, struct fuse_file_info *fi)
{
    (void)path;
    HANDLE h = (HANDLE)(uintptr_t)fi->fh;
    pthread_mutex_lock(&g_lock);
    IRAPISession_CeCloseHandle(g_session, h);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int sf_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    (void)mode;
    WCHAR *wpath = fuse_to_wm_path(path);
    if (!wpath) return -ENOMEM;

    pthread_mutex_lock(&g_lock);
    HANDLE h = IRAPISession_CeCreateFile(g_session, wpath,
                   GENERIC_READ | GENERIC_WRITE, 0, NULL,
                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    pthread_mutex_unlock(&g_lock);
    free(wpath);

    if (h == INVALID_HANDLE_VALUE) return -EIO;
    fi->fh = (uint64_t)(uintptr_t)h;
    return 0;
}

static int sf_mkdir(const char *path, mode_t mode)
{
    (void)mode;
    WCHAR *wpath = fuse_to_wm_path(path);
    if (!wpath) return -ENOMEM;

    pthread_mutex_lock(&g_lock);
    BOOL ok = IRAPISession_CeCreateDirectory(g_session, wpath, NULL);
    pthread_mutex_unlock(&g_lock);
    free(wpath);

    return ok ? 0 : -EIO;
}

static int sf_unlink(const char *path)
{
    WCHAR *wpath = fuse_to_wm_path(path);
    if (!wpath) return -ENOMEM;

    pthread_mutex_lock(&g_lock);
    BOOL ok = IRAPISession_CeDeleteFile(g_session, wpath);
    pthread_mutex_unlock(&g_lock);
    free(wpath);

    return ok ? 0 : -ENOENT;
}

static int sf_rmdir(const char *path)
{
    WCHAR *wpath = fuse_to_wm_path(path);
    if (!wpath) return -ENOMEM;

    pthread_mutex_lock(&g_lock);
    BOOL ok = IRAPISession_CeRemoveDirectory(g_session, wpath);
    pthread_mutex_unlock(&g_lock);
    free(wpath);

    return ok ? 0 : -ENOENT;
}

static int sf_rename(const char *from, const char *to, unsigned int flags)
{
    (void)flags;
    WCHAR *wfrom = fuse_to_wm_path(from);
    WCHAR *wto   = fuse_to_wm_path(to);
    if (!wfrom || !wto) { free(wfrom); free(wto); return -ENOMEM; }

    pthread_mutex_lock(&g_lock);
    /* Delete destination first if it exists (CeMoveFile won't overwrite) */
    IRAPISession_CeDeleteFile(g_session, wto);
    BOOL ok = IRAPISession_CeMoveFile(g_session, wfrom, wto);
    pthread_mutex_unlock(&g_lock);

    free(wfrom); free(wto);
    return ok ? 0 : -EIO;
}

static int sf_truncate(const char *path, off_t size, struct fuse_file_info *fi)
{
    HANDLE h;
    int opened = 0;

    if (fi && fi->fh) {
        h = (HANDLE)(uintptr_t)fi->fh;
    } else {
        WCHAR *wpath = fuse_to_wm_path(path);
        if (!wpath) return -ENOMEM;
        pthread_mutex_lock(&g_lock);
        h = IRAPISession_CeCreateFile(g_session, wpath,
                GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        pthread_mutex_unlock(&g_lock);
        free(wpath);
        if (h == INVALID_HANDLE_VALUE) return -ENOENT;
        opened = 1;
    }

    pthread_mutex_lock(&g_lock);
    IRAPISession_CeSetFilePointer(g_session, h, (LONG)size, NULL, FILE_BEGIN);
    IRAPISession_CeSetEndOfFile(g_session, h);
    pthread_mutex_unlock(&g_lock);

    if (opened) {
        pthread_mutex_lock(&g_lock);
        IRAPISession_CeCloseHandle(g_session, h);
        pthread_mutex_unlock(&g_lock);
    }
    return 0;
}

static int sf_utimens(const char *path, const struct timespec tv[2],
                      struct fuse_file_info *fi)
{
    (void)fi;
    WCHAR *wpath = fuse_to_wm_path(path);
    if (!wpath) return -ENOMEM;

    pthread_mutex_lock(&g_lock);
    HANDLE h = IRAPISession_CeCreateFile(g_session, wpath,
                   GENERIC_WRITE, FILE_SHARE_READ, NULL,
                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    pthread_mutex_unlock(&g_lock);
    free(wpath);

    if (h == INVALID_HANDLE_VALUE) return -ENOENT;

    FILETIME atime = unix_to_filetime(tv[0].tv_sec);
    FILETIME mtime = unix_to_filetime(tv[1].tv_sec);

    pthread_mutex_lock(&g_lock);
    IRAPISession_CeSetFileTime(g_session, h, NULL, &atime, &mtime);
    IRAPISession_CeCloseHandle(g_session, h);
    pthread_mutex_unlock(&g_lock);

    return 0;
}

/* ── FUSE operations table ────────────────────────────────────────────────── */

static const struct fuse_operations sf_ops = {
    .getattr  = sf_getattr,
    .readdir  = sf_readdir,
    .open     = sf_open,
    .read     = sf_read,
    .write    = sf_write,
    .release  = sf_release,
    .create   = sf_create,
    .mkdir    = sf_mkdir,
    .unlink   = sf_unlink,
    .rmdir    = sf_rmdir,
    .rename   = sf_rename,
    .truncate = sf_truncate,
    .utimens  = sf_utimens,
};

/* ── RAPI init / cleanup ──────────────────────────────────────────────────── */

static int rapi_connect(void)
{
    HRESULT hr;
    IRAPIEnumDevices *enumdev = NULL;
    RAPI_DEVICEINFO devinfo;

    if (FAILED(hr = IRAPIDesktop_Get(&g_desktop))) {
        fprintf(stderr,"synce-fuse: failed to init RAPI: %08x\n", hr);
        return -1;
    }
    if (FAILED(hr = IRAPIDesktop_EnumDevices(g_desktop, &enumdev))) {
        fprintf(stderr,"synce-fuse: failed to enumerate devices: %08x\n", hr);
        return -1;
    }
    while (SUCCEEDED(hr = IRAPIEnumDevices_Next(enumdev, &g_device))) {
        if (!g_devname) break;
        if (FAILED(IRAPIDevice_GetDeviceInfo(g_device, &devinfo))) continue;
        if (strcmp(g_devname, devinfo.bstrName) == 0) break;
    }
    IRAPIEnumDevices_Release(enumdev);

    if (FAILED(hr)) {
        fprintf(stderr,"synce-fuse: device not found\n");
        return -1;
    }
    IRAPIDevice_AddRef(g_device);

    if (FAILED(hr = IRAPIDevice_CreateSession(g_device, &g_session))) {
        fprintf(stderr,"synce-fuse: could not create session: %08x\n", hr);
        return -1;
    }
    if (FAILED(hr = IRAPISession_CeRapiInit(g_session))) {
        fprintf(stderr,"synce-fuse: CeRapiInit failed: %08x\n", hr);
        return -1;
    }
    return 0;
}

static void rapi_disconnect(void)
{
    if (g_session) {
        IRAPISession_CeRapiUninit(g_session);
        IRAPISession_Release(g_session);
        g_session = NULL;
    }
    if (g_device)  { IRAPIDevice_Release(g_device);   g_device  = NULL; }
    if (g_desktop) { IRAPIDesktop_Release(g_desktop);  g_desktop = NULL; }
}

/* ── main ─────────────────────────────────────────────────────────────────── */

static void show_usage(const char *name)
{
    fprintf(stderr,
        "Usage: %s <mountpoint> [-d DEVNAME] [FUSE options]\n"
        "\n"
        "  <mountpoint>   Directory to mount the device on\n"
        "  -d DEVNAME     Device name (default: first connected device)\n"
        "\n"
        "FUSE options (passed through):\n"
        "  -f             Run in foreground\n"
        "  -s             Single-threaded\n"
        "  -o allow_other Allow other users to access the mount\n"
        "\n"
        "Example:\n"
        "  mkdir -p ~/Axim && synce-fuse ~/Axim\n",
        name);
}

int main(int argc, char *argv[])
{
    /* Pull out our -d option before passing to fuse_main */
    struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
    fuse_opt_add_arg(&args, argv[0]);

    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            g_devname = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_usage(argv[0]);
            return 0;
        } else {
            fuse_opt_add_arg(&args, argv[i]);
        }
    }

    if (args.argc < 2) {
        show_usage(argv[0]);
        return 1;
    }

    if (rapi_connect() != 0)
        return 1;

    /* Run single-threaded: RAPI is not re-entrant, mutex protects us anyway */
    fuse_opt_add_arg(&args, "-s");

    int ret = fuse_main(args.argc, args.argv, &sf_ops, NULL);

    rapi_disconnect();
    fuse_opt_free_args(&args);
    return ret;
}
