/* $Id: backend_ops_1.h 4141 2012-11-23 14:09:17Z mark_ellis $ */
#ifndef __backend_ops_1_h__
#define __backend_ops_1_h__
#include "synce.h"
#include "rapi_context.h"
#include "irapistream.h"

#ifndef SWIG

BOOL _CeCloseHandle(
        RapiContext *context,
        HANDLE hObject);

HANDLE _CeCreateFile(
        RapiContext *context,
        LPCWSTR lpFileName,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile);

BOOL _CeReadFile(
        RapiContext *context,
        HANDLE hFile,
        LPVOID lpBuffer,
        DWORD nNumberOfBytesToRead,
        LPDWORD lpNumberOfBytesRead,
        LPOVERLAPPED lpOverlapped);

BOOL _CeWriteFile(
        RapiContext *context,
        HANDLE hFile,
        LPCVOID lpBuffer,
        DWORD nNumberOfBytesToWrite,
        LPDWORD lpNumberOfBytesWritten,
        LPOVERLAPPED lpOverlapped);

BOOL _CeSetEndOfFile(
        RapiContext *context,
        HANDLE hFile);

DWORD _CeSetFilePointer(
        RapiContext *context,
        HANDLE hFile,
        LONG lDistanceToMove,
        LONG *lpDistanceToMoveHigh,
        DWORD dwMoveMethod);

BOOL _NotImplementedCeGetFileTime(
        RapiContext *context,
        HANDLE hFile,
        LPFILETIME lpCreationTime,
        LPFILETIME lpLastAccessTime,
        LPFILETIME lpLastWriteTime);

BOOL _NotImplementedCeSetFileTime(
        RapiContext *context,
        HANDLE hFile,
        LPFILETIME lpCreationTime,
        LPFILETIME lpLastAccessTime,
        LPFILETIME lpLastWriteTime);

#endif /* SWIG */


/*
 * File management functions
 */

#ifndef SWIG

BOOL _CeCopyFile(
        RapiContext *context,
        LPCWSTR lpExistingFileName,
        LPCWSTR lpNewFileName,
        BOOL bFailIfExists);

BOOL _CeCreateDirectory(
        RapiContext *context,
        LPCWSTR lpPathName,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes);

BOOL _CeDeleteFile(
        RapiContext *context,
        LPCWSTR lpFileName);

BOOL _CeFindAllFiles(
        RapiContext *context,
        LPCWSTR szPath,
        DWORD dwFlags,
        LPDWORD lpdwFoundCount,
        LPLPCE_FIND_DATA ppFindDataArray);

HANDLE _CeFindFirstFile(
        RapiContext *context,
        LPCWSTR lpFileName,
        LPCE_FIND_DATA lpFindFileData);

BOOL _CeFindNextFile(
        RapiContext *context,
        HANDLE hFindFile,
        LPCE_FIND_DATA lpFindFileData);

BOOL _CeFindClose(
        RapiContext *context,
        HANDLE hFindFile);

DWORD _CeGetFileAttributes(
        RapiContext *context,
        LPCWSTR lpFileName);

DWORD _CeGetFileSize(
        RapiContext *context,
        HANDLE hFile,
        LPDWORD lpFileSizeHigh);

DWORD _CeGetSpecialFolderPath(
        RapiContext *context,
        int nFolder,
        DWORD nBufferLength,
        LPWSTR lpBuffer);

BOOL _CeMoveFile(
        RapiContext *context,
        LPCWSTR lpExistingFileName,
        LPCWSTR lpNewFileName);

BOOL _CeRemoveDirectory(
        RapiContext *context,
        LPCWSTR lpPathName);

BOOL _CeSetFileAttributes(
        RapiContext *context,
        LPCWSTR lpFileName,
        DWORD dwFileAttributes);

BOOL _CeSHCreateShortcut(
        RapiContext *context,
        LPCWSTR lpszShortcut,
        LPCWSTR lpszTarget);

