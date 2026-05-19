#ifndef __backend_ops_2_h__
#define __backend_ops_2_h__
#include "synce.h"
#include "rapi_context.h"
#include "irapistream.h"

DWORD _CeGetSpecialFolderPath2(
        RapiContext *context,
        int nFolder,
        DWORD nBufferLength,
        LPWSTR lpBuffer);

BOOL _CeFindAllFiles2(
        RapiContext *context,
        LPCWSTR szPath,
        DWORD dwFlags,
        LPDWORD lpdwFoundCount,
        LPLPCE_FIND_DATA ppFindDataArray);

HANDLE _CeFindFirstFile2(
        RapiContext *context,
        LPCWSTR lpFileName,
        LPCE_FIND_DATA lpFindFileData);

BOOL _CeFindNextFile2(
        RapiContext *context,
        HANDLE hFindFile,
        LPCE_FIND_DATA lpFindFileData);

BOOL _CeFindClose2(
        RapiContext *context,
        HANDLE hFindFile);

DWORD _CeGetFileAttributes2(
        RapiContext *context,
        LPCWSTR lpFileName);


HANDLE _CeCreateFile2(
        RapiContext *context,
        LPCWSTR lpFileName,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile);

BOOL _CeReadFile2(
        RapiContext *context,
        HANDLE hFile,
        LPVOID lpBuffer,
        DWORD nNumberOfBytesToRead,
        LPDWORD lpNumberOfBytesRead,
        LPOVERLAPPED lpOverlapped);

BOOL _CeDeleteFile2(
        RapiContext *context,
        LPCWSTR lpFileName);

BOOL _CeWriteFile2(
        RapiContext *context,
        HANDLE hFile,
        LPCVOID lpBuffer,
        DWORD nNumberOfBytesToWrite,
        LPDWORD lpNumberOfBytesWritten,
        LPOVERLAPPED lpOverlapped);

DWORD _CeSetFilePointer2(
        RapiContext *context,
        HANDLE hFile,
        LONG lDistanceToMove,
        PLONG lpDistanceToMoveHigh,
        DWORD dwMoveMethod);

BOOL _CeSetEndOfFile2(
        RapiContext *context,
        HANDLE hFile);

BOOL _CeGetFileTime2(
        RapiContext *context,
        HANDLE hFile,
        LPFILETIME lpCreationTime,
        LPFILETIME lpLastAccessTime,
        LPFILETIME lpLastWriteTime);

BOOL _CeSetFileTime2(
        RapiContext *context,
        HANDLE hFile,
        LPFILETIME lpCreationTime,
        LPFILETIME lpLastAccessTime,
        LPFILETIME lpLastWriteTime);

BOOL _CeCloseHandle2(
        RapiContext *context,
        HANDLE hObject);

BOOL _CeCreateDirectory2(
        RapiContext *context,
        LPCWSTR lpPathName,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes);

BOOL _CeRemoveDirectory2(
        RapiContext *context,
        LPCWSTR lpPathName);

DWORD _CeGetFileSize2(
        RapiContext *context,
        HANDLE hFile,
        LPDWORD lpFileSizeHigh);

BOOL _CeCopyFile2(
        RapiContext *context,
        LPCWSTR lpExistingFileName,
        LPCWSTR lpNewFileName,
        BOOL bFailIfExists);


