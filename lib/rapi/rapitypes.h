/* $Id: rapitypes.h 3791 2009-07-12 17:27:04Z mark_ellis $ */
#ifndef __rapitypes_h__
#define __rapitypes_h__

#include <synce.h>

#ifdef __cplusplus
namespace synce
{
extern "C"
{
#endif

/*
 * for the CeFind*Files functions
 */

typedef struct _CE_FIND_DATA {
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
	DWORD dwOID;
	WCHAR cFileName[MAX_PATH];
} CE_FIND_DATA, *LPCE_FIND_DATA, **LPLPCE_FIND_DATA;

#define FAF_ATTRIBUTES      0x00001
#define FAF_CREATION_TIME   0x00002
#define FAF_LASTACCESS_TIME   0x00004
#define FAF_LASTWRITE_TIME    0x00008

#define FAF_SIZE_HIGH     0x00010
#define FAF_SIZE_LOW      0x00020
#define FAF_OID       0x00040
#define FAF_NAME      0x00080

#define FAF_ATTRIB_CHILDREN   0x01000
#define FAF_ATTRIB_NO_HIDDEN    0x02000
#define FAF_FOLDERS_ONLY    0x04000
#define FAF_NO_HIDDEN_SYS_ROMMODULES  0x08000


/*
 * for the CeFind*Database functions
 */

#define FAD_OID                         0x0001
#define FAD_FLAGS                       0x0002
#define FAD_NAME                        0x0004
#define FAD_TYPE                        0x0008

#define FAD_NUM_RECORDS                 0x0010
#define FAD_NUM_SORT_ORDER              0x0020
#define FAD_SIZE                        0x0040
#define FAD_LAST_MODIFIED               0x0080

#define FAD_SORT_SPECS                  0x0100

/*
 * CeGetStoreInformation
 */
typedef struct STORE_INFORMATION {
	DWORD dwStoreSize;
	DWORD dwFreeSize;
} STORE_INFORMATION, *LPSTORE_INFORMATION;

/*
 * types for database and fileinfo functions
 */
typedef DWORD CEPROPID;
typedef CEPROPID *PCEPROPID;
typedef DWORD CEOID;
typedef CEOID *PCEOID;

typedef struct _CEFILEINFO {
	DWORD dwAttributes;
	CEOID oidParent;
	WCHAR szFileName[MAX_PATH];
	FILETIME ftLastChanged;
	DWORD dwLength;
} CEFILEINFO;

typedef struct _CEDIRINFO {
	DWORD dwAttributes;
	CEOID oidParent;
	WCHAR szDirName[MAX_PATH];
} CEDIRINFO;

typedef struct _CERECORDINFO {
	CEOID oidParent;
} CERECORDINFO;


#define CEDB_SORT_DESCENDING            0x00000001
#define CEDB_SORT_CASEINSENSITIVE       0x00000002
#define CEDB_SORT_UNKNOWNFIRST          0x00000004
#define CEDB_SORT_GENERICORDER          0x00000008

typedef struct _SORTORDERSPEC {
	CEPROPID propid;
	DWORD dwFlags;
} SORTORDERSPEC;

#define CEDB_MAXDBASENAMELEN    32
#define CEDB_MAXSORTORDER       4

#define CEDB_VALIDNAME          0x0001
#define CEDB_VALIDTYPE          0x0002
#define CEDB_VALIDSORTSPEC      0x0004
#define CEDB_VALIDMODTIME       0x0008

#define CEDB_NOCOMPRESS         0x00010000

typedef struct _CEDBASEINFO {
	DWORD dwFlags;
	WCHAR szDbaseName[CEDB_MAXDBASENAMELEN];
	DWORD dwDbaseType;
	WORD wNumRecords;
	WORD wNumSortOrder;
	DWORD dwSize;
	FILETIME ftLastModified;
	SORTORDERSPEC rgSortSpecs[CEDB_MAXSORTORDER];
} CEDBASEINFO;

typedef struct _CEDB_FIND_DATA {
	CEOID OidDb;
	CEDBASEINFO DbInfo;
} CEDB_FIND_DATA, *LPCEDB_FIND_DATA, **LPLPCEDB_FIND_DATA;

#define OBJTYPE_INVALID     0
#define OBJTYPE_FILE        1
#define OBJTYPE_DIRECTORY   2
#define OBJTYPE_DATABASE    3
#define OBJTYPE_RECORD      4
/*
 * returned by CeOidGetInfo() for an ActiveSync notification when
 * an object has been deleted.
 */
#define OBJTYPE_DELETED     8

typedef struct _CEOIDINFO {
	WORD wObjType;
	WORD wPad;
	union {
		CEFILEINFO infFile;
		CEDIRINFO infDirectory;
		CEDBASEINFO infDatabase;
		CERECORDINFO infRecord;
	} u;
} CEOIDINFO;

#define CEDB_AUTOINCREMENT              0x00000001

#define CEDB_SEEK_CEOID                 0x00000001
#define CEDB_SEEK_BEGINNING             0x00000002
#define CEDB_SEEK_END                   0x00000004
#define CEDB_SEEK_CURRENT               0x00000008

#define CEDB_SEEK_VALUESMALLER          0x00000010
#define CEDB_SEEK_VALUEFIRSTEQUAL       0x00000020
#define CEDB_SEEK_VALUEGREATER          0x00000040
#define CEDB_SEEK_VALUENEXTEQUAL        0x00000080

typedef struct _CEBLOB {
	DWORD dwCount;
	LPBYTE lpb;
} CEBLOB;

#define CEVT_I2         2
#define CEVT_I4         3
#define CEVT_R8         5
#define CEVT_BOOL       11
#define CEVT_UI2        18
#define CEVT_UI4        19
#define CEVT_LPWSTR     31
#define CEVT_FILETIME   64
#define CEVT_BLOB       65

/* Undocumented flag used by synchronization to denote an empty field */
#define CEVT_FLAG_EMPTY                 0x0400


typedef union _CEVALUNION {
	short iVal;
	USHORT uiVal;
	LONG lVal;
	ULONG ulVal;
	FILETIME filetime;
	LPWSTR lpwstr;
	CEBLOB blob;
	BOOL boolVal;
	double dblVal;
} CEVALUNION;

#define CEDB_PROPNOTFOUND               0x0100
#define CEDB_PROPDELETE                 0x0200

typedef struct _CEPROPVAL {
	CEPROPID propid;
	WORD wLenData;
	WORD wFlags;
	CEVALUNION val;
} CEPROPVAL;
typedef CEPROPVAL *PCEPROPVAL;

#define CEDB_MAXDATABLOCKSIZE           4092
#define CEDB_MAXPROPDATASIZE            (CEDB_MAXDATABLOCKSIZE*16)
#define CEDB_MAXRECORDSIZE              (128*1024)

#define CEDB_ALLOWREALLOC               0x00000001


#define SYSMEM_CHANGED        0
#define SYSMEM_MUSTREBOOT     1
#define SYSMEM_REBOOTPENDING  2
#define SYSMEM_FAILED         3

typedef struct _CEOSVERSIONINFO{
	DWORD dwOSVersionInfoSize;
	DWORD dwMajorVersion;
	DWORD dwMinorVersion;
	DWORD dwBuildNumber;
	DWORD dwPlatformId;
	WCHAR szCSDVersion[128];
} CEOSVERSIONINFO, *LPCEOSVERSIONINFO;

/*
 * device power status
 */
#define AC_LINE_OFFLINE                 0x00
#define AC_LINE_ONLINE                  0x01
#define AC_LINE_BACKUP_POWER            0x02
#define AC_LINE_UNKNOWN                 0xFF

#define BATTERY_FLAG_HIGH               0x01
#define BATTERY_FLAG_LOW                0x02
#define BATTERY_FLAG_CRITICAL           0x04
#define BATTERY_FLAG_CHARGING           0x08
#define BATTERY_FLAG_NO_BATTERY         0x80
#define BATTERY_FLAG_UNKNOWN            0xFF

#define BATTERY_PERCENTAGE_UNKNOWN      0xFF

#define BATTERY_LIFE_UNKNOWN        0xFFFFFFFF

typedef struct _SYSTEM_POWER_STATUS_EX {
	BYTE ACLineStatus;
	BYTE BatteryFlag;
	BYTE BatteryLifePercent;
	BYTE Reserved1;
	DWORD BatteryLifeTime;
	DWORD BatteryFullLifeTime;
	BYTE Reserved2;
	BYTE BackupBatteryFlag;
	BYTE BackupBatteryLifePercent;
	BYTE Reserved3;
	DWORD BackupBatteryLifeTime;
	DWORD BackupBatteryFullLifeTime;
} SYSTEM_POWER_STATUS_EX, *PSYSTEM_POWER_STATUS_EX, *LPSYSTEM_POWER_STATUS_EX;

/*
 * returned by CeRapiInit() when already initialized
 */
#define CERAPI_E_ALREADYINITIALIZED  0x80041001


/*
 * The following are found in various header files, all included
 * through windows.h
 */

/*
 * from winbase.h
 */

/*
 * File access functions
 */
/* dwShareMode */
#define GENERIC_WRITE              0x40000000
#define GENERIC_READ               0x80000000
#define FILE_SHARE_READ            0x00000001

/* dwCreationDisposition */
#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5
#define OPEN_FOR_LOADER     6

/* dwMoveMethod */
#define FILE_BEGIN      0
#define FILE_CURRENT    1
#define FILE_END        2

#define INVALID_FILE_SIZE 0xFFFFFFFF

/* not relevant for ce, set to void* */
typedef void* LPSECURITY_ATTRIBUTES;

/* not relevant for ce, set to void* */
typedef void* LPOVERLAPPED;


typedef struct _PROCESS_INFORMATION {
	HANDLE hProcess;
	HANDLE hThread;
	DWORD dwProcessId;
	DWORD dwThreadId;
} PROCESS_INFORMATION;
typedef PROCESS_INFORMATION* LPPROCESS_INFORMATION;

typedef struct _SYSTEM_INFO {
/*	DWORD dwOemId;*/
	WORD wProcessorArchitecture;
	WORD wReserved;
	DWORD dwPageSize;
	ULONG lpMinimumApplicationAddress;
	ULONG lpMaximumApplicationAddress;
	DWORD dwActiveProcessorMask;
	DWORD dwNumberOfProcessors;
	DWORD dwProcessorType;
	DWORD dwAllocationGranularity;
	WORD wProcessorLevel;
	WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

#define VER_PLATFORM_WIN32s             0
#define VER_PLATFORM_WIN32_WINDOWS      1
#define VER_PLATFORM_WIN32_NT           2
#define VER_PLATFORM_WIN32_HH           3
#define VER_PLATFORM_WIN32_CE           3


/*
 * from shellapi.h
 */

#define CSIDL_PROGRAMS           0x0002
#define CSIDL_PERSONAL           0x0005
#define CSIDL_FAVORITES_GRYPHON  0x0006
#define CSIDL_STARTUP            0x0007
#define CSIDL_RECENT             0x0008
#define CSIDL_STARTMENU          0x000b
#define CSIDL_DESKTOPDIRECTORY   0x0010
#define CSIDL_FONTS              0x0014
#define CSIDL_FAVORITES          0x0016


/*
 * from winreg.h
 */

/*
 * Registry
 */
#define HKEY_CLASSES_ROOT           ((HKEY)0x80000000)
#define HKEY_CURRENT_USER           ((HKEY)0x80000001)
#define HKEY_LOCAL_MACHINE          ((HKEY)0x80000002)
#define HKEY_USERS                  ((HKEY)0x80000003)


/*
 * from winnt.h
 */

/*
 * Registry
 */
#define REG_NONE                    0
#define REG_SZ                      1
#define REG_EXPAND_SZ               2
#define REG_BINARY                  3
#define REG_DWORD                   4
#define REG_DWORD_LITTLE_ENDIAN     4
#define REG_DWORD_BIG_ENDIAN        5
#define REG_LINK                    6
#define REG_MULTI_SZ                7

#define REG_CREATED_NEW_KEY         1
#define REG_OPENED_EXISTING_KEY     2

/*
 * file access
 */
/* dwFlagsAndAttributes */
#define FILE_ATTRIBUTE_READONLY     0x00000001
#define FILE_ATTRIBUTE_HIDDEN       0x00000002
#define FILE_ATTRIBUTE_SYSTEM       0x00000004
#define FILE_ATTRIBUTE_1            0x00000008

#define FILE_ATTRIBUTE_DIRECTORY    0x00000010
#define FILE_ATTRIBUTE_ARCHIVE      0x00000020
#define FILE_ATTRIBUTE_INROM        0x00000040
#define FILE_ATTRIBUTE_NORMAL       0x00000080

#define FILE_ATTRIBUTE_TEMPORARY    0x00000100
#define FILE_ATTRIBUTE_2            0x00000200 /* SPARSE_FILE ? */
#define FILE_ATTRIBUTE_3            0x00000400 /* REPARSE_POINT ? */
#define FILE_ATTRIBUTE_COMPRESSED   0x00000800

#define FILE_ATTRIBUTE_ROMSTATICREF 0x00001000
#define FILE_ATTRIBUTE_ROMMODULE    0x00002000
#define FILE_ATTRIBUTE_4          0x00004000
#define FILE_ATTRIBUTE_5          0x00008000

#define FILE_ATTRIBUTE_HAS_CHILDREN 0x00010000
#define FILE_ATTRIBUTE_SHORTCUT   0x00020000
#define FILE_ATTRIBUTE_6          0x00040000
#define FILE_ATTRIBUTE_7          0x00080000


/*
 * system info
 */
#define PROCESSOR_INTEL_386     386
#define PROCESSOR_INTEL_486     486
#define PROCESSOR_INTEL_PENTIUM 586
#define PROCESSOR_INTEL_PENTIUMII 686
#define PROCESSOR_MIPS_R4000    4000
#define PROCESSOR_ALPHA_21064   21064
#define PROCESSOR_PPC_403       403
#define PROCESSOR_PPC_601       601
#define PROCESSOR_PPC_603       603
#define PROCESSOR_PPC_604       604
#define PROCESSOR_PPC_620       620
#define PROCESSOR_HITACHI_SH3   10003
#define PROCESSOR_HITACHI_SH3E  10004
#define PROCESSOR_HITACHI_SH4   10005
#define PROCESSOR_MOTOROLA_821  821
#define PROCESSOR_SHx_SH3       103
#define PROCESSOR_SHx_SH4       104
#define PROCESSOR_STRONGARM     2577
#define PROCESSOR_ARM720        1824
#define PROCESSOR_ARM820        2080
#define PROCESSOR_ARM920        23360
#define PROCESSOR_ARM_7TDMI     70001

#define PROCESSOR_ARCHITECTURE_INTEL 0
#define PROCESSOR_ARCHITECTURE_MIPS  1
#define PROCESSOR_ARCHITECTURE_ALPHA 2
#define PROCESSOR_ARCHITECTURE_PPC   3
#define PROCESSOR_ARCHITECTURE_SHX   4
#define PROCESSOR_ARCHITECTURE_ARM   5
#define PROCESSOR_ARCHITECTURE_IA64  6
#define PROCESSOR_ARCHITECTURE_ALPHA64 7
#define PROCESSOR_ARCHITECTURE_UNKNOWN 0xFFFF



#ifdef __cplusplus
}
}
#endif

#endif

