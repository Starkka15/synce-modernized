AM_CXXFLAGS = -I$(top_srcdir)/lib/utils -I$(top_builddir)/lib/utils -Wall
AM_LDFLAGS = $(top_builddir)/lib/libsynce.la

INCLUDES = -I$(top_srcdir)/lib/rapi -I$(top_srcdir)/lib/rapi/support
LDADD = $(top_builddir)/lib/libsynce.la

noinst_PROGRAMS = \
	CeCreateDatabase \
	CeCreateDirectory \
	CeCreateFile \
	CeFindAllDatabases \
	CeFindAllFiles \
	CeFindFirstFile \
	CeGetVersionEx \
	CeGetSpecialFolderPath \
	CeMoveFile \
	CeOpenDatabase \
	CeRapiInit

CeCreateDatabase_SOURCES = test.h CeCreateDatabase.cpp
CeRapiInit_SOURCES = test.h CeRapiInit.cpp
CeGetVersionEx_SOURCES = test.h CeGetVersionEx.cpp
CeGetSpecialFolderPath_SOURCES = test.h CeGetSpecialFolderPath.cpp
CeCreateFile_SOURCES = test.h CeCreateFile.cpp
CeFindFirstFile_SOURCES = test.h CeFindFirstFile.cpp
CeFindAllFiles_SOURCES = test.h CeFindAllFiles.cpp
CeCreateDirectory_SOURCES = test.h CeCreateDirectory.cpp
CeMoveFile_SOURCES = test.h CeMoveFile.cpp
CeOpenDatabase_SOURCES = test.h CeOpenDatabase.cpp
CeFindAllDatabases_SOURCES = test.h CeFindAllDatabases.cpp

