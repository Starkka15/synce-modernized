## Makefile.am -- Process this file with automake to produce Makefile.in
@SET_MAKE@

## don't complain about missing AUTHORS, NEWS, ChangeLog, etc
AUTOMAKE_OPTIONS = foreign 1.4

pcfiles = libsynce.pc

all_local: $(pcfiles)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcfiles)

EXTRA_DIST = libsynce.pc.in LICENSE BUGS README.contributing README.design

CLEANFILES = $(pcfiles)

SUBDIRS = lib man tools tests python dccm scripts bluetooth etc docs .