BOOL _CeSyncTimeToPc(
        RapiContext *context);

#endif /* SWIG */


/*
 * Database functions
 */

#ifndef SWIG

CEOID _CeCreateDatabase(
        RapiContext *context,
        LPWSTR lpszName,
        DWORD dwDbaseType,
        WORD wNumSortOrder,
        SORTORDERSPEC *rgSortSpecs);

BOOL _CeDeleteDatabase(
        RapiContext *context,
        CEOID oid);

BOOL _CeFindAllDatabases(
        RapiContext *context,
        DWORD dwDbaseType,
        WORD wFlags,
        LPWORD cFindData,
        LPLPCEDB_FIND_DATA ppFindData);

HANDLE _CeFindFirstDatabase(
        RapiContext *context,
        DWORD dwDbaseType);

CEOID _CeFindNextDatabase(
        RapiContext *context,
        HANDLE hEnum);

HANDLE _CeOpenDatabase(
        RapiContext *context,
        PCEOID poid,
        LPWSTR lpszName,
        CEPROPID propid,
        DWORD dwFlags,
        HWND hwndNotify);

CEOID _CeReadRecordProps(
        RapiContext *context,
        HANDLE hDbase,
        DWORD dwFlags,
        LPWORD lpcPropID,
        CEPROPID *rgPropID,
        LPBYTE *lplpBuffer,
        LPDWORD lpcbBuffer);

CEOID _CeSeekDatabase(
        RapiContext *context,
        HANDLE hDatabase,
        DWORD dwSeekType,
        DWORD dwValue,
        LPDWORD lpdwIndex);

CEOID _CeWriteRecordProps(
        RapiContext *context,
        HANDLE hDbase,
        CEOID oidRecord,
        WORD cPropID,
        CEPROPVAL *rgPropVal);

BOOL _CeDeleteRecord(
        RapiContext *context,
        HANDLE hDatabase,
        CEOID oidRecord);

BOOL _CeSetDatabaseInfo(
        RapiContext *context,
        CEOID oidDbase,
        CEDBASEINFO* pNewInfo);

#endif /* SWIG */

/*
 * Registry
 */

#ifndef SWIG

LONG _CeRegCreateKeyEx(
        RapiContext *context,
        HKEY hKey,
        LPCWSTR lpszSubKey,
        DWORD Reserved,
        LPWSTR lpszClass,
        DWORD ulOptions,
        REGSAM samDesired,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        PHKEY phkResult,
        LPDWORD lpdwDisposition);

LONG _CeRegOpenKeyEx(
        RapiContext *context,
        HKEY hKey,
        LPCWSTR lpszSubKey,
        DWORD ulOptions,
        REGSAM samDesired,
        PHKEY phkResult);

LONG _CeRegCloseKey(
        RapiContext *context,
        HKEY hKey);

LONG _CeRegQueryInfoKey(
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
        PFILETIME lpftLastWriteTime);

LONG _CeRegQueryValueEx(
        RapiContext *context,
        HKEY hKey,
        LPCWSTR lpValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE lpData,
        LPDWORD lpcbData);

LONG _CeRegEnumValue(
        RapiContext *context,
        HKEY hKey,
        DWORD dwIndex,
        LPWSTR lpszValueName,
        LPDWORD lpcbValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE lpData,
        LPDWORD lpcbData);

LONG _CeRegEnumKeyEx(
        RapiContext *context,
        HKEY hKey,
        DWORD dwIndex,
        LPWSTR lpName,
        LPDWORD lpcbName,
        LPDWORD lpReserved,
        LPWSTR lpClass,
        LPDWORD lpcbClass,
        PFILETIME lpftLastWriteTime);

LONG _CeRegSetValueEx(
        RapiContext *context,
        HKEY hKey,
        LPCWSTR lpValueName,
        DWORD Reserved,
        DWORD dwType,
        const BYTE *lpData,
        DWORD cbData);

