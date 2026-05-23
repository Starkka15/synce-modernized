cdef extern from "stddef.h":
    ctypedef unsigned int       size_t

cdef extern from "stdint.h":
    ctypedef unsigned char      uint8_t
    ctypedef short int          int16_t
    ctypedef unsigned short int uint16_t
    ctypedef int                int32_t
    # FIXME: this isn't portable
    ctypedef unsigned long      uint32_t
    ctypedef long int           int64_t
    ctypedef unsigned long long	uint64_t

cdef extern from "synce.h":
    ctypedef void               VOID

    ctypedef uint8_t            BYTE
    ctypedef BYTE               BOOLEAN

    ctypedef int16_t            CSHORT

    ctypedef uint16_t           WORD
    ctypedef uint16_t           USHORT
    ctypedef uint16_t           WCHAR
    ctypedef WCHAR              TCHAR

    ctypedef int32_t            LONG
    ctypedef int32_t            HKEY
    ctypedef int32_t            REGSAM

    ctypedef uint32_t           DWORD
    ctypedef uint32_t           UINT
    ctypedef uint32_t           ULONG
    ctypedef uint32_t           HWND
    ctypedef uint32_t           BOOL

    ctypedef int64_t            LONGLONG
    ctypedef uint64_t			ULARGE_INTEGER
    
    ctypedef uint64_t*			PULARGE_INTEGER

    # XXX: sizeof(double) must be 8 */
    ctypedef double             DATE

    ctypedef void*              LPVOID
    ctypedef char*              LPSTR
    ctypedef BYTE*              LPBYTE
    ctypedef WORD*              LPWORD
    ctypedef WCHAR*             LPWSTR
    ctypedef HKEY*              PHKEY
    ctypedef DWORD*             LPDWORD
    ctypedef LONG*              PLONG

    ctypedef void*              LPCVOID
    ctypedef char*              LPCSTR
    ctypedef char*              LPCTSTR
    ctypedef WCHAR*             LPCWSTR

    ctypedef int32_t            HRESULT
    ctypedef HRESULT            STDAPI
    ctypedef uint32_t           HANDLE


    ctypedef struct _FILETIME:
        DWORD dwLowDateTime
        DWORD dwHighDateTime

    ctypedef _FILETIME  FILETIME
    ctypedef FILETIME*  PFILETIME
    ctypedef FILETIME*  LPFILETIME

cdef extern from "rapitypes.h":
    ctypedef void*              LPSECURITY_ATTRIBUTES
    ctypedef void*				LPOVERLAPPED
    ctypedef struct _SYSTEM_POWER_STATUS_EX: 
        BYTE ACLineStatus
        BYTE BatteryFlag
        BYTE BatteryLifePercent
        BYTE Reserved1
        DWORD BatteryLifeTime
        DWORD BatteryFullLifeTime
        BYTE Reserved2
        BYTE BackupBatteryFlag
        BYTE BackupBatteryLifePercent
        BYTE Reserved3
        DWORD BackupBatteryLifeTime
        DWORD BackupBatteryFullLifeTime
    ctypedef _SYSTEM_POWER_STATUS_EX	SYSTEM_POWER_STATUS_EX 
    ctypedef SYSTEM_POWER_STATUS_EX*	PSYSTEM_POWER_STATUS_EX
    
    ctypedef struct _CE_FIND_DATA:
        DWORD dwFileAttributes
        FILETIME ftCreationTime
        FILETIME ftLastAccessTime
        FILETIME ftLastWriteTime
        DWORD nFileSizeHigh
        DWORD nFileSizeLow
        DWORD dwOID
        WCHAR cFileName[256]
    ctypedef _CE_FIND_DATA  CE_FIND_DATA
    ctypedef CE_FIND_DATA*  LPCE_FIND_DATA
    ctypedef CE_FIND_DATA** LPLPCE_FIND_DATA

    ctypedef struct _PROCESS_INFORMATION:
        HANDLE hProcess
        HANDLE hThread
        DWORD dwProcessId
        DWORD dwThreadId
    ctypedef _PROCESS_INFORMATION   PROCESS_INFORMATION
    ctypedef PROCESS_INFORMATION*   LPPROCESS_INFORMATION

    ctypedef struct _SYSTEM_INFO:
        WORD wProcessorArchitecture
        WORD wReserved
        DWORD dwPageSize
        ULONG lpMinimumApplicationAddress
        ULONG lpMaximumApplicationAddress
        DWORD dwActiveProcessorMask
        DWORD dwNumberOfProcessors
        DWORD dwProcessorType
        DWORD dwAllocationGranularity
        WORD wProcessorLevel
        WORD wProcessorRevision
    ctypedef _SYSTEM_INFO       SYSTEM_INFO
    ctypedef SYSTEM_INFO*       LPSYSTEM_INFO

    ctypedef struct _CEOSVERSIONINFO:
        DWORD dwOSVersionInfoSize
        DWORD dwMajorVersion
        DWORD dwMinorVersion
        DWORD dwBuildNumber
        DWORD dwPlatformId
        WCHAR szCSDVersion[128]
    ctypedef _CEOSVERSIONINFO   CEOSVERSIONINFO
    ctypedef CEOSVERSIONINFO*   LPCEOSVERSIONINFO

