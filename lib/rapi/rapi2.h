/* $Id: rapi2.h 3813 2009-07-21 17:56:08Z mark_ellis $ */
#ifndef __rapi2_h__
#define __rapi2_h__

#include <synce.h>
#include <rapitypes.h>
#include <rapitypes2.h>
#include <irapistream.h>

#ifdef __cplusplus
namespace synce
{
extern "C"
{
#endif


struct _IRAPISession;
typedef struct _IRAPISession IRAPISession;
struct _IRAPIDevice;
typedef struct _IRAPIDevice IRAPIDevice;
struct _IRAPIEnumDevices;
typedef struct _IRAPIEnumDevices IRAPIEnumDevices;
struct _IRAPIDesktop;
typedef struct _IRAPIDesktop IRAPIDesktop;


/* IRAPISession */

void IRAPISession_AddRef(IRAPISession *session);

void IRAPISession_Release(IRAPISession *session);

IRAPIDevice * IRAPISession_get_device(IRAPISession *session);

HRESULT IRAPISession_CeRapiFreeBuffer(IRAPISession *session,
                                      LPVOID Buffer);

HRESULT IRAPISession_CeRapiInit(IRAPISession *session);

HRESULT IRAPISession_CeRapiUninit(IRAPISession *session);

HRESULT IRAPISession_CeRapiGetError(IRAPISession *session);

DWORD IRAPISession_CeGetLastError(IRAPISession *session);


/*
 * File access functions
 */


BOOL IRAPISession_CeCloseHandle(IRAPISession *session,
		HANDLE hObject);

HANDLE IRAPISession_CeCreateFile(IRAPISession *session,
		LPCWSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile);

BOOL IRAPISession_CeReadFile(IRAPISession *session,
		HANDLE hFile,
		LPVOID lpBuffer,
		DWORD nNumberOfBytesToRead,
		LPDWORD lpNumberOfBytesRead,
		LPOVERLAPPED lpOverlapped);

BOOL IRAPISession_CeWriteFile(IRAPISession *session,
		HANDLE hFile,
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten,
		LPOVERLAPPED lpOverlapped);

DWORD IRAPISession_CeSetFilePointer(IRAPISession *session,
		HANDLE hFile,
		LONG lDistanceToMove,
		PLONG lpDistanceToMoveHigh,
		DWORD dwMoveMethod);

BOOL IRAPISession_CeSetEndOfFile(IRAPISession *session,
                HANDLE hObject);

BOOL IRAPISession_CeGetFileTime(IRAPISession *session,
		HANDLE hFile,
		LPFILETIME lpCreationTime,
		LPFILETIME lpLastAccessTime,
		LPFILETIME lpLastWriteTime);

BOOL IRAPISession_CeSetFileTime(IRAPISession *session,
		HANDLE hFile,
		LPFILETIME lpCreationTime,
		LPFILETIME lpLastAccessTime,
		LPFILETIME lpLastWriteTime);

/*
 * File management functions
 */

BOOL IRAPISession_CeCopyFile(IRAPISession *session,
		LPCWSTR lpExistingFileName,
		LPCWSTR lpNewFileName,
		BOOL bFailIfExists);

BOOL IRAPISession_CeCreateDirectory(IRAPISession *session,
		LPCWSTR lpPathName,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes);

BOOL IRAPISession_CeDeleteFile(IRAPISession *session,
		LPCWSTR lpFileName);

BOOL IRAPISession_CeFindAllFiles(IRAPISession *session,
		LPCWSTR szPath,
		DWORD dwFlags,
		LPDWORD lpdwFoundCount,
		LPLPCE_FIND_DATA ppFindDataArray);

HANDLE IRAPISession_CeFindFirstFile(IRAPISession *session,
		LPCWSTR lpFileName,
		LPCE_FIND_DATA lpFindFileData);

BOOL IRAPISession_CeFindNextFile(IRAPISession *session,
		HANDLE hFindFile,
		LPCE_FIND_DATA lpFindFileData);

BOOL IRAPISession_CeFindClose(IRAPISession *session,
		HANDLE hFindFile);

DWORD IRAPISession_CeGetFileAttributes(IRAPISession *session,
		LPCWSTR lpFileName);

DWORD IRAPISession_CeGetFileSize(IRAPISession *session,
		HANDLE hFile,
		LPDWORD lpFileSizeHigh);

DWORD IRAPISession_CeGetSpecialFolderPath(IRAPISession *session,
		int nFolder,
		DWORD nBufferLength,
		LPWSTR lpBuffer);

BOOL IRAPISession_CeMoveFile(IRAPISession *session,
		LPCWSTR lpExistingFileName,
		LPCWSTR lpNewFileName);

BOOL IRAPISession_CeRemoveDirectory(IRAPISession *session,
		LPCWSTR lpPathName);

BOOL IRAPISession_CeSetFileAttributes(IRAPISession *session,
		LPCWSTR lpFileName,
		DWORD dwFileAttributes);

BOOL IRAPISession_CeSHCreateShortcut(IRAPISession *session,
		LPCWSTR lpszShortcut,
		LPCWSTR lpszTarget);

BOOL IRAPISession_CeSyncTimeToPc(IRAPISession *session);

/*
 * Database functions
 */

CEOID IRAPISession_CeCreateDatabase(IRAPISession *session,
		LPWSTR lpszName,
		DWORD dwDbaseType,
		WORD wNumSortOrder,
		SORTORDERSPEC *rgSortSpecs);

BOOL IRAPISession_CeDeleteDatabase(IRAPISession *session,
		CEOID oid);

BOOL IRAPISession_CeFindAllDatabases(IRAPISession *session,
		DWORD dwDbaseType,
		WORD wFlags,
		LPWORD cFindData,
		LPLPCEDB_FIND_DATA ppFindData);

HANDLE IRAPISession_CeFindFirstDatabase(IRAPISession *session,
		DWORD dwDbaseType);

CEOID IRAPISession_CeFindNextDatabase(IRAPISession *session,
		HANDLE hEnum);

HANDLE IRAPISession_CeOpenDatabase(IRAPISession *session,
		PCEOID poid,
		LPWSTR lpszName,
		CEPROPID propid,
		DWORD dwFlags,
		HWND hwndNotify);

CEOID IRAPISession_CeReadRecordProps(IRAPISession *session,
		HANDLE hDbase,
		DWORD dwFlags,
		LPWORD lpcPropID,
		CEPROPID *rgPropID,
		LPBYTE *lplpBuffer,
		LPDWORD lpcbBuffer);

CEOID IRAPISession_CeSeekDatabase(IRAPISession *session,
		HANDLE hDatabase,
		DWORD dwSeekType,
		DWORD dwValue,
		LPDWORD lpdwIndex);

CEOID IRAPISession_CeWriteRecordProps(IRAPISession *session,
		HANDLE hDbase,
		CEOID oidRecord,
		WORD cPropID,
		CEPROPVAL *rgPropVal);

BOOL IRAPISession_CeDeleteRecord(IRAPISession *session,
		HANDLE hDatabase,
		CEOID oidRecord);

BOOL IRAPISession_CeSetDatabaseInfo(IRAPISession *session,
		CEOID oidDbase,
		CEDBASEINFO* pNewInfo);

/*
 * Registry
 */

LONG IRAPISession_CeRegCreateKeyEx(IRAPISession *session,
		HKEY hKey,
		LPCWSTR lpszSubKey,
		DWORD Reserved,
		LPWSTR lpszClass,
		DWORD ulOptions,
		REGSAM samDesired,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		PHKEY phkResult,
		LPDWORD lpdwDisposition);

LONG IRAPISession_CeRegOpenKeyEx(IRAPISession *session,
		HKEY hKey,
		LPCWSTR lpszSubKey,
		DWORD ulOptions,
		REGSAM samDesired,
		PHKEY phkResult);

LONG IRAPISession_CeRegCloseKey(IRAPISession *session,
		HKEY hKey);

LONG IRAPISession_CeRegDeleteKey(IRAPISession *session,
		HKEY hKey,
		LPCWSTR lpszSubKey);

LONG IRAPISession_CeRegDeleteValue(IRAPISession *session,
		HKEY hKey,
		LPCWSTR lpszValueName);

LONG IRAPISession_CeRegQueryInfoKey(IRAPISession *session,
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

LONG IRAPISession_CeRegQueryValueEx(IRAPISession *session,
		HKEY hKey,
		LPCWSTR lpValueName,
		LPDWORD lpReserved,
		LPDWORD lpType,
		LPBYTE lpData,
		LPDWORD lpcbData);

LONG IRAPISession_CeRegEnumValue(IRAPISession *session,
		HKEY hKey,
		DWORD dwIndex,
		LPWSTR lpszValueName,
		LPDWORD lpcbValueName,
		LPDWORD lpReserved,
		LPDWORD lpType,
		LPBYTE lpData,
		LPDWORD lpcbData);

LONG IRAPISession_CeRegEnumKeyEx(IRAPISession *session,
		HKEY hKey,
		DWORD dwIndex,
		LPWSTR lpName,
		LPDWORD lpcbName,
		LPDWORD lpReserved,
		LPWSTR lpClass,
		LPDWORD lpcbClass,
		PFILETIME lpftLastWriteTime);

LONG IRAPISession_CeRegSetValueEx(IRAPISession *session,
		HKEY hKey,
		LPCWSTR lpValueName,
		DWORD Reserved,
		DWORD dwType,
		const BYTE *lpData,
		DWORD cbData);


/*
 * Misc functions
 */

/* Flags for CeProcessConfig */

#define CONFIG_PROCESS_DOCUMENT   1
#define CONFIG_RETURN_METADATA    2


BOOL IRAPISession_CeCheckPassword(IRAPISession *session,
		LPWSTR lpszPassword);

BOOL IRAPISession_CeCreateProcess(IRAPISession *session,
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

BOOL IRAPISession_CeGetStoreInformation(IRAPISession *session,
		LPSTORE_INFORMATION lpsi);

void IRAPISession_CeGetSystemInfo(IRAPISession *session,
		LPSYSTEM_INFO lpSystemInfo);

BOOL IRAPISession_CeGetSystemPowerStatusEx(IRAPISession *session, PSYSTEM_POWER_STATUS_EX pSystemPowerStatus, BOOL refresh);

BOOL IRAPISession_CeGetVersionEx(IRAPISession *session,
		LPCEOSVERSIONINFO lpVersionInformation);

BOOL IRAPISession_CeOidGetInfo(IRAPISession *session,
		CEOID oid,
		CEOIDINFO *poidInfo);

HRESULT IRAPISession_CeProcessConfig(IRAPISession *session, LPCWSTR config, DWORD flags, LPWSTR* reply);

BOOL IRAPISession_CeStartReplication(IRAPISession *session);

HRESULT IRAPISession_CeSyncStart(IRAPISession *session, LPCWSTR params);

HRESULT IRAPISession_CeSyncResume(IRAPISession *session);

HRESULT IRAPISession_CeSyncPause(IRAPISession *session);

BOOL IRAPISession_CeGetSystemMemoryDivision(IRAPISession *session,
    LPDWORD lpdwStoragePages,
    LPDWORD lpdwRamPages,
    LPDWORD lpdwPageSize);

DWORD IRAPISession_CeSetSystemMemoryDivision(IRAPISession *session,
    DWORD dwStoragePages);

BOOL IRAPISession_CeRegCopyFile(IRAPISession *session, LPCWSTR filename);
BOOL IRAPISession_CeRegRestoreFile(IRAPISession *session, LPCWSTR filename);

BOOL IRAPISession_CeKillAllApps(IRAPISession *session);

DWORD IRAPISession_CeGetDiskFreeSpaceEx(IRAPISession *session,
		LPCTSTR lpDirectoryName, 
		PULARGE_INTEGER lpFreeBytesAvailable, 
		PULARGE_INTEGER lpTotalNumberOfBytes, 
		PULARGE_INTEGER lpTotalNumberOfFreeBytes);


/*
 * CeRapiInvoke stuff
 */

HRESULT IRAPISession_CeRapiInvoke(IRAPISession *session,
		LPCWSTR pDllPath,
		LPCWSTR pFunctionName,
		DWORD cbInput,
		const BYTE *pInput,
		DWORD *pcbOutput,
		BYTE **ppOutput,
		IRAPIStream **ppIRAPIStream,
		DWORD dwReserved);





/* IRAPIDevice */

void IRAPIDevice_AddRef(IRAPIDevice *self);

void IRAPIDevice_Release(IRAPIDevice *self);


HRESULT IRAPIDevice_CreateSession(IRAPIDevice *self, IRAPISession** ppISession);

HRESULT IRAPIDevice_GetConnectionInfo(IRAPIDevice *self, RAPI_CONNECTIONINFO* pConnInfo);

HRESULT IRAPIDevice_GetConnectStat(IRAPIDevice *self, RAPI_DEVICESTATUS* pStat);

HRESULT IRAPIDevice_GetDeviceInfo(IRAPIDevice *self, RAPI_DEVICEINFO* pDevInfo);

const char * IRAPIDevice_get_name(IRAPIDevice *self);
bool IRAPIDevice_get_os_version(IRAPIDevice *self, unsigned int *os_major, unsigned int *os_minor);
unsigned int IRAPIDevice_get_build_number(IRAPIDevice *self);
unsigned int IRAPIDevice_get_processor_type(IRAPIDevice *self);
const char * IRAPIDevice_get_os_name(IRAPIDevice *self);
const char * IRAPIDevice_get_model(IRAPIDevice *self);
const char * IRAPIDevice_get_device_ip(IRAPIDevice *self);
const char * IRAPIDevice_get_local_ip(IRAPIDevice *self);


/* IRAPIEnumDevices */

void IRAPIEnumDevices_AddRef(IRAPIEnumDevices *self);

void IRAPIEnumDevices_Release(IRAPIEnumDevices *self);


HRESULT IRAPIEnumDevices_Clone(IRAPIEnumDevices *self, IRAPIEnumDevices** ppIEnum);

HRESULT IRAPIEnumDevices_GetCount(IRAPIEnumDevices *self, ULONG* pcElt);

HRESULT IRAPIEnumDevices_Next(IRAPIEnumDevices *self, IRAPIDevice** ppIDevice);

HRESULT IRAPIEnumDevices_Reset(IRAPIEnumDevices *self);

HRESULT IRAPIEnumDevices_Skip(IRAPIEnumDevices *self, ULONG cElt);


/* IRAPISink */

/* These functions gain a reference to the device object, and should free the
 * reference if they dont want it
 */

struct _IRAPISink;
typedef struct _IRAPISink IRAPISink;
struct _IRAPISink {
        HRESULT (*IRAPISink_OnDeviceConnected)(IRAPISink *self, IRAPIDevice* pIDevice);
        HRESULT (*IRAPISink_OnDeviceDisconnected)(IRAPISink *self, IRAPIDevice* pIDevice);
        void *user_data;
};



/* IRAPIDesktop */

HRESULT IRAPIDesktop_Get(IRAPIDesktop **ppIRAPIDesktop);

void IRAPIDesktop_AddRef(IRAPIDesktop *self);

void IRAPIDesktop_Release(IRAPIDesktop *self);

HRESULT IRAPIDesktop_Advise(IRAPIDesktop *self, IRAPISink* pISink, DWORD* pdwContext);

HRESULT IRAPIDesktop_EnumDevices(IRAPIDesktop *self, IRAPIEnumDevices** ppIEnum);

HRESULT IRAPIDesktop_FindDevice(IRAPIDesktop *self, RAPIDEVICEID *pDeviceID, RAPI_GETDEVICEOPCODE opFlags, IRAPIDevice** ppIDevice);

HRESULT IRAPIDesktop_UnAdvise(IRAPIDesktop *self, DWORD dwContext);



#ifdef __cplusplus
}
}
#endif

#endif /* __rapi2_h__ */