LONG _CeRegDeleteValue(
        RapiContext *context,
	HKEY hKey,
	LPCWSTR lpszValueName);

LONG _CeRegDeleteKey(
        RapiContext *context,
	HKEY hKey,
	LPCWSTR lpszSubKey);

/*
 * Convenience functions for easy registry access
 */

bool _rapi_reg_create_key(
        HKEY parent,
        const char* name,
        HKEY* key);

bool _rapi_reg_open_key(
        HKEY parent, const char* name, HKEY* key);

bool _rapi_reg_query_dword(
        HKEY key,
        const char* name,
        DWORD* value);

bool _rapi_reg_query_string(
        HKEY key,
        const char* name,
        char** value);

bool _rapi_reg_set_dword(
        HKEY key,
        const char* name,
        DWORD value);

bool _rapi_reg_set_string(
        HKEY key,
        const char* name,
        const char *value);

#endif /* SWIG */

/*
 * Misc functions
 */

#ifndef SWIG

BOOL _CeCheckPassword(
        RapiContext *context,
        LPWSTR lpszPassword);

BOOL _CeCreateProcess(
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
        LPPROCESS_INFORMATION lpProcessInformation);

DWORD _CeGetLastError(
        RapiContext *context);

BOOL _CeGetStoreInformation(
        RapiContext *context,
        LPSTORE_INFORMATION lpsi);

void _CeGetSystemInfo(
        RapiContext *context,
        LPSYSTEM_INFO lpSystemInfo);

BOOL _CeGetSystemPowerStatusEx(
        RapiContext *context,
        PSYSTEM_POWER_STATUS_EX pSystemPowerStatus, BOOL refresh);

BOOL _CeGetVersionEx(
        RapiContext *context,
        LPCEOSVERSIONINFO lpVersionInformation);

BOOL _CeOidGetInfo(
        RapiContext *context,
        CEOID oid,
        CEOIDINFO *poidInfo);

HRESULT _CeProcessConfig(
        RapiContext *context,
        LPCWSTR config, DWORD flags, LPWSTR* reply);

BOOL _CeStartReplication(
        RapiContext *context);

HRESULT _NotImplementedCeSyncStart(
        RapiContext *context,
        LPCWSTR params);

HRESULT _NotImplementedCeSyncResume(
        RapiContext *context);

HRESULT _NotImplementedCeSyncPause(
        RapiContext *context);

BOOL _CeGetSystemMemoryDivision(
        RapiContext *context,
        LPDWORD lpdwStoragePages,
        LPDWORD lpdwRamPages,
        LPDWORD lpdwPageSize);

DWORD _CeSetSystemMemoryDivision(
        RapiContext *context,
        DWORD dwStoragePages);

BOOL _CeRegCopyFile(
        RapiContext *context,
        LPCWSTR filename);

BOOL _CeRegRestoreFile(
        RapiContext *context,
        LPCWSTR filename);

BOOL _CeKillAllApps(
        RapiContext *context);

DWORD _NotImplementedCeGetDiskFreeSpaceEx(
        RapiContext *context,
        LPCTSTR _lpDirectoryName, 
        PULARGE_INTEGER lpFreeBytesAvailable, 
        PULARGE_INTEGER lpTotalNumberOfBytes, 
        PULARGE_INTEGER lpTotalNumberOfFreeBytes);

#endif /* SWIG */

/*
 * CeRapiInvoke stuff
 */

#ifndef SWIG

HRESULT _CeRapiInvoke(
        RapiContext *context,
        LPCWSTR pDllPath,
        LPCWSTR pFunctionName,
        DWORD cbInput,
        const BYTE *pInput,
        DWORD *pcbOutput,
        BYTE **ppOutput,
        IRAPIStream **ppIRAPIStream,
        DWORD dwReserved);

#endif /* SWIG */

#endif /* __backend_ops_1_h__ */


