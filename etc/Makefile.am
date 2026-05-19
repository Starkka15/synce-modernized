syncedatadir = ${datadir}/@PACKAGE_TARNAME@
syncedata_DATA = dhclient.conf

udevservicefile = org.synce.dccm.conf
udevservicestartfile = org.synce.dccm.service

dbusservicedir = "`pkg-config --variable=sysconfdir dbus-1`/dbus-1/system.d"
dbusservicestartdir = `pkg-config --variable=session_bus_services_dir dbus-1 | sed s/services/system-services/`
dbusservice_DATA = $(udevservicefile)
dbusservicestart_DATA = $(udevservicestartfile)

EXTRA_DIST = $(syncedata_DATA) $(udevservicefile)