LONG _CeRegQueryInfoKey2(
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

BOOL _CeMoveFile2(
        RapiContext *context,
        LPCWSTR lpExistingFileName,
        LPCWSTR lpNewFileName);

BOOL _CeCreateProcess2(
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

void _CeGetSystemInfo2(
        RapiContext *context,
        LPSYSTEM_INFO lpSystemInfo);

BOOL _CeGetVersionEx2(
        RapiContext *context,
        LPCEOSVERSIONINFO lpVersionInformation);

BOOL _CeGetSystemPowerStatusEx2(
        RapiContext *context,
        PSYSTEM_POWER_STATUS_EX pSystemPowerStatus,
        BOOL refresh);

BOOL _CeGetStoreInformation2(
        RapiContext *context,
        LPSTORE_INFORMATION lpsi);

#if 0

BOOL _CeGetSystemMemoryDivision2(
        RapiContext *context,
        LPDWORD lpdwStoragePages,
    LPDWORD lpdwRamPages,
    LPDWORD lpdwPageSize);

#else

BOOL _NotImplementedCeGetSystemMemoryDivision2(
        RapiContext *context,
	LPDWORD lpdwStoragePages,
	LPDWORD lpdwRamPages,
	LPDWORD lpdwPageSize);

#endif

LONG _CeRegCreateKeyEx2(
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

LONG _CeRegOpenKeyEx2(
        RapiContext *context,
        HKEY hKey,
        LPCWSTR lpszSubKey,
        DWORD ulOptions,
        REGSAM samDesired,
        PHKEY phkResult);

LONG _CeRegEnumValue2( 
        RapiContext *context,
        HKEY hKey, 
        DWORD dwIndex, 
        LPWSTR lpszValueName, 
        LPDWORD lpcbValueName, 
        LPDWORD lpReserved, 
        LPDWORD lpType, 
        LPBYTE lpData, 
        LPDWORD lpcbData); 

LONG _CeRegEnumKeyEx2(
        RapiContext *context,
        HKEY hKey,
        DWORD dwIndex,
        LPWSTR lpName,
        LPDWORD lpcbName,
        LPDWORD lpReserved,
        LPWSTR lpClass,
        LPDWORD lpcbClass,
        PFILETIME lpftLastWriteTime);

LONG _CeRegQueryValueEx2(
        RapiContext *context,
        HKEY hKey,
        LPCWSTR lpValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE lpData,
        LPDWORD lpcbData);

LONG _CeRegSetValueEx2(
        RapiContext *context,
        HKEY hKey,
        LPCWSTR lpValueName,
        DWORD Reserved,
        DWORD dwType,
        const BYTE *lpData,
        DWORD cbData);

LONG _CeRegCloseKey2(
        RapiContext *context,
        HKEY hKey);

LONG _CeRegDeleteKey2(
        RapiContext *context,
        HKEY hKey,
        LPCWSTR lpszSubKey);

LONG _CeRegDeleteValue2(
        RapiContext *context,
        HKEY hKey,
        LPCWSTR lpszValueName);

HRESULT _CeProcessConfig2(
        RapiContext *context,
        LPCWSTR config, DWORD flags, LPWSTR* reply);

BOOL _CeStartReplication2(
        RapiContext *context);

HRESULT _CeSyncStart2(
        RapiContext *context,
        LPCWSTR params);

HRESULT _CeSyncResume2(
        RapiContext *context);

HRESULT _CeSyncPause2(
        RapiContext *context);

BOOL _CeSyncTimeToPc2(
        RapiContext *context);

DWORD _CeGetDiskFreeSpaceEx2(
        RapiContext *context,
		LPCTSTR _lpDirectoryName, 
		PULARGE_INTEGER lpFreeBytesAvailable, 
		PULARGE_INTEGER lpTotalNumberOfBytes, 
		PULARGE_INTEGER lpTotalNumberOfFreeBytes);

BOOL _NotImplementedCeSetFileAttributes2(
        RapiContext *context,
	LPCWSTR lpFileName,
	DWORD dwFileAttributes);

BOOL _NotImplementedCeSHCreateShortcut2(
        RapiContext *context,
	LPCWSTR lpszShortcut,
	LPCWSTR lpszTarget);

BOOL _NotImplementedCeRegCopyFile2(
        RapiContext *context,
        LPCWSTR filename);

BOOL _NotImplementedCeRegRestoreFile2(
        RapiContext *context,
        LPCWSTR filename);

BOOL _NotImplementedCeKillAllApps2(
        RapiContext *context);

DWORD _NotImplementedCeSetSystemMemoryDivision2(
        RapiContext *context,
	DWORD dwStoragePages);

BOOL _NotImplementedCeOidGetInfo2(
        RapiContext *context,
	CEOID oid,
	CEOIDINFO *poidInfo);

BOOL _NotImplementedCeCheckPassword2(
        RapiContext *context,
	LPWSTR lpszPassword);

CEOID _NotImplementedCeCreateDatabase2(
        RapiContext *context,
	LPWSTR lpszName,
	DWORD dwDbaseType,
	WORD wNumSortOrder,
	SORTORDERSPEC *rgSortSpecs);

BOOL _NotImplementedCeDeleteDatabase2(
        RapiContext *context,
	CEOID oid);

BOOL _NotImplementedCeFindAllDatabases2(
        RapiContext *context,
	DWORD dwDbaseType,
	WORD wFlags,
	LPWORD cFindData,
	LPLPCEDB_FIND_DATA ppFindData);

HANDLE _NotImplementedCeFindFirstDatabase2(
        RapiContext *context,
	DWORD dwDbaseType);

CEOID _NotImplementedCeFindNextDatabase2(
        RapiContext *context,
	HANDLE hEnum);

HANDLE _NotImplementedCeOpenDatabase2(
        RapiContext *context,
	PCEOID poid,
	LPWSTR lpszName,
	CEPROPID propid,
	DWORD dwFlags,
	HWND hwndNotify);

CEOID _NotImplementedCeReadRecordProps2(
        RapiContext *context,
	HANDLE hDbase,
	DWORD dwFlags,
	LPWORD lpcPropID,
	CEPROPID *rgPropID,
	LPBYTE *lplpBuffer,
	LPDWORD lpcbBuffer);

CEOID _NotImplementedCeWriteRecordProps2(
        RapiContext *context,
	HANDLE hDbase,
	CEOID oidRecord,
	WORD cPropID,
	CEPROPVAL* rgPropVal);

CEOID _NotImplementedCeSeekDatabase2(
        RapiContext *context,
	HANDLE hDatabase,
	DWORD dwSeekType,
	DWORD dwValue,
	LPDWORD lpdwIndex);

BOOL _NotImplementedCeDeleteRecord2(
        RapiContext *context,
	HANDLE hDatabase,
	CEOID oidRecord);

BOOL _NotImplementedCeSetDatabaseInfo2(
        RapiContext *context,
	CEOID oidDbase,
	CEDBASEINFO* pNewInfo);

/*
 * CeRapiInvoke stuff
 */

#ifndef SWIG

HRESULT _CeRapiInvoke2(
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

#endif /* __backend_ops_2_h__ */
