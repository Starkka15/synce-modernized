#!/usr/bin/env python3
"""SynCE GUI — system tray tool for Dell Axim X50 / Windows Mobile 6.5"""

import gi
gi.require_version('Gtk', '3.0')
gi.require_version('AyatanaAppIndicator3', '0.1')
from gi.repository import Gtk, GLib, Gdk, AyatanaAppIndicator3 as AppIndicator3

import os
import subprocess
import threading

def _fmt_size(n):
    if n >= 1024**3: return f'{n/1024**3:.1f} GB'
    if n >= 1024**2: return f'{n/1024**2:.0f} MB'
    return f'{n/1024:.0f} KB'

ICON_DIR    = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'icons')
ICON_CONN   = os.path.join(ICON_DIR, 'synce-connected.svg')
ICON_DISC   = os.path.join(ICON_DIR, 'synce-disconnected.svg')
AXIM_MOUNT  = '/home/starkka15/Axim'
POLL_MS     = 2000


# ── Helpers ────────────────────────────────────────────────────────────────────

def is_connected():
    try:
        with open('/proc/mounts') as f:
            return any('synce-fuse' in line for line in f)
    except OSError:
        return False

def run_tool(cmd, callback=None):
    """Run command in thread; call callback(success, output) on main thread."""
    def _worker():
        try:
            r = subprocess.run(cmd, capture_output=True, text=True, timeout=15)
            ok  = r.returncode == 0
            out = (r.stdout + r.stderr).strip()
        except subprocess.TimeoutExpired:
            ok, out = False, 'Timed out'
        except Exception as e:
            ok, out = False, str(e)
        if callback:
            GLib.idle_add(callback, ok, out)
    threading.Thread(target=_worker, daemon=True).start()

def show_dialog(parent, title, msg, msg_type=Gtk.MessageType.INFO):
    d = Gtk.MessageDialog(transient_for=parent, modal=True,
                          message_type=msg_type,
                          buttons=Gtk.ButtonsType.OK, text=title)
    d.format_secondary_text(msg)
    d.run()
    d.destroy()


# ── Programs window ────────────────────────────────────────────────────────────

class ProgramsWindow(Gtk.Window):
    def __init__(self):
        super().__init__(title='Installed Programs — Axim X50')
        self.set_default_size(380, 400)
        self.set_border_width(8)

        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        self.add(vbox)

        # Toolbar
        hbox = Gtk.Box(spacing=6)
        vbox.pack_start(hbox, False, False, 0)

        self.lbl_status = Gtk.Label(label='')
        self.lbl_status.set_halign(Gtk.Align.START)
        hbox.pack_start(self.lbl_status, True, True, 0)

        btn_refresh = Gtk.Button(label='Refresh')
        btn_refresh.connect('clicked', self._on_refresh)
        hbox.pack_start(btn_refresh, False, False, 0)

        self.btn_uninstall = Gtk.Button(label='Uninstall')
        self.btn_uninstall.set_sensitive(False)
        self.btn_uninstall.get_style_context().add_class('destructive-action')
        self.btn_uninstall.connect('clicked', self._on_uninstall)
        hbox.pack_start(self.btn_uninstall, False, False, 0)

        # List
        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        vbox.pack_start(sw, True, True, 0)

        self.store = Gtk.ListStore(str)
        self.tree  = Gtk.TreeView(model=self.store)
        self.tree.get_selection().connect('changed', self._on_sel)
        col = Gtk.TreeViewColumn('Program Name',
                                 Gtk.CellRendererText(), text=0)
        col.set_sort_column_id(0)
        self.tree.append_column(col)
        sw.add(self.tree)

        self.show_all()
        self._refresh()

    def _refresh(self):
        self.store.clear()
        self.lbl_status.set_text('Loading…')
        run_tool(['synce-list-programs'], self._on_list_done)

    def _on_refresh(self, _btn):
        self._refresh()

    def _on_list_done(self, ok, output):
        self.store.clear()
        if ok and output:
            for name in sorted(output.splitlines()):
                name = name.strip()
                if name:
                    self.store.append([name])
            self.lbl_status.set_text(f'{len(self.store)} program(s)')
        else:
            self.lbl_status.set_text('Failed to list programs')

    def _on_sel(self, sel):
        model, it = sel.get_selected()
        self.btn_uninstall.set_sensitive(it is not None)

    def _on_uninstall(self, _btn):
        model, it = self.tree.get_selection().get_selected()
        if not it:
            return
        name = model[it][0]
        d = Gtk.MessageDialog(transient_for=self, modal=True,
                              message_type=Gtk.MessageType.WARNING,
                              buttons=Gtk.ButtonsType.OK_CANCEL,
                              text=f'Uninstall "{name}"?')
        d.format_secondary_text('This will remove the program from the device.')
        resp = d.run()
        d.destroy()
        if resp != Gtk.ResponseType.OK:
            return
        self.lbl_status.set_text(f'Uninstalling {name}…')
        run_tool(['synce-remove-program', name], self._on_uninstall_done)

    def _on_uninstall_done(self, ok, output):
        if ok:
            self.lbl_status.set_text('Uninstalled. Refreshing…')
            self._refresh()
        else:
            self.lbl_status.set_text('Uninstall failed')
            if 'unload.exe not found' in output or 'Settings > Remove Programs' in output:
                title = 'Remote uninstall not supported'
                msg_type = Gtk.MessageType.WARNING
            else:
                title = 'Uninstall failed'
                msg_type = Gtk.MessageType.ERROR
            show_dialog(self, title, output, msg_type)


