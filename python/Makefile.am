if HAVE_PYTHON

rapibindingsdir = $(pyexecdir)
rapibindings_LTLIBRARIES = pyrapi2.la

pyrapi2_la_CFLAGS = @PYTHON_INCLUDES@ -I$(top_srcdir)/lib/utils -I$(top_builddir)/lib/utils -I$(top_srcdir)/lib/rapi
pyrapi2_la_LDFLAGS = -module -avoid-version -fPIC
pyrapi2_la_LIBADD = $(top_builddir)/lib/libsynce.la
nodist_pyrapi2_la_SOURCES = pyrapi2.c

SUBDIRS = tests

endif

CLEANFILES = pyrapi2.c

EXTRA_DIST = types.pxi constants.pxi pyrapi2.pyx

pyrapi2.c: $(srcdir)/types.pxi $(srcdir)/pyrapi2.pyx
	pyrexc $(srcdir)/pyrapi2.pyx -o ./pyrapi2.c
