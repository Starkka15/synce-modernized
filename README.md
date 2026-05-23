# SynCE Modernized

A modernized fork of [SynCE](https://sourceforge.net/projects/synce/) — Linux connectivity for Windows Mobile / Windows CE devices.

Based on synce-core 0.17 (2013). Updated to build clean on modern Linux (Ubuntu 22.04+, GCC 12+).

## What's included

- **libsynce** — RAPI protocol library for communicating with WM/CE devices
- **dccm** — connection manager daemon (handles USB device detection via udev)
- **CLI tools** — `pls`, `pcp`, `prm`, `pmkdir`, `pmv`, `prun`, `pstatus`, `pcat`, `pbackup` and more
- **synce-fuse** — FUSE filesystem: mount your device as `~/Axim` (or any directory)

## Changes from upstream

- Fixed `inline` → `static inline` for `isKeyValSep`/`isCommentStart` (GCC C99 semantics)
- RAPI chunk size 1024 → 4096 bytes, watchdog interval 3 s → 10 s (reduces USB corruption)
- Added `pcat` — print device file to stdout
- Added `pbackup` — recursive device backup to a local directory
- Added `statfs` support to synce-fuse (shows device free space in `df`, file managers)
- Added operation logging to synce-fuse (open/close/mkdir/unlink/rename go to stderr)
- udev rule for auto-mount and auto-unmount on plug/unplug

## Dependencies

```
sudo apt install build-essential autoconf automake libtool pkg-config \
    libglib2.0-dev libdbus-1-dev libusb-1.0-0-dev libudev-dev \
    libfuse3-dev fuse3 \
    isc-dhcp-client net-tools ppp
```

## Build

```bash
autoreconf -fi   # if building from git
./configure --prefix=/usr/local
make -j$(nproc)
sudo make install
sudo ldconfig
```

## Device setup

Connect your Windows Mobile device via USB. The `dccm` daemon handles the initial RAPI handshake.

### Auto-mount via udev

The udev rule at `/etc/udev/rules.d/85-synce.rules` automatically mounts the device at `~/Axim` on plug-in and unmounts on unplug. `synce-fuse` runs in the background; the watchdog kills it when the device disconnects.

To install the udev rule:

```bash
sudo cp etc/udev/rules.d/85-synce.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
```

### Manual mount

```bash
mkdir -p ~/Axim
synce-fuse ~/Axim          # blocks in foreground; Ctrl-C to unmount
# or background:
synce-fuse ~/Axim -f &
```

## CLI tools

```bash
pstatus                                           # show connected device info
pls /                                             # list device root
pls "/My Documents"                               # list a directory
pcp localfile.txt ":/My Documents/localfile.txt"  # copy local → device
pcp ":localfile.txt" ./localfile.txt              # copy device → local
pcat ":/My Documents/notes.txt"                   # print device file to stdout
prun //Windows/calc.exe                           # run program on device
prm ":/My Documents/old.txt"                      # delete device file
pbackup ./device-backup/                          # full device backup to local dir
pbackup ./device-backup/ -p "/My Documents"       # backup a specific path only
```

## Backup

`pbackup` mirrors the device filesystem to a local directory. On subsequent runs it overwrites files that already exist, so it can be used for incremental-style snapshots by pointing it at a timestamped directory:

```bash
pbackup ~/axim-backups/$(date +%Y-%m-%d)/
```

## License

MIT — same as upstream synce-core.