FUSE_LOG = '/tmp/synce-fuse.log'

# ── FUSE log window ────────────────────────────────────────────────────────────

class FuseLogWindow(Gtk.Window):
    def __init__(self):
        super().__init__(title='synce-fuse Log')
        self.set_default_size(700, 400)
        self.set_border_width(6)
        self._timeout_id = None

        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        self.add(vbox)

        hbox = Gtk.Box(spacing=6)
        vbox.pack_start(hbox, False, False, 0)

        self.lbl = Gtk.Label(label=FUSE_LOG)
        self.lbl.set_halign(Gtk.Align.START)
        hbox.pack_start(self.lbl, True, True, 0)

        btn_clear = Gtk.Button(label='Clear')
        btn_clear.connect('clicked', self._on_clear)
        hbox.pack_start(btn_clear, False, False, 0)

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        vbox.pack_start(sw, True, True, 0)

        self.text = Gtk.TextView()
        self.text.set_editable(False)
        self.text.set_monospace(True)
        self.text.set_cursor_visible(False)
        self.text.set_wrap_mode(Gtk.WrapMode.CHAR)
        sw.add(self.text)

        self.show_all()
        self._reload()
        self._timeout_id = GLib.timeout_add(1500, self._reload)
        self.connect('delete-event', self._on_close)

    def _reload(self):
        try:
            with open(FUSE_LOG) as f:
                content = f.read()
        except FileNotFoundError:
            content = '(log file not yet created — connect device to start logging)'
        except OSError as e:
            content = f'Error reading log: {e}'

        buf = self.text.get_buffer()
        if buf.get_text(buf.get_start_iter(), buf.get_end_iter(), False) != content:
            buf.set_text(content)
            it = buf.get_end_iter()
            self.text.scroll_to_iter(it, 0.0, False, 0.0, 1.0)

        return True  # keep timer running

    def _on_clear(self, _btn):
        try:
            open(FUSE_LOG, 'w').close()
        except OSError:
            pass
        self.text.get_buffer().set_text('')

    def _on_close(self, win, _event):
        if self._timeout_id:
            GLib.source_remove(self._timeout_id)
            self._timeout_id = None
        win.hide()
        return True


