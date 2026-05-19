# SynCE Modernized

A modernized fork of [SynCE](https://sourceforge.net/projects/synce/) — Linux connectivity for Windows Mobile / Windows CE devices.

Based on synce-core 0.17 (2013). Updated to build clean on modern Linux (Ubuntu 22.04+, GCC 12+).

## What's included

- **libsynce** — RAPI protocol library for communicating with WM/CE devices
- **dccm** — connection manager daemon (handles USB device detection via udev)
- **CLI tools** — `pls`, `pcp`, `prm`, `pmkdir`, `pmv`, `prun`, `pstatus` and more
- **synce-fuse** *(planned)* — FUSE filesystem mount replacing the old gvfs plugin

## Changes from upstream

- Fixed `inline` → `static inline` for `isKeyValSep`/`isCommentStart` (GCC C99 semantics)
- *(more fixes as modernization continues)*

## Dependencies

```
sudo apt install build-essential autoconf automake libtool pkg-config \
    libglib2.0-dev libdbus-1-dev libusb-1.0-0-dev libudev-dev \
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

Connect your Windows Mobile device via USB. The `dccm` daemon handles the initial
RAPI handshake. Then use the CLI tools:

```bash
pstatus          # show connected device info
pls /            # list root of device filesystem
pcp localfile.txt "/My Documents/localfile.txt"   # copy to device
```

## License

MIT — same as upstream synce-core.
