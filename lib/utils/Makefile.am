INCLUDES = -I..

## create this library
noinst_LTLIBRARIES = libutils.la

## use these sources
libutils_la_SOURCES = \
	datetype.c \
	errors.c \
	files.c \
	filetime.c \
	info.c \
	password.c \
	synce_hash.h synce_hash.c \
	synce_ini.h synce_ini.c \
	synce_log.h synce_log.c \
	synce_socket.h synce_socket.c \
	synce_vector_template.h \
	time_fields.c \
	wstr.c

nodist_libutils_la_SOURCES = synce.h

if IMPLEMENT_BSWAP_XX
libutils_la_SOURCES += bswap.c
endif

libutils_la_CFLAGS = @GLIB_CFLAGS@
libutils_la_LIBADD  = config/libconfig.la -lm @LTLIBICONV@ @LTLIBOBJS@ @GLIB_LIBS@

if !USE_GDBUS
libutils_la_CFLAGS += @DBUS_CFLAGS@
libutils_la_LIBADD += @DBUS_LIBS@
endif

include_HEADERS = \
	synce_hash.h \
	synce_ini.h \
	synce_log.h \
	synce_socket.h \
	synce_types.h \
	synce_sys_error.h \
	synce_vector_template.h

nodist_include_HEADERS = synce.h

##noinst_PROGRAMS = ftconv

##ftconv_SOURCES = ftconv.c
##ftconv_LDADD = libsynce.la

SUBDIRS = config .

