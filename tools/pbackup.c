/* pbackup.c — recursive device backup to a local directory */
#include "pcommon.h"
#include <rapi2.h>
#include <synce_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

#define BUFSIZE (64 * 1024)

static char        *dev_name   = NULL;
static const char  *src_prefix = ":/";   /* remote root by default */
static int          verbose    = 0;
static IRAPISession *g_session = NULL;
static unsigned long g_files   = 0;
static unsigned long g_bytes   = 0;
static time_t        g_start;

static void show_usage(const char *name)
{
    fprintf(stderr,
        "Usage: %s [-p DEVNAME] [-s REMOTE_PATH] [-v] LOCAL_DIR\n"
        "\n"
        "  Recursively copy the device filesystem to LOCAL_DIR.\n"
        "\n"
        "  -p DEVNAME      Device name (default: first connected device)\n"
        "  -s REMOTE_PATH  Remote path to back up (default: :/ — full device)\n"
        "  -v              Verbose: print each file as it is copied\n"
        "\n"
        "Examples:\n"
        "  %s ~/axim-backup/\n"
        "  %s -s \":/My Documents\" ~/docs-backup/\n"
        "  %s ~/axim-backups/$(date +%%Y-%%m-%%d)/\n",
        name, name, name, name);
}

static bool ensure_local_dir(const char *dir)
{
    struct stat st;
    if (stat(dir, &st) == 0) {
        if (S_ISDIR(st.st_mode)) return true;
        fprintf(stderr, "pbackup: %s exists but is not a directory\n", dir);
        return false;
    }
    if (mkdir(dir, 0755) == 0) return true;
    if (errno == EEXIST) return true;
    fprintf(stderr, "pbackup: cannot create %s: %s\n", dir, strerror(errno));
    return false;
}

/* Copy one remote file → local path */
static bool copy_file(const char *remote, const char *local)
{
    AnyFile *src = anyfile_open(remote, READ, g_session);
    if (!src) {
        fprintf(stderr, "pbackup: cannot open %s\n", remote);
        return false;
    }

    FILE *dst = fopen(local, "wb");
    if (!dst) {
        fprintf(stderr, "pbackup: cannot create %s: %s\n", local, strerror(errno));
        anyfile_close(src); free(src);
        return false;
    }

    unsigned char *buf = malloc(BUFSIZE);
    if (!buf) { fclose(dst); anyfile_close(src); free(src); return false; }

    bool ok = true;
    for (;;) {
        size_t nread = 0;
        if (!anyfile_read(src, buf, BUFSIZE, &nread)) {
            fprintf(stderr, "pbackup: read error on %s\n", remote);
            ok = false; break;
        }
        if (nread == 0) break;
        if (fwrite(buf, 1, nread, dst) != nread) {
            fprintf(stderr, "pbackup: write error on %s: %s\n", local, strerror(errno));
            ok = false; break;
        }
        g_bytes += nread;
    }

    free(buf);
    fclose(dst);
    anyfile_close(src); free(src);

    if (ok) {
        g_files++;
        if (verbose) fprintf(stderr, "  %s\n", remote);
    } else {
        remove(local);
    }
    return ok;
}

