syncedatadir = ${datadir}/@PACKAGE_TARNAME@
syncedata_DATA = synceconnector.py


udevrulesdir = @udev_dir@/rules.d
udevrules_DATA = 85-synce.rules

udevstartersdir = @udev_dir@
udevstarters_SCRIPTS = \
	synce-udev-serial \
	synce-udev-rndis

syncedata_SCRIPTS = \
	udev-synce-rndis \
	udev-synce-serial

bin_SCRIPTS = synce-serial

