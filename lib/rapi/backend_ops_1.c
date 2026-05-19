/* $Id: backend_ops_1.c 4034 2011-12-28 18:47:08Z mark_ellis $ */
#undef __STRICT_ANSI__
#define _GNU_SOURCE
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "backend_ops_1/backend_ops_1.h"
#include "rapi_ops.h"
#include <stdlib.h>


/* The rapi application interface indirection */
struct rapi_ops_s rapi_ops =
{
#ifndef SWIG
        &_CeCloseHandle,                  /* CeCloseHandle */
        &_CeCreateFile,                   /* CeCreateFile */
        &_CeReadFile,                     /* CeReadFile */
        &_CeWriteFile,                    /* CeWriteFile */
        &_CeSetFilePointer,               /* CeSetFilePointer */
        &_CeSetEndOfFile,                 /* CeSetEndOfFile */
        &_NotImplementedCeGetFileTime,    /* CeGetFileTime */
        &_NotImplementedCeSetFileTime,    /* CeSetFileTime */
#endif /* SWIG */
        /*
         * File management functions
         */
#ifndef SWIG
        &_CeCopyFile,                     /* CeCopyFile */
        &_CeCreateDirectory,              /* CeCreateDirectory */
        &_CeDeleteFile,                   /* CeDeleteFile */
        &_CeFindAllFiles,                 /* CeFindAllFiles */
        &_CeFindFirstFile,                /* CeFindFirstFile */
        &_CeFindNextFile,                 /* CeFindNextFile */
        &_CeFindClose,                    /* CeFindClose */
        &_CeGetFileAttributes,            /* CeGetFileAttributes */
        &_CeGetFileSize,                  /* CeGetFileSize */
        &_CeGetSpecialFolderPath,         /* CeGetSpecialFolderPath */
        &_CeMoveFile,                     /* CeMoveFile */
        &_CeRemoveDirectory,              /* CeRemoveDirectory */
        &_CeSetFileAttributes,            /* CeSetFileAttributes */
        &_CeSHCreateShortcut,             /* CeSHCreateShortcut */
        &_CeSyncTimeToPc,                 /* CeSyncTimeToPc */
#endif /* SWIG */
        /*
         * Database functions
         */
#ifndef SWIG
        &_CeCreateDatabase,               /* CeCreateDatabase */
        &_CeDeleteDatabase,               /* CeDeleteDatabase */
        &_CeFindAllDatabases,             /* CeFindAllDatabases */
        &_CeFindFirstDatabase,            /* CeFindFirstDatabase */
        &_CeFindNextDatabase,             /* CeFindNextDatabase */
        &_CeOpenDatabase,                 /* CeOpenDatabase */
        &_CeReadRecordProps,              /* CeReadRecordProps */
        &_CeSeekDatabase,                 /* CeSeekDatabase */
        &_CeWriteRecordProps,             /* CeWriteRecordProps */
        &_CeDeleteRecord,                 /* CeDeleteRecord */
        &_CeSetDatabaseInfo,              /* CeSetDatabaseInfo */
#endif /* SWIG */
        /*
         * Registry
         */
#ifndef SWIG
        &_CeRegCreateKeyEx,               /* CeRegCreateKeyEx */
        &_CeRegOpenKeyEx,                 /* CeRegOpenKeyEx */
        &_CeRegCloseKey,                  /* CeRegCloseKey */
        &_CeRegDeleteKey,                 /* CeRegDeleteKey */
        &_CeRegDeleteValue,               /* CeRegDeleteValue */
        &_CeRegQueryInfoKey,              /* CeRegQueryInfoKey */
        &_CeRegQueryValueEx,              /* CeRegQueryValueEx */
        &_CeRegEnumValue,                 /* CeRegEnumValue */
        &_CeRegEnumKeyEx,                 /* CeRegEnumKeyEx */
        &_CeRegSetValueEx,                /* CeRegSetValueEx */
#endif /* SWIG */
        /*
         * Misc functions
         */
#ifndef SWIG
        &_CeCheckPassword,                /* CeCheckPassword */
        &_CeCreateProcess,                /* CeCreateProcess */
        &_CeGetStoreInformation,          /* CeGetStoreInformation */
        &_CeGetSystemInfo,                /* CeGetSystemInfo */
        &_CeGetSystemPowerStatusEx,       /* CeGetSystemPowerStatusEx */
        &_CeGetVersionEx,                 /* CeGetVersionEx */
        &_CeOidGetInfo,                   /* CeOidGetInfo */
        &_CeProcessConfig,                /* CeProcessConfig */
        &_CeStartReplication,             /* CeStartReplication */
        &_NotImplementedCeSyncStart,      /* CeSyncStart */
        &_NotImplementedCeSyncResume,     /* CeSyncResume */
        &_NotImplementedCeSyncPause,      /* CeSyncPause */
        &_CeGetSystemMemoryDivision,      /* CeGetSystemMemoryDivision */
        &_CeSetSystemMemoryDivision,      /* CeSetSystemMemoryDivision */
        &_CeRegCopyFile,                  /* CeRegCopyFile */
        &_CeRegRestoreFile,               /* CeRegRestoreFile */
        &_CeKillAllApps,                  /* CeKillAllApps */
        &_NotImplementedCeGetDiskFreeSpaceEx, /* CeGetDiskFreeSpaceEx */
#endif /* SWIG */
        /*
         * CeRapiInvoke stuff
         */
#ifndef SWIG
        &_CeRapiInvoke,                   /* CeRapiInvoke */
#endif /* SWIG */
};