# ── Device info window ─────────────────────────────────────────────────────────

class DeviceInfoWindow(Gtk.Window):
    def __init__(self):
        super().__init__(title='Device Info — Axim X50')
        self.set_default_size(340, 340)
        self.set_border_width(8)

        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        self.add(vbox)

        hbox = Gtk.Box(spacing=6)
        vbox.pack_start(hbox, False, False, 0)
        self.lbl = Gtk.Label(label='')
        self.lbl.set_halign(Gtk.Align.START)
        hbox.pack_start(self.lbl, True, True, 0)
        btn = Gtk.Button(label='Refresh')
        btn.connect('clicked', lambda _: self._load())
        hbox.pack_start(btn, False, False, 0)

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        vbox.pack_start(sw, True, True, 0)

        self.text = Gtk.TextView()
        self.text.set_editable(False)
        self.text.set_monospace(True)
        self.text.set_cursor_visible(False)
        sw.add(self.text)

        self.show_all()
        self._load()

    def _load(self):
        self.lbl.set_text('Loading…')
        self._pstatus_text = None
        self._storage_rows = None
        run_tool(['pstatus'], self._on_pstatus)
        threading.Thread(target=self._query_storage, daemon=True).start()

    def _query_storage(self):
        vols = [
            ('Internal',      AXIM_MOUNT),
            ('Storage Card',  f'{AXIM_MOUNT}/Storage Card'),
            ('CF Card',       f'{AXIM_MOUNT}/CF Card'),
        ]
        rows = []
        for label, path in vols:
            try:
                st = os.statvfs(path)
                total = st.f_blocks * st.f_frsize
                free  = st.f_bavail * st.f_frsize
                used  = total - free
                rows.append((label, total, used, free))
            except OSError:
                pass
        GLib.idle_add(self._on_storage, rows)

    def _on_pstatus(self, ok, output):
        self._pstatus_text = output if ok else f'Error:\n{output}'
        self._try_render()

    def _on_storage(self, rows):
        self._storage_rows = rows
        self._try_render()
        return False

    def _try_render(self):
        if self._pstatus_text is None or self._storage_rows is None:
            return
        text = self._pstatus_text
        if self._storage_rows:
            text += '\n\n── Storage ──────────────────────────────\n'
            for label, total, used, free in self._storage_rows:
                pct = int(100 * used / total) if total else 0
                text += (f'{label:<14}  {_fmt_size(total):>8} total'
                         f'  {_fmt_size(used):>7} used'
                         f'  {_fmt_size(free):>8} free'
                         f'  ({pct}%)\n')
        self.text.get_buffer().set_text(text)
        self.lbl.set_text('Device status')


# ── Main app ───────────────────────────────────────────────────────────────────

