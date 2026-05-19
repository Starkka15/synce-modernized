/* $Id: rapi_ops.h 4141 2012-11-23 14:09:17Z mark_ellis $ */
#undef __STRICT_ANSI__
#define _GNU_SOURCE
#include "synce.h"
#include "rapi_context.h"
#include "irapistream.h"
#include <stdlib.h>

struct rapi_ops_s
{
#ifndef SWIG

    BOOL ( *CeCloseHandle ) (
        RapiContext *context,
        HANDLE hObject );

    HANDLE ( *CeCreateFile ) (
            RapiContext *context,
            LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile );

    BOOL ( *CeReadFile ) (
            RapiContext *context,
            HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped );

    BOOL ( *CeWriteFile ) (
            RapiContext *context,
            HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped );

    DWORD ( *CeSetFilePointer ) (
            RapiContext *context,
    HANDLE hFile,
    LONG lDistanceToMove,
    PLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod );

    BOOL  ( *CeSetEndOfFile ) (
            RapiContext *context,
    HANDLE hObject);

    BOOL ( *CeGetFileTime ) (
            RapiContext *context,
    HANDLE hFile,
    LPFILETIME lpCreationTime,
    LPFILETIME lpLastAccessTime,
    LPFILETIME lpLastWriteTime );

    BOOL ( *CeSetFileTime ) (
            RapiContext *context,
    HANDLE hFile,
    LPFILETIME lpCreationTime,
    LPFILETIME lpLastAccessTime,
    LPFILETIME lpLastWriteTime );

#endif /* SWIG */


    /*
    * File management functions
    */

#ifndef SWIG

    BOOL ( *CeCopyFile ) (
            RapiContext *context,
            LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists );

    BOOL ( *CeCreateDirectory ) (
            RapiContext *context,
            LPCWSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes );

    BOOL ( *CeDeleteFile ) (
            RapiContext *context,
            LPCWSTR lpFileName );

    BOOL ( *CeFindAllFiles ) (
            RapiContext *context,
            LPCWSTR szPath,
    DWORD dwFlags,
    LPDWORD lpdwFoundCount,
    LPLPCE_FIND_DATA ppFindDataArray );

    HANDLE ( *CeFindFirstFile ) (
            RapiContext *context,
            LPCWSTR lpFileName,
    LPCE_FIND_DATA lpFindFileData );

    BOOL ( *CeFindNextFile ) (
            RapiContext *context,
            HANDLE hFindFile,
    LPCE_FIND_DATA lpFindFileData );

    BOOL ( *CeFindClose ) (
            RapiContext *context,
            HANDLE hFindFile );

    DWORD ( *CeGetFileAttributes ) (
            RapiContext *context,
            LPCWSTR lpFileName );

    DWORD ( *CeGetFileSize ) (
            RapiContext *context,
            HANDLE hFile,
    LPDWORD lpFileSizeHigh );

    DWORD ( *CeGetSpecialFolderPath ) (
            RapiContext *context,
            int nFolder,
    DWORD nBufferLength,
    LPWSTR lpBuffer );

    BOOL ( *CeMoveFile ) (
            RapiContext *context,
            LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName );

    BOOL ( *CeRemoveDirectory ) (
            RapiContext *context,
            LPCWSTR lpPathName );

    BOOL ( *CeSetFileAttributes ) (
            RapiContext *context,
            LPCWSTR lpFileName,
    DWORD dwFileAttributes );

    BOOL ( *CeSHCreateShortcut ) (
            RapiContext *context,
            LPCWSTR lpszShortcut,
    LPCWSTR lpszTarget );

    BOOL ( *CeSyncTimeToPc ) (
            RapiContext *context );

#endif /* SWIG */


    /*
    * Database functions
    */

#ifndef SWIG

    CEOID ( *CeCreateDatabase ) (
        RapiContext *context,
        LPWSTR lpszName,
        DWORD dwDbaseType,
        WORD wNumSortOrder,
        SORTORDERSPEC *rgSortSpecs );

    BOOL ( *CeDeleteDatabase ) (
            RapiContext *context,
            CEOID oid );

    BOOL ( *CeFindAllDatabases ) (
            RapiContext *context,
            DWORD dwDbaseType,
    WORD wFlags,
    LPWORD cFindData,
    LPLPCEDB_FIND_DATA ppFindData );

    HANDLE ( *CeFindFirstDatabase ) (
            RapiContext *context,
            DWORD dwDbaseType );

    CEOID ( *CeFindNextDatabase ) (
            RapiContext *context,
            HANDLE hEnum );

    HANDLE ( *CeOpenDatabase ) (
            RapiContext *context,
            PCEOID poid,
    LPWSTR lpszName,
    CEPROPID propid,
    DWORD dwFlags,
    HWND hwndNotify );

    CEOID ( *CeReadRecordProps ) (
            RapiContext *context,
            HANDLE hDbase,
    DWORD dwFlags,
    LPWORD lpcPropID,
    CEPROPID *rgPropID,
    LPBYTE *lplpBuffer,
    LPDWORD lpcbBuffer );

    CEOID ( *CeSeekDatabase ) (
            RapiContext *context,
            HANDLE hDatabase,
    DWORD dwSeekType,
    DWORD dwValue,
    LPDWORD lpdwIndex );

