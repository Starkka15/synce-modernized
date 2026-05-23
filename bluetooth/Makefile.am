if ENABLE_BLUETOOTH_SUPPORT

pppdir = ${sysconfdir}/ppp
pppupdir = ${pppdir}/ip-up.d
pppdowndir = ${pppdir}/ip-down.d
ppppeerdir = ${pppdir}/peers

EXTRA_DIST = README
ppppeer_DATA = synce-bt-peer

syncedatadir = ${datadir}/@PACKAGE_TARNAME@

pppup_SCRIPTS = synce-udev-bt-ipup

else

EXTRA_DIST = synce-bt-peer \
	README \
	synce-udev-bt-ipup.in

endif
