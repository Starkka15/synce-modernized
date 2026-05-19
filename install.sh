#!/usr/bin/env bash
# install.sh — synce-modernized full system installer
# Fixes all issues needed for modern Linux (Ubuntu 22.04+/kernel 6.x)
set -euo pipefail

if [ "$EUID" -ne 0 ]; then
    echo "Run as root: sudo ./install.sh"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PREFIX=/usr/local

echo "=== synce-modernized installer ==="
echo "Source: $SCRIPT_DIR"
echo ""

# ── 1. Build dependencies ─────────────────────────────────────────────────────
echo "[1/9] Installing build dependencies..."
apt-get install -y \
    build-essential dkms \
    libglib2.0-dev libdbus-glib-1-dev \
    libfuse3-dev \
    python3 python3-dbus \
    isc-dhcp-client \
    udev systemd >/dev/null

# ── 2. Build synce-core ───────────────────────────────────────────────────────
echo "[2/8] Building synce-core..."
cd "$SCRIPT_DIR"
if [ ! -f config.h ]; then
    ./configure --prefix="$PREFIX"
fi
make -j"$(nproc)"
make install
cd "$SCRIPT_DIR"

# ── 3. Build and install synce-fuse ──────────────────────────────────────────
echo "[3/9] Building synce-fuse..."
cd "$SCRIPT_DIR/fuse"
make -j"$(nproc)"
make install
cd "$SCRIPT_DIR"

# ── 4. Install Python 3 scripts ───────────────────────────────────────────────
echo "[4/9] Installing Python 3 scripts..."
install -m 644 "$SCRIPT_DIR/scripts/synceconnector.py" \
    "$PREFIX/share/synce-core/synceconnector.py"
install -m 755 "$SCRIPT_DIR/scripts/udev-synce-rndis" \
    /usr/lib/udev/synce-udev-rndis

# ── 5. Install udev rules ─────────────────────────────────────────────────────
echo "[5/9] Installing udev rules..."
install -m 644 "$SCRIPT_DIR/scripts/85-synce.rules" \
    /etc/udev/rules.d/85-synce.rules
udevadm control --reload-rules

# ── 6. Install D-Bus files ────────────────────────────────────────────────────
echo "[6/9] Installing D-Bus policy and service files..."
install -m 644 "$SCRIPT_DIR/etc/org.synce.dccm.conf" \
    /etc/dbus-1/system.d/org.synce.dccm.conf
install -m 644 "$SCRIPT_DIR/etc/org.synce.dccm.service" \
    /usr/share/dbus-1/system-services/org.synce.dccm.service

# ── 7. DKMS: patched rndis_host (fixes Dell Axim X50/X51 EAGAIN on USB init) ─
echo "[7/9] Installing rndis-activesync DKMS module..."
DKMS_SRC="/usr/src/rndis-activesync-1.0"
# Remove any previous install/source
dkms remove -m rndis-activesync -v 1.0 --all 2>/dev/null || true
rm -rf "$DKMS_SRC"
# Copy only the source files (not build artifacts)
mkdir -p "$DKMS_SRC"
install -m 644 "$SCRIPT_DIR/kmod/rndis-activesync/rndis_host.c" "$DKMS_SRC/"
install -m 644 "$SCRIPT_DIR/kmod/rndis-activesync/Makefile"     "$DKMS_SRC/"
install -m 644 "$SCRIPT_DIR/kmod/rndis-activesync/dkms.conf"    "$DKMS_SRC/"
dkms add     -m rndis-activesync -v 1.0
dkms build   -m rndis-activesync -v 1.0
dkms install -m rndis-activesync -v 1.0
# Reload the module (harmless if no device is connected)
modprobe -r rndis_host 2>/dev/null || true
modprobe    rndis_host

# ── 8. AppArmor fix: allow dhclient to access synce paths ────────────────────
echo "[8/9] Patching AppArmor dhclient profile..."
APPARMOR_LOCAL="/etc/apparmor.d/local/sbin.dhclient"
APPARMOR_MAIN="/etc/apparmor.d/sbin.dhclient"
if [ -f "$APPARMOR_MAIN" ]; then
    if ! grep -q "synce-modernized" "$APPARMOR_LOCAL" 2>/dev/null; then
        cat >> "$APPARMOR_LOCAL" << 'EOF'

# synce-modernized: dhclient needs access to synce config and lease files
/usr/local/share/synce-core/dhclient.conf r,
/usr/local/var/run/dhclient-synce-* lrw,
EOF
    fi
    apparmor_parser -r "$APPARMOR_MAIN"
else
    echo "  AppArmor not active, skipping"
fi

# ── 9. Runtime directory (socket + dhclient lease files) ─────────────────────
echo "[9/9] Creating runtime directories and systemd service..."
mkdir -p "$PREFIX/var/run/synce"
chmod 777 "$PREFIX/var/run/synce"
if [ ! -f /etc/tmpfiles.d/synce.conf ]; then
    echo "d $PREFIX/var/run/synce 0777 root root -" \
        > /etc/tmpfiles.d/synce.conf
fi

# ── 10. dccm systemd service (auto-start + clean restart on disconnect) ──────
install -m 644 "$SCRIPT_DIR/etc/synce-dccm.service" \
    /etc/systemd/system/synce-dccm.service
systemctl daemon-reload
systemctl enable synce-dccm
systemctl restart synce-dccm

echo ""
echo "=== Installation complete ==="
echo ""
echo "Connect your Windows Mobile device via USB — it will connect automatically."
echo "Run 'pstatus' to verify the connection."