    CEOID ( *CeWriteRecordProps ) (
            RapiContext *context,
            HANDLE hDbase,
    CEOID oidRecord,
    WORD cPropID,
    CEPROPVAL *rgPropVal );

    BOOL ( *CeDeleteRecord ) (
            RapiContext *context,
            HANDLE hDatabase,
    CEOID oidRecord );

    BOOL ( *CeSetDatabaseInfo ) (
            RapiContext *context,
            CEOID oidDbase,
    CEDBASEINFO* pNewInfo );

#endif /* SWIG */


    /*
    * Registry
    */

#ifndef SWIG

    LONG ( *CeRegCreateKeyEx ) (
        RapiContext *context,
        HKEY hKey,
        LPCWSTR lpszSubKey,
        DWORD Reserved,
        LPWSTR lpszClass,
        DWORD ulOptions,
        REGSAM samDesired,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        PHKEY phkResult,
        LPDWORD lpdwDisposition );

    LONG ( *CeRegOpenKeyEx ) (
            RapiContext *context,
            HKEY hKey,
    LPCWSTR lpszSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult );

    LONG ( *CeRegCloseKey ) (
            RapiContext *context,
            HKEY hKey );

    LONG ( *CeRegDeleteKey ) (
            RapiContext *context,
            HKEY hKey,
            LPCWSTR lpszSubKey );

    LONG ( *CeRegDeleteValue ) (
            RapiContext *context,
            HKEY hKey,
            LPCWSTR lpszValueName );

    LONG ( *CeRegQueryInfoKey ) (
            RapiContext *context,
            HKEY hKey,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    LPDWORD lpReserved,
    LPDWORD lpcSubKeys,
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcbMaxClassLen,
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime );

    LONG ( *CeRegQueryValueEx ) (
            RapiContext *context,
            HKEY hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData );

    LONG ( *CeRegEnumValue ) (
            RapiContext *context,
            HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpszValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData );

    LONG ( *CeRegEnumKeyEx ) (
            RapiContext *context,
            HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime );

    LONG ( *CeRegSetValueEx ) (
            RapiContext *context,
            HKEY hKey,
    LPCWSTR lpValueName,
    DWORD Reserved,
    DWORD dwType,
    const BYTE *lpData,
    DWORD cbData );

#endif /* SWIG */


    /*
    * Misc functions
    */

#ifndef SWIG

    BOOL ( *CeCheckPassword ) (
        RapiContext *context,
        LPWSTR lpszPassword );

    BOOL ( *CeCreateProcess ) (
            RapiContext *context,
            LPCWSTR lpApplicationName,
    LPCWSTR lpCommandLine,
    void* lpProcessAttributes,
    void* lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPWSTR lpCurrentDirectory,
    void* lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation );

    BOOL ( *CeGetStoreInformation ) (
            RapiContext *context,
            LPSTORE_INFORMATION lpsi );

    void ( *CeGetSystemInfo ) (
            RapiContext *context,
            LPSYSTEM_INFO lpSystemInfo );

    BOOL ( *CeGetSystemPowerStatusEx ) (
            RapiContext *context,
            PSYSTEM_POWER_STATUS_EX pSystemPowerStatus,
            BOOL refresh );

    BOOL ( *CeGetVersionEx ) (
            RapiContext *context,
            LPCEOSVERSIONINFO lpVersionInformation );

    BOOL ( *CeOidGetInfo ) (
            RapiContext *context,
            CEOID oid,
    CEOIDINFO *poidInfo );

    HRESULT ( *CeProcessConfig ) (
            RapiContext *context,
            LPCWSTR config,
            DWORD flags,
            LPWSTR* reply );

    BOOL ( *CeStartReplication ) (
            RapiContext *context );

    HRESULT ( *CeSyncStart ) (
            RapiContext *context,
            LPCWSTR params );

    HRESULT ( *CeSyncResume ) (
            RapiContext *context );

    HRESULT ( *CeSyncPause ) (
            RapiContext *context );

    BOOL ( *CeGetSystemMemoryDivision ) (
            RapiContext *context,
            LPDWORD lpdwStoragePages,
    LPDWORD lpdwRamPages,
    LPDWORD lpdwPageSize );

    DWORD ( *CeSetSystemMemoryDivision ) (
            RapiContext *context,
            DWORD dwStoragePages );

    BOOL ( *CeRegCopyFile ) (
            RapiContext *context,
            LPCWSTR filename );

    BOOL ( *CeRegRestoreFile ) (
            RapiContext *context,
            LPCWSTR filename );

    BOOL ( *CeKillAllApps ) (
            RapiContext *context );

    DWORD ( *CeGetDiskFreeSpaceEx)(
            RapiContext *context,
            LPCTSTR _lpDirectoryName, 
            PULARGE_INTEGER lpFreeBytesAvailable, 
            PULARGE_INTEGER lpTotalNumberOfBytes, 
            PULARGE_INTEGER lpTotalNumberOfFreeBytes );


#endif /* SWIG */


    /*
    * CeRapiInvoke stuff
    */

#ifndef SWIG

    HRESULT ( *CeRapiInvoke ) (
            RapiContext *context,
            LPCWSTR pDllPath,
    LPCWSTR pFunctionName,
    DWORD cbInput,
    const BYTE *pInput,
    DWORD *pcbOutput,
    BYTE **ppOutput,
    IRAPIStream **ppIRAPIStream,
    DWORD dwReserved );

#endif /* SWIG */
};
