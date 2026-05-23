AM_CXXFLAGS = -I$(top_srcdir)/lib/utils -I$(top_builddir)/lib/utils -Wall
AM_CFLAGS = -I$(top_srcdir)/lib/utils -I$(top_builddir)/lib/utils -Wall
AM_LDFLAGS = $(top_builddir)/lib/libsynce.la

INCLUDES = -I$(top_srcdir)/lib/rapi

noinst_PROGRAMS = CeRapiInvoke

CeRapiInvoke_SOURCES = CeRapiInvoke.cc pcommon.h pcommon.c
CeRapiInvoke_LDADD = $(top_builddir)/lib/libsynce.la

##TESTS = CeRapiInvoke

SUBDIRS = dll
