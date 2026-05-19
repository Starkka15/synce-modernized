@SET_MAKE@

INCLUDES = -I$(top_srcdir)/lib/rapi/support -I$(top_srcdir)/lib/utils -I$(top_builddir)/lib/utils

## create this library
noinst_LTLIBRARIES = librapi.la

## use these sources
librapi_la_SOURCES = backend_ops_1.c backend_ops_2.c \
	connection.c window.c rapi_api.c \
	misc.c irapistream.h irapistream.c \
	rapi2_api.c
librapi_la_CFLAGS = -I$(top_srcdir)/lib/utils \
	@GLIB_CFLAGS@
librapi_la_LIBADD = $(top_builddir)/lib/rapi/support/libsupport.la \
	$(top_builddir)/lib/rapi/backend_ops_1/librapiops1.la \
	$(top_builddir)/lib/rapi/backend_ops_2/librapiops2.la
#	$(top_builddir)/lib/utils/libutils.la \
#	@DBUS_LIBS@

if !USE_GDBUS
librapi_la_CFLAGS += @DBUS_CFLAGS@
endif

include_HEADERS = rapi.h \
		rapitypes.h \
		rapi2.h \
		rapitypes2.h \
		irapistream.h

noinst_HEADERS = rapi_internal.h \
		rapi_ops.h \
		irapistream_internal.h

SUBDIRS = support backend_ops_1 backend_ops_2
