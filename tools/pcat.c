/* pcat.c — print a Windows Mobile file to stdout */
#include "pcommon.h"
#include <rapi2.h>
#include <synce_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE (64 * 1024)

static char *dev_name = NULL;

static void show_usage(const char *name)
{
    fprintf(stderr,
        "Usage: %s [-p DEVNAME] :REMOTE_PATH\n"
        "\n"
        "  Print the contents of a Windows Mobile file to stdout.\n"
        "  The path must be prefixed with ':' to indicate a remote file.\n"
        "\n"
        "  -p DEVNAME   Device name (default: first connected device)\n"
        "\n"
        "Example:\n"
        "  %s \":/My Documents/notes.txt\"\n"
        "  %s :/Windows/system.ini | grep -i processor\n",
        name, name, name);
}

int main(int argc, char **argv)
{
    int result = 1;
    IRAPIDesktop    *desktop = NULL;
    IRAPIEnumDevices *enumdev = NULL;
    IRAPIDevice     *device  = NULL;
    IRAPISession    *session = NULL;
    RAPI_DEVICEINFO  devinfo;
    AnyFile         *src     = NULL;
    unsigned char   *buf     = NULL;
    HRESULT hr;
    const char *path = NULL;

    int c;
    while ((c = getopt(argc, argv, "p:h")) != -1) {
        switch (c) {
        case 'p': dev_name = optarg; break;
        case 'h': show_usage(argv[0]); return 0;
        default:  show_usage(argv[0]); return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "%s: missing remote path\n\n", argv[0]);
        show_usage(argv[0]);
        return 1;
    }
    path = argv[optind];
    if (path[0] != ':') {
        fprintf(stderr, "%s: path must start with ':' for a remote file\n", argv[0]);
        return 1;
    }

    if (FAILED(hr = IRAPIDesktop_Get(&desktop))) {
        fprintf(stderr, "%s: failed to initialise RAPI: %08x\n", argv[0], hr);
        goto exit;
    }
    if (FAILED(hr = IRAPIDesktop_EnumDevices(desktop, &enumdev))) {
        fprintf(stderr, "%s: failed to enumerate devices: %08x\n", argv[0], hr);
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
        fprintf(stderr, "%s: device not found: %08x\n", argv[0], hr);
        goto exit;
    }

    IRAPIDevice_AddRef(device);

    if (FAILED(hr = IRAPIDevice_CreateSession(device, &session))) {
        fprintf(stderr, "%s: could not create session: %08x\n", argv[0], hr);
        goto exit;
    }
    if (FAILED(hr = IRAPISession_CeRapiInit(session))) {
        fprintf(stderr, "%s: CeRapiInit failed: %08x\n", argv[0], hr);
        goto exit;
    }

    src = anyfile_open(path, READ, session);
    if (!src) {
        fprintf(stderr, "%s: cannot open %s\n", argv[0], path);
        goto exit;
    }

    buf = malloc(BUFSIZE);
    if (!buf) {
        fprintf(stderr, "%s: out of memory\n", argv[0]);
        goto exit;
    }

    for (;;) {
        size_t nread = 0;
        if (!anyfile_read(src, buf, BUFSIZE, &nread)) {
            fprintf(stderr, "%s: read error\n", argv[0]);
            goto exit;
        }
        if (nread == 0) break;
        size_t off = 0;
        while (off < nread) {
            ssize_t n = write(STDOUT_FILENO, buf + off, nread - off);
            if (n < 0) {
                perror("write");
                goto exit;
            }
            off += (size_t)n;
        }
    }

    result = 0;

exit:
    if (buf) free(buf);
    if (src) { anyfile_close(src); free(src); }
    if (session) {
        IRAPISession_CeRapiUninit(session);
        IRAPISession_Release(session);
    }
    if (device)  IRAPIDevice_Release(device);
    if (desktop) IRAPIDesktop_Release(desktop);
    return result;
}
