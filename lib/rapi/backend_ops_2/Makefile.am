@SET_MAKE@

INCLUDES = -I$(top_srcdir)/lib/rapi -I$(top_srcdir)/lib/rapi/support -I$(top_builddir)/lib/utils

## create this library
noinst_LTLIBRARIES = librapiops2.la

## use these sources
librapiops2_la_SOURCES = file_management2.c file_access2.c misc2.c registry2.c password2.c database2.c invoke2.c
librapiops2_la_CFLAGS = -I$(top_srcdir)/lib/utils
#librapiops2_la_LIBADD = $(top_builddir)/lib/utils/libutils.la

noinst_HEADERS = backend_ops_2.h
