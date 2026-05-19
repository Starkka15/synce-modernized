AM_CFLAGS = -I$(top_srcdir)/lib/utils -I$(top_builddir)/lib/utils

INCLUDES = -I$(top_srcdir)/lib/rapi -I$(top_srcdir)/lib/rapi/support
LDADD = $(top_builddir)/lib/libsynce.la

noinst_PROGRAMS = oidinfo

oidinfo_SOURCES = oidinfo.c

bin_PROGRAMS = pcp \
	pls \
	pmkdir \
	pmv \
	prm \
	prmdir \
	prun \
	pstatus \
	rapiconfig \
	pkillall \
	pshortcut \
	psettime \
	synce-list-programs \
	synce-registry \
	synce-database

pcp_SOURCES    = pcommon.c pcp.c
pls_SOURCES    = pcommon.c pls.c
pmkdir_SOURCES = pcommon.c pmkdir.c
pmv_SOURCES    = pcommon.c pmv.c
prm_SOURCES    = pcommon.c prm.c
prmdir_SOURCES = pcommon.c prmdir.c
prun_SOURCES   = pcommon.c prun.c
pstatus_SOURCES= pstatus.c
pkillall_SOURCES=pkillall.c
pshortcut_SOURCES=pcommon.c pshortcut.c
psettime_SOURCES=psettime.c

rapiconfig_SOURCES = rapiconfig.c

synce_list_programs_SOURCES = synce-list-programs.c
synce_registry_SOURCES = synce-registry.c pcommon.c
synce_database_SOURCES = synce-database.c pcommon.c

noinst_HEADERS = pcommon.h

bin_SCRIPTS = synce-install-cab \
	synce-remove-program
