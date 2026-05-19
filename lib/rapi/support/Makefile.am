INCLUDES = -I$(top_srcdir)/lib/rapi -I$(top_builddir)/lib/utils

## create this library
noinst_LTLIBRARIES = libsupport.la

## use these sources
libsupport_la_SOURCES = \
	rapi_buffer.h   rapi_buffer.c  \
	rapi_context.h  rapi_context.c
libsupport_la_CFLAGS = -I$(top_srcdir)/lib/utils @GLIB_CFLAGS@
#libsupport_la_LIBADD  = @DBUS_LIBS@ $(top_builddir)/lib/utils/libutils.la

if !USE_GDBUS
libsupport_la_CFLAGS += @DBUS_CFLAGS@
endif