class SynceApp:
    def __init__(self):
        self._connected   = False
        self._prog_win    = None
        self._info_win    = None
        self._log_win     = None

        self.indicator = AppIndicator3.Indicator.new(
            'synce-gui', ICON_DISC,
            AppIndicator3.IndicatorCategory.HARDWARE)
        self.indicator.set_status(AppIndicator3.IndicatorStatus.ACTIVE)

        self._build_menu()
        self._update_state(is_connected())
        GLib.timeout_add(POLL_MS, self._poll)

    def _build_menu(self):
        self.menu = Gtk.Menu()

        self.item_status = Gtk.MenuItem(label='No device connected')
        self.item_status.set_sensitive(False)
        self.menu.append(self.item_status)

        self.menu.append(Gtk.SeparatorMenuItem())

        self.item_synctime = Gtk.MenuItem(label='Sync Time')
        self.item_synctime.connect('activate', self._on_sync_time)
        self.menu.append(self.item_synctime)

        self.item_installcab = Gtk.MenuItem(label='Install CAB…')
        self.item_installcab.connect('activate', self._on_install_cab)
        self.menu.append(self.item_installcab)

        self.item_programs = Gtk.MenuItem(label='Programs…')
        self.item_programs.connect('activate', self._on_programs)
        self.menu.append(self.item_programs)

        self.item_devinfo = Gtk.MenuItem(label='Device Info…')
        self.item_devinfo.connect('activate', self._on_dev_info)
        self.menu.append(self.item_devinfo)

        self.item_browse = Gtk.MenuItem(label='Browse Files…')
        self.item_browse.connect('activate', self._on_browse)
        self.menu.append(self.item_browse)

        self.menu.append(Gtk.SeparatorMenuItem())

        item_log = Gtk.MenuItem(label='FUSE Log…')
        item_log.connect('activate', self._on_log)
        self.menu.append(item_log)

        self.menu.append(Gtk.SeparatorMenuItem())

        item_quit = Gtk.MenuItem(label='Quit')
        item_quit.connect('activate', lambda _: Gtk.main_quit())
        self.menu.append(item_quit)

        self.menu.show_all()
        self.indicator.set_menu(self.menu)

        self._device_items = [
            self.item_synctime, self.item_installcab,
            self.item_programs, self.item_devinfo, self.item_browse,
        ]

    def _update_state(self, connected):
        self._connected = connected
        if connected:
            self.indicator.set_icon_full(ICON_CONN, 'Axim connected')
            self.item_status.set_label('Axim X50 — Connected')
        else:
            self.indicator.set_icon_full(ICON_DISC, 'Axim disconnected')
            self.item_status.set_label('No device connected')
        for item in self._device_items:
            item.set_sensitive(connected)

    def _poll(self):
        conn = is_connected()
        if conn != self._connected:
            self._update_state(conn)
        return True  # keep polling

    # ── Actions ────────────────────────────────────────────────────────────────

    def _on_sync_time(self, _):
        def done(ok, out):
            if ok:
                show_dialog(None, 'Time Synced', 'Device clock synced to PC.')
            else:
                show_dialog(None, 'Sync Failed', out, Gtk.MessageType.ERROR)
        run_tool(['synce-time'], done)

    def _on_install_cab(self, _):
        dlg = Gtk.FileChooserDialog(
            title='Select CAB file',
            action=Gtk.FileChooserAction.OPEN,
            buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                     Gtk.STOCK_OPEN,   Gtk.ResponseType.OK))
        f = Gtk.FileFilter()
        f.set_name('CAB files')
        f.add_pattern('*.cab')
        f.add_pattern('*.CAB')
        dlg.add_filter(f)
        if dlg.run() == Gtk.ResponseType.OK:
            path = dlg.get_filename()
            dlg.destroy()
            def done(ok, out):
                if ok:
                    show_dialog(None, 'Install Complete',
                                f'Installed: {os.path.basename(path)}')
                else:
                    show_dialog(None, 'Install Failed', out, Gtk.MessageType.ERROR)
            run_tool(['synce-install-cab', path], done)
        else:
            dlg.destroy()

    def _on_programs(self, _):
        if self._prog_win is None or not self._prog_win.get_visible():
            self._prog_win = ProgramsWindow()
            self._prog_win.connect('delete-event', lambda w, _: w.hide() or True)
        self._prog_win.present()

    def _on_dev_info(self, _):
        if self._info_win is None or not self._info_win.get_visible():
            self._info_win = DeviceInfoWindow()
            self._info_win.connect('delete-event', lambda w, _: w.hide() or True)
        self._info_win.present()

    def _on_browse(self, _):
        subprocess.Popen(['xdg-open', AXIM_MOUNT])

    def _on_log(self, _):
        if self._log_win is None or not self._log_win.get_visible():
            self._log_win = FuseLogWindow()
            self._log_win.connect('delete-event', lambda w, _: w.hide() or True)
        self._log_win.present()


def main():
    app = SynceApp()
    Gtk.main()

if __name__ == '__main__':
    main()
