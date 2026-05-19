/* $Id: backend_ops_2.c 4034 2011-12-28 18:47:08Z mark_ellis $ */
#undef __STRICT_ANSI__
#define _GNU_SOURCE
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "backend_ops_2/backend_ops_2.h"
#include "rapi_ops.h"
#include <stdlib.h>


/* The rapi2 application interface indirection */
struct rapi_ops_s rapi2_ops =
{
#ifndef SWIG
        &_CeCloseHandle2,                 /* CeCloseHandle */
        &_CeCreateFile2,                  /* CeCreateFile */
        &_CeReadFile2,                    /* CeReadFile */
        &_CeWriteFile2,                   /* CeWriteFile */
        &_CeSetFilePointer2,              /* CeSetFilePointer */
        &_CeSetEndOfFile2,                /* CeSetEndOfFile */
        &_CeGetFileTime2,                 /* CeGetFileTime */
        &_CeSetFileTime2,                 /* CeSetFileTime */
#endif /* SWIG */
        /*
         * File management functions
        */
#ifndef SWIG
        &_CeCopyFile2,                    /* CeCopyFile */
        &_CeCreateDirectory2,             /* CeCreateDirectory */
        &_CeDeleteFile2,                  /* CeDeleteFile */
        &_CeFindAllFiles2,                /* CeFindAllFiles */
        &_CeFindFirstFile2,               /* CeFindFirstFile */
        &_CeFindNextFile2,                /* CeFindNextFile */
        &_CeFindClose2,                   /* CeFindClose */
        &_CeGetFileAttributes2,           /* CeGetFileAttributes */
        &_CeGetFileSize2,                 /* CeGetFileSize */
        &_CeGetSpecialFolderPath2,        /* CeGetSpecialFolderPath */
        &_CeMoveFile2,                    /* CeMoveFile */
        &_CeRemoveDirectory2,             /* CeRemoveDirectory */
        &_NotImplementedCeSetFileAttributes2, /* CeSetFileAttributes */
        &_NotImplementedCeSHCreateShortcut2, /* CeSHCreateShortcut */
        _CeSyncTimeToPc2,                 /* CeSyncTimeToPc */
#endif /* SWIG */
        /*
         * Database functions
        */
#ifndef SWIG
        _NotImplementedCeCreateDatabase2, /* CeCreateDatabase */
        _NotImplementedCeDeleteDatabase2, /* CeDeleteDatabase */
        _NotImplementedCeFindAllDatabases2, /* CeFindAllDatabases */
        _NotImplementedCeFindFirstDatabase2, /* CeFindFirstDatabase */
        _NotImplementedCeFindNextDatabase2, /* CeFindNextDatabase */
        _NotImplementedCeOpenDatabase2,   /* CeOpenDatabase */
        _NotImplementedCeReadRecordProps2, /* CeReadRecordProps */
        _NotImplementedCeSeekDatabase2,   /* CeSeekDatabase */
        _NotImplementedCeWriteRecordProps2, /* CeWriteRecordProps */
        _NotImplementedCeDeleteRecord2,   /* CeDeleteRecord */
        _NotImplementedCeSetDatabaseInfo2, /* CeSetDatabaseInfo */
#endif /* SWIG */
        /*
         * Registry
        */
#ifndef SWIG
        _CeRegCreateKeyEx2,               /* CeRegCreateKeyEx */
        _CeRegOpenKeyEx2,                 /* CeRegOpenKeyEx */
        _CeRegCloseKey2,                  /* CeRegCloseKey */
        _CeRegDeleteKey2,                 /* CeRegDeleteKey */
        _CeRegDeleteValue2,               /* CeRegDeleteValue */
        _CeRegQueryInfoKey2,              /* CeRegQueryInfoKey */
        _CeRegQueryValueEx2,              /* CeRegQueryValueEx */
        _CeRegEnumValue2,                 /* CeRegEnumValue */
        _CeRegEnumKeyEx2,                 /* CeRegEnumKeyEx */
        _CeRegSetValueEx2,                /* CeRegSetValueEx */
#endif /* SWIG */
        /*
         * Misc functions
        */
#ifndef SWIG
        _NotImplementedCeCheckPassword2,  /* CeCheckPassword */
        _CeCreateProcess2,                /* CeCreateProcess */
        _CeGetStoreInformation2,          /* CeGetStoreInformation */
        _CeGetSystemInfo2,                /* CeGetSystemInfo */
        _CeGetSystemPowerStatusEx2,       /* CeGetSystemPowerStatusEx */
        _CeGetVersionEx2,                 /* CeGetVersionEx */
        _NotImplementedCeOidGetInfo2,     /* CeOidGetInfo */
        _CeProcessConfig2,                /* CeProcessConfig */
        _CeStartReplication2,             /* CeStartReplication */
        _CeSyncStart2,                    /* CeSyncStart */
        _CeSyncResume2,                   /* CeSyncResume */
        _CeSyncPause2,                    /* CeSyncPause */
        _NotImplementedCeGetSystemMemoryDivision2, /* CeGetSystemMemoryDivision */
        _NotImplementedCeSetSystemMemoryDivision2, /* CeSetSystemMemoryDivision */
        _NotImplementedCeRegCopyFile2,    /* CeRegCopyFile */
        _NotImplementedCeRegRestoreFile2, /* CeRegRestoreFile */
        _NotImplementedCeKillAllApps2,    /* CeKillAllApps */
        _CeGetDiskFreeSpaceEx2,           /* CeGetDiskFreeSpaceEx */
#endif /* SWIG */
        /*
         * CeRapiInvoke stuff
        */
#ifndef SWIG
        _CeRapiInvoke2,     /* CeRapiInvoke */
#endif /* SWIG */
};
