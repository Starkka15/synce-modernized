/* $Id: rapi.h 4129 2012-11-20 16:43:57Z mark_ellis $ */
#ifndef __rapi_h__
#define __rapi_h__

/*
 * Note on #ifndef SWIG preprocessor token.
 *
 * The scripting langauge code in the pyrapi module uses
 * SWIG to generate the python wrapper code. To enable SWIG to
 * parse this header file some parts of it must be
 * selectively removed. SWIG defines "SWIG" as a
 * preproccessor token when is it parsing headers.
 * Therefore where you see #ifndef SWIG declarations
 * in this header they are intended to remove elements for
 * which the SWIG wrapper provides alternative
 * declarations. In most cases this is type and function
 * declarations.
 */

#ifndef SWIG
#include <synce.h>
#include <rapitypes.h>
#include <irapistream.h>
#endif

#ifdef __cplusplus
namespace synce
{
extern "C"
{
#endif

/*
 * SynCE support for switching between multiple devices
 *
 * Example code for two devices follows!
 *
 * It shows two different ways to get a RapiConnection object.
 *
 *   RapiConnection* a = rapi_connection_from_name("device_a");
 *   rapi_connection_select(a);
 *   CeRapiInit()
 *
 *   SynceInfo* info_b = synce_info_new("device_b");
 *   RapiConnection* b = rapi_connection_from_info(info_b);
 *   rapi_connection_select(b);
 *   CeRapiInit()
 *
 *   rapi_connection_select(a);
 *   ...some RAPI calls to device A...
 *
 *   rapi_connection_select(b);
 *   ...some RAPI calls to device B...
 *
 *   rapi_connection_select(a);
 *   CeRapiUninit();
 *   rapi_connection_destroy(a);
 *
 *   rapi_connection_select(b);
 *   CeRapiUninit();
 *   rapi_connection_destroy(b);
 *   synce_info_destroy(info_b);
 */

typedef struct _RapiConnection RapiConnection;

/**
 * Create a connection - use this before you call CeRapiInit()
 *
 * @path Parameter sent to synce_info_new() in libsynce/lib/info.c
 */
RapiConnection* rapi_connection_from_name(const char* device_name);

/**
 * Create a connection - low-level version
 */
RapiConnection* rapi_connection_from_info(SynceInfo* info);

/** Select what connection is used for RAPI calls */
void rapi_connection_select(RapiConnection* connection);

/** Destroy connection object - use this after CeRapiUninit() */
void rapi_connection_destroy(RapiConnection* connection);

/** get information about the connected device */
const char *rapi_connection_get_name(RapiConnection* connection);
bool rapi_connection_get_os_version(RapiConnection* connection, unsigned int *os_major, unsigned int *os_minor);
unsigned int rapi_connection_get_build_number(RapiConnection* connection);
unsigned int rapi_connection_get_processor_type(RapiConnection* connection);
const char *rapi_connection_get_os_name(RapiConnection* connection);
const char *rapi_connection_get_model(RapiConnection* connection);
const char *rapi_connection_get_device_ip(RapiConnection* connection);
const char *rapi_connection_get_local_ip(RapiConnection* connection);
/** this should not be used directly for data, only for monitoring purposes */
int rapi_connection_get_fd(RapiConnection* connection);

/*
 * Main RAPI functions
 */

#ifndef SWIG

HRESULT CeRapiFreeBuffer(
		LPVOID Buffer);

HRESULT CeRapiInit(void);

STDAPI CeRapiUninit(void);

HRESULT CeRapiGetError(void);

/** Not part of the real RAPI.
 * Use it instead of Win32's GetLastError() */
DWORD CeRapiGetLastError();

#endif /* SWIG */


/*
 * File access functions
 */

#ifndef SWIG

BOOL CeCloseHandle(
		HANDLE hObject);

HANDLE CeCreateFile(
		LPCWSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile);

BOOL CeReadFile(
		HANDLE hFile,
		LPVOID lpBuffer,
		DWORD nNumberOfBytesToRead,
		LPDWORD lpNumberOfBytesRead,
		LPOVERLAPPED lpOverlapped);

BOOL CeWriteFile(
		HANDLE hFile,
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten,
		LPOVERLAPPED lpOverlapped);

DWORD CeSetFilePointer(
		HANDLE hFile,
		LONG lDistanceToMove,
		PLONG lpDistanceToMoveHigh,
		DWORD dwMoveMethod);

BOOL CeSetEndOfFile(
                HANDLE hObject);

BOOL CeGetFileTime(
		HANDLE hFile,
		LPFILETIME lpCreationTime,
		LPFILETIME lpLastAccessTime,
		LPFILETIME lpLastWriteTime);

BOOL CeSetFileTime(
		HANDLE hFile,
		LPFILETIME lpCreationTime,
		LPFILETIME lpLastAccessTime,
		LPFILETIME lpLastWriteTime);

#endif /* SWIG */


/*
 * File management functions
 */

#ifndef SWIG

/* deprecated, use rapi_copy_file() */
BOOL CeCopyFileA(
		LPCSTR lpExistingFileName,
		LPCSTR lpNewFileName,
		BOOL bFailIfExists);

BOOL CeCopyFile(
		LPCWSTR lpExistingFileName,
		LPCWSTR lpNewFileName,
		BOOL bFailIfExists);

BOOL CeCreateDirectory(
		LPCWSTR lpPathName,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes);

BOOL CeDeleteFile(
		LPCWSTR lpFileName);

BOOL CeFindAllFiles(
		LPCWSTR szPath,
		DWORD dwFlags,
		LPDWORD lpdwFoundCount,
		LPLPCE_FIND_DATA ppFindDataArray);

HANDLE CeFindFirstFile(
		LPCWSTR lpFileName,
		LPCE_FIND_DATA lpFindFileData);

BOOL CeFindNextFile(
		HANDLE hFindFile,
		LPCE_FIND_DATA lpFindFileData);

BOOL CeFindClose(
		HANDLE hFindFile);

DWORD CeGetFileAttributes(
		LPCWSTR lpFileName);

DWORD CeGetFileSize(
		HANDLE hFile,
		LPDWORD lpFileSizeHigh);

DWORD CeGetSpecialFolderPath(
		int nFolder,
		DWORD nBufferLength,
		LPWSTR lpBuffer);

BOOL CeMoveFile(
		LPCWSTR lpExistingFileName,
		LPCWSTR lpNewFileName);

BOOL CeRemoveDirectory(
		LPCWSTR lpPathName);

BOOL CeSetFileAttributes(
		LPCWSTR lpFileName,
		DWORD dwFileAttributes);

BOOL CeSHCreateShortcut(
		LPCWSTR lpszShortcut,
		LPCWSTR lpszTarget);

BOOL CeSyncTimeToPc();

bool rapi_copy_file(
		const char *source_file_name,
		const char *dest_file_name,
		bool fail_if_exists);

#endif /* SWIG */


/*
 * Database functions
 */

#ifndef SWIG

CEOID CeCreateDatabase(
		LPWSTR lpszName,
		DWORD dwDbaseType,
		WORD wNumSortOrder,
		SORTORDERSPEC *rgSortSpecs);

BOOL CeDeleteDatabase(
		CEOID oid);

BOOL CeFindAllDatabases(
		DWORD dwDbaseType,
		WORD wFlags,
		LPWORD cFindData,
		LPLPCEDB_FIND_DATA ppFindData);

HANDLE CeFindFirstDatabase(
		DWORD dwDbaseType);

CEOID CeFindNextDatabase(
		HANDLE hEnum);

HANDLE CeOpenDatabase(
		PCEOID poid,
		LPWSTR lpszName,
		CEPROPID propid,
		DWORD dwFlags,
		HWND hwndNotify);

CEOID CeReadRecordProps(
		HANDLE hDbase,
		DWORD dwFlags,
		LPWORD lpcPropID,
		CEPROPID *rgPropID,
		LPBYTE *lplpBuffer,
		LPDWORD lpcbBuffer);

CEOID CeSeekDatabase(
		HANDLE hDatabase,
		DWORD dwSeekType,
		DWORD dwValue,
		LPDWORD lpdwIndex);

CEOID CeWriteRecordProps(
		HANDLE hDbase,
		CEOID oidRecord,
		WORD cPropID,
		CEPROPVAL *rgPropVal);

BOOL CeDeleteRecord(
		HANDLE hDatabase,
		CEOID oidRecord);

BOOL CeSetDatabaseInfo(
		CEOID oidDbase,
		CEDBASEINFO* pNewInfo);

#endif /* SWIG */

/*
 * Registry
 */

#ifndef SWIG

LONG CeRegCreateKeyEx(
		HKEY hKey,
		LPCWSTR lpszSubKey,
		DWORD Reserved,
		LPWSTR lpszClass,
		DWORD ulOptions,
		REGSAM samDesired,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		PHKEY phkResult,
		LPDWORD lpdwDisposition);

LONG CeRegOpenKeyEx(
		HKEY hKey,
		LPCWSTR lpszSubKey,
		DWORD ulOptions,
		REGSAM samDesired,
		PHKEY phkResult);

LONG CeRegCloseKey(
		HKEY hKey);

LONG CeRegDeleteKey(
		HKEY hKey,
		LPCWSTR lpszSubKey);

LONG CeRegDeleteValue(
		HKEY hKey,
		LPCWSTR lpszValueName);

LONG CeRegQueryInfoKey(
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

LONG CeRegQueryValueEx(
		HKEY hKey,
		LPCWSTR lpValueName,
		LPDWORD lpReserved,
		LPDWORD lpType,
		LPBYTE lpData,
		LPDWORD lpcbData);

LONG CeRegEnumValue(
		HKEY hKey,
		DWORD dwIndex,
		LPWSTR lpszValueName,
		LPDWORD lpcbValueName,
		LPDWORD lpReserved,
		LPDWORD lpType,
		LPBYTE lpData,
		LPDWORD lpcbData);

LONG CeRegEnumKeyEx(
		HKEY hKey,
		DWORD dwIndex,
		LPWSTR lpName,
		LPDWORD lpcbName,
		LPDWORD lpReserved,
		LPWSTR lpClass,
		LPDWORD lpcbClass,
		PFILETIME lpftLastWriteTime);

LONG CeRegSetValueEx(
		HKEY hKey,
		LPCWSTR lpValueName,
		DWORD Reserved,
		DWORD dwType,
		const BYTE *lpData,
		DWORD cbData);

/*
 * Convenience functions for easy registry access
 */

bool rapi_reg_create_key(
		HKEY parent,
		const char* name,
		HKEY* key);

bool rapi_reg_open_key(
		HKEY parent, const char* name, HKEY* key);

bool rapi_reg_query_dword(
		HKEY key,
		const char* name,
		DWORD* value);

bool rapi_reg_query_string(
		HKEY key,
		const char* name,
		char** value);

#define rapi_reg_free_string(str) wstr_free_string(str)

bool rapi_reg_set_dword(
		HKEY key,
		const char* name,
		DWORD value);

bool rapi_reg_set_string(
		HKEY key,
		const char* name,
		const char *value);

#endif /* SWIG */

/*
 * Misc functions
 */

/* Flags for CeProcessConfig */

#define CONFIG_PROCESS_DOCUMENT   1
#define CONFIG_RETURN_METADATA    2

#ifndef SWIG

BOOL CeCheckPassword(
		LPWSTR lpszPassword);

BOOL CeCreateProcess(
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

DWORD CeGetLastError( void );

BOOL CeGetStoreInformation(
		LPSTORE_INFORMATION lpsi);

void CeGetSystemInfo(
		LPSYSTEM_INFO lpSystemInfo);

BOOL CeGetSystemPowerStatusEx(PSYSTEM_POWER_STATUS_EX pSystemPowerStatus, BOOL refresh);

BOOL CeGetVersionEx(
		LPCEOSVERSIONINFO lpVersionInformation);

BOOL CeOidGetInfo(
		CEOID oid,
		CEOIDINFO *poidInfo);

HRESULT CeProcessConfig(LPCWSTR config, DWORD flags, LPWSTR* reply);

BOOL CeStartReplication( void );

HRESULT CeSyncStart( LPCWSTR params );

HRESULT CeSyncResume( void );

HRESULT CeSyncPause( void );

BOOL CeGetSystemMemoryDivision(
    LPDWORD lpdwStoragePages,
    LPDWORD lpdwRamPages,
    LPDWORD lpdwPageSize);

DWORD CeSetSystemMemoryDivision(
    DWORD dwStoragePages);

BOOL CeRegCopyFile(LPCWSTR filename);
BOOL CeRegRestoreFile(LPCWSTR filename);

BOOL CeKillAllApps();

DWORD CeGetDiskFreeSpaceEx( 
		LPCTSTR lpDirectoryName, 
		PULARGE_INTEGER lpFreeBytesAvailable, 
		PULARGE_INTEGER lpTotalNumberOfBytes, 
		PULARGE_INTEGER lpTotalNumberOfFreeBytes);

#endif /* SWIG */

/*
 * CeRapiInvoke stuff
 */

#ifndef SWIG

HRESULT CeRapiInvoke(
		LPCWSTR pDllPath,
		LPCWSTR pFunctionName,
		DWORD cbInput,
		const BYTE *pInput,
		DWORD *pcbOutput,
		BYTE **ppOutput,
		IRAPIStream **ppIRAPIStream,
		DWORD dwReserved);

/* deprecated, use rapi_invoke() */
HRESULT CeRapiInvokeA(
		LPCSTR pDllPath,
		LPCSTR pFunctionName,
		DWORD cbInput,
		const BYTE *pInput,
		DWORD *pcbOutput,
		BYTE **ppOutput,
		IRAPIStream **ppIRAPIStream,
		DWORD dwReserved);

HRESULT rapi_invoke( /*{{{*/
		const char *dll_path,
		const char *function_name,
		DWORD cbInput,
		const BYTE *pInput,
		DWORD *pcbOutput,
		BYTE **ppOutput,
		IRAPIStream **ppIRAPIStream,
		DWORD dwReserved);

#endif /* SWIG */

#ifdef __cplusplus
}
}
#endif

#endif