/* Recursively back up remote_dir (e.g. ":/My Documents") into local_dir */
static bool backup_dir(const char *remote_dir, const char *local_dir)
{
    if (!ensure_local_dir(local_dir)) return false;

    /* Build wildcard: :/My Documents → :/My Documents/*.* */
    size_t rlen = strlen(remote_dir);
    char *wc = malloc(rlen + 5);
    if (!wc) return false;
    memcpy(wc, remote_dir, rlen);
    /* strip trailing slash for the glob */
    while (rlen > 0 && (wc[rlen-1] == '/' || wc[rlen-1] == '\\')) rlen--;
    wc[rlen] = '\0';
    strcat(wc, "/*.*");   /* gives :/My Documents/*.* */

    /* anyfile/pcommon uses ':' prefix for remote; convert to WCHAR for RAPI */
    const char *rapi_path = (wc[0] == ':') ? wc + 1 : wc;
    /* convert forward slashes to backslashes for RAPI */
    char *rapi_tmp = strdup(rapi_path);
    for (char *p = rapi_tmp; *p; p++) if (*p == '/') *p = '\\';
    WCHAR *wwc = wstr_from_utf8(rapi_tmp);
    free(rapi_tmp); free(wc);
    if (!wwc) return false;

    DWORD count = 0;
    CE_FIND_DATA *fdata = NULL;
    BOOL found = IRAPISession_CeFindAllFiles(g_session, wwc,
                     FAF_ATTRIBUTES | FAF_NAME | FAF_SIZE_LOW | FAF_SIZE_HIGH,
                     &count, &fdata);
    free(wwc);

    if (!found) {
        /* Empty dir or access denied — not fatal */
        return true;
    }

    bool ok = true;
    for (DWORD i = 0; i < count; i++) {
        char *name = wstr_to_utf8(fdata[i].cFileName);
        if (!name) continue;

        /* remote sub-path */
        size_t plen = strlen(remote_dir);
        /* strip trailing slash */
        while (plen > 0 && (remote_dir[plen-1] == '/' || remote_dir[plen-1] == '\\')) plen--;
        char *rpath = malloc(plen + 1 + strlen(name) + 1);
        memcpy(rpath, remote_dir, plen);
        rpath[plen] = '/';
        strcpy(rpath + plen + 1, name);

        /* local sub-path */
        char *lpath = malloc(strlen(local_dir) + 1 + strlen(name) + 1);
        sprintf(lpath, "%s/%s", local_dir, name);

        if (fdata[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            ok = backup_dir(rpath, lpath) && ok;
        } else {
            ok = copy_file(rpath, lpath) && ok;
        }

        free(rpath); free(lpath); free(name);
    }

    IRAPISession_CeRapiFreeBuffer(g_session, fdata);
    return ok;
}

int main(int argc, char **argv)
{
    int result = 1;
    IRAPIDesktop     *desktop = NULL;
    IRAPIEnumDevices *enumdev = NULL;
    IRAPIDevice      *device  = NULL;
    RAPI_DEVICEINFO   devinfo;
    HRESULT hr;
    const char *local_dir = NULL;

    int c;
    while ((c = getopt(argc, argv, "p:s:vh")) != -1) {
        switch (c) {
        case 'p': dev_name   = optarg;     break;
        case 's': src_prefix = optarg;     break;
        case 'v': verbose    = 1;          break;
        case 'h': show_usage(argv[0]);     return 0;
        default:  show_usage(argv[0]);     return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "pbackup: missing LOCAL_DIR\n\n");
        show_usage(argv[0]);
        return 1;
    }
    local_dir = argv[optind];

    /* Normalise src_prefix: ensure it starts with ':' */
    if (src_prefix[0] != ':') {
        fprintf(stderr, "pbackup: -s path must start with ':'\n");
        return 1;
    }

    if (FAILED(hr = IRAPIDesktop_Get(&desktop))) {
        fprintf(stderr, "pbackup: failed to initialise RAPI: %08x\n", hr);
        goto exit;
    }
    if (FAILED(hr = IRAPIDesktop_EnumDevices(desktop, &enumdev))) {
        fprintf(stderr, "pbackup: failed to enumerate devices: %08x\n", hr);
        goto exit;
    }
    while (SUCCEEDED(hr = IRAPIEnumDevices_Next(enumdev, &device))) {
        if (!dev_name) break;
        if (FAILED(IRAPIDevice_GetDeviceInfo(device, &devinfo))) continue;
        if (strcmp(dev_name, devinfo.bstrName) == 0) break;
    }
    IRAPIEnumDevices_Release(enumdev);
    enumdev = NULL;

    if (FAILED(hr)) {
        fprintf(stderr, "pbackup: device not found: %08x\n", hr);
        goto exit;
    }

    IRAPIDevice_AddRef(device);

    if (FAILED(hr = IRAPIDevice_CreateSession(device, &g_session))) {
        fprintf(stderr, "pbackup: could not create session: %08x\n", hr);
        goto exit;
    }
    if (FAILED(hr = IRAPISession_CeRapiInit(g_session))) {
        fprintf(stderr, "pbackup: CeRapiInit failed: %08x\n", hr);
        goto exit;
    }

    g_start = time(NULL);
    fprintf(stderr, "pbackup: backing up %s → %s\n", src_prefix, local_dir);

    if (!backup_dir(src_prefix, local_dir)) {
        fprintf(stderr, "pbackup: backup completed with errors\n");
        /* still report partial success */
    }

    time_t elapsed = time(NULL) - g_start;
    if (elapsed < 1) elapsed = 1;
    fprintf(stderr, "pbackup: done — %lu files, %lu bytes in %lds (%lu B/s)\n",
            g_files, g_bytes, (long)elapsed, g_bytes / (unsigned long)elapsed);

    result = 0;

exit:
    if (g_session) {
        IRAPISession_CeRapiUninit(g_session);
        IRAPISession_Release(g_session);
    }
    if (device)  IRAPIDevice_Release(device);
    if (desktop) IRAPIDesktop_Release(desktop);
    return result;
}
