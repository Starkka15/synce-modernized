"""Python bindings to librapi2

RAPI is the protocol used to communicate with Windows CE and
Windows Mobile devices, and librapi2 is the open source
implementation from the SynCE project.

For more information, go to www.synce.org"""

include "types.pxi"
include "constants.pxi"
import sys

cdef extern from "Python.h":
    object PyString_FromStringAndSize ( char *, int )
    object PyCObject_FromVoidPtr(void* cobj, void (*destr)(void *))

cdef extern from "stdlib.h":
    void *malloc(size_t size) nogil
    void *realloc(void *ptr, size_t size) nogil
    void free(void *ptr) nogil

cdef extern from "string.h":
    void *memcpy(void *s1, void *s2, size_t n) nogil

cdef extern from "synce.h":
    char *wstr_to_utf8(LPCWSTR unicode) nogil
    LPWSTR wstr_from_utf8(char *utf8) nogil
    void wstr_free_string(void *str) nogil
    char* synce_strerror(DWORD error) nogil

cdef extern from "synce_log.h":
    void synce_log_set_level(int level) nogil

cdef extern from "rapi2.h":
    ctypedef void IRAPISession
    ctypedef void IRAPIDevice
    ctypedef void IRAPIEnumDevices
    ctypedef void IRAPIDesktop
    ctypedef struct GUID:
        DWORD Data1
        WORD Data2
        WORD Data3
        BYTE Data4
    ctypedef GUID RAPIDEVICEID
    ctypedef struct RAPI_DEVICEINFO:
        RAPIDEVICEID DeviceId
        DWORD dwOsVersionMajor
        DWORD dwOsVersionMinor
        char *bstrName
        char *bstrPlatform

    # connection functions
    HRESULT IRAPIDesktop_Get(IRAPIDesktop **ppIRAPIDesktop) nogil
    void IRAPIDesktop_Release(IRAPIDesktop *self) nogil
    HRESULT IRAPIDesktop_EnumDevices(IRAPIDesktop *self, IRAPIEnumDevices** ppIEnum) nogil

    HRESULT IRAPIEnumDevices_Next(IRAPIEnumDevices *self, IRAPIDevice** ppIDevice) nogil
    void IRAPIEnumDevices_Release(IRAPIEnumDevices *self) nogil

    void IRAPIDevice_AddRef(IRAPIDevice *self) nogil
    void IRAPIDevice_Release(IRAPIDevice *self) nogil
    HRESULT IRAPIDevice_CreateSession(IRAPIDevice *self, IRAPISession** ppISession) nogil
    HRESULT IRAPIDevice_GetDeviceInfo(IRAPIDevice *self, RAPI_DEVICEINFO* pDevInfo) nogil

    void IRAPISession_Release(IRAPISession *session) nogil
    HRESULT IRAPISession_CeRapiFreeBuffer(IRAPISession *session, LPVOID Buffer) nogil
    HRESULT IRAPISession_CeRapiInit(IRAPISession *session) nogil
    HRESULT IRAPISession_CeRapiUninit(IRAPISession *session) nogil

    # registry functions
    LONG IRAPISession_CeRegCreateKeyEx(IRAPISession *session, HKEY hKey, LPCWSTR lpszSubKey, DWORD Reserved, LPWSTR lpszClass, DWORD ulOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition) nogil
    LONG IRAPISession_CeRegOpenKeyEx(IRAPISession *session, HKEY hKey, LPCWSTR lpszSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) nogil
    LONG IRAPISession_CeRegCloseKey(IRAPISession *session, HKEY hKey) nogil
    LONG IRAPISession_CeRegDeleteKey(IRAPISession *session, HKEY hKey, LPCWSTR lpszSubKey) nogil
    LONG IRAPISession_CeRegDeleteValue(IRAPISession *session, HKEY hKey, LPCWSTR lpszValueName) nogil
    LONG IRAPISession_CeRegQueryValueEx(IRAPISession *session, HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) nogil
    LONG IRAPISession_CeRegSetValueEx(IRAPISession *session, HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, DWORD dwType, BYTE *lpData, DWORD cbData) nogil
    LONG IRAPISession_CeRegQueryInfoKey (IRAPISession *session, HKEY hKey, LPWSTR lpClass, LPDWORD lpcbClass, LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcbMaxSubKeyLen, LPDWORD lpcbMaxClassLen, LPDWORD lpcValues, LPDWORD lpcbMaxValueNameLen, LPDWORD lpcbMaxValueLen, LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime ) nogil

    LONG IRAPISession_CeRegEnumKeyEx(IRAPISession *session, HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcbName, LPDWORD lpReserved, LPWSTR lpClass, LPDWORD lpcbClass, PFILETIME lpftLastWriteTime) nogil
    LONG IRAPISession_CeRegEnumValue(IRAPISession *session, HKEY hKey, DWORD dwIndex, LPWSTR lpszValueName, LPDWORD lpcbValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) nogil



    # sync functions
    BOOL IRAPISession_CeStartReplication(IRAPISession *session) nogil
    HRESULT IRAPISession_CeSyncStart(IRAPISession *session, LPCWSTR params) nogil
    HRESULT IRAPISession_CeSyncResume(IRAPISession *session) nogil
    HRESULT IRAPISession_CeSyncPause(IRAPISession *session) nogil
    HRESULT IRAPISession_CeSyncTimeToPc(IRAPISession *session) nogil

    # miscellaneous functions
    HRESULT IRAPISession_CeProcessConfig(IRAPISession *session, LPCWSTR config, DWORD flags, LPWSTR* reply) nogil
    BOOL IRAPISession_CeGetSystemPowerStatusEx(IRAPISession *session, PSYSTEM_POWER_STATUS_EX pSystemPowerStatus, BOOL refresh) nogil
    DWORD IRAPISession_CeGetDiskFreeSpaceEx(IRAPISession *session, LPCTSTR _lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailable, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes) nogil
    BOOL IRAPISession_CeFindAllFiles(IRAPISession *session, LPCWSTR szPath, DWORD dwFlags, LPDWORD lpdwFoundCount, LPLPCE_FIND_DATA ppFindDataArray) nogil
    BOOL IRAPISession_CeCreateProcess(IRAPISession *session, LPCWSTR lpApplicationName, LPCWSTR lpCommandLine, void* lpProcessAttributes, void* lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPWSTR lpCurrentDirectory, void* lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) nogil

    VOID IRAPISession_CeGetSystemInfo(IRAPISession *session, LPSYSTEM_INFO lpSystemInfo) nogil
    BOOL IRAPISession_CeGetVersionEx(IRAPISession *session, LPCEOSVERSIONINFO lpVersionInformation) nogil

    # file access
    BOOL IRAPISession_CeCloseHandle(IRAPISession *session, HANDLE hObject) nogil
    HANDLE IRAPISession_CeCreateFile(IRAPISession *session, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) nogil
    BOOL IRAPISession_CeWriteFile(IRAPISession *session, HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) nogil
    BOOL IRAPISession_CeReadFile(IRAPISession *session, HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) nogil
    DWORD IRAPISession_CeSetFilePointer(IRAPISession *session, HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) nogil

    # error handling
    HRESULT IRAPISession_CeRapiGetError(IRAPISession *session) nogil
    DWORD IRAPISession_CeGetLastError(IRAPISession *session) nogil

    # connection parameters

    char * IRAPIDevice_get_name(IRAPIDevice *self) nogil
    int IRAPIDevice_get_os_version(IRAPIDevice *self, unsigned int *os_major, unsigned int *os_minor) nogil
    unsigned int IRAPIDevice_get_build_number(IRAPIDevice *self) nogil
    unsigned int IRAPIDevice_get_processor_type(IRAPIDevice *self) nogil
    char * IRAPIDevice_get_os_name(IRAPIDevice *self) nogil
    char * IRAPIDevice_get_model(IRAPIDevice *self) nogil
    char * IRAPIDevice_get_device_ip(IRAPIDevice *self) nogil
    char * IRAPIDevice_get_local_ip(IRAPIDevice *self) nogil

#
# Public constants
#

# from synce_log.h
SYNCE_LOG_LEVEL_LOWEST  = 0

SYNCE_LOG_LEVEL_ERROR   = 1
SYNCE_LOG_LEVEL_WARNING = 2
SYNCE_LOG_LEVEL_INFO    = 3
SYNCE_LOG_LEVEL_DEBUG   = 4
SYNCE_LOG_LEVEL_TRACE   = SYNCE_LOG_LEVEL_DEBUG

SYNCE_LOG_LEVEL_HIGHEST = 5
SYNCE_LOG_LEVEL_DEFAULT = 2

# from synce_sys_error.h
ERROR_SUCCESS           = 0
ERROR_NO_MORE_ITEMS     = 259

# from synce_types.h
FALSE                   = 0
TRUE                    = 1

# some HRESULTs
E_ABORT                 = 0x80004004
E_ACCESSDENIED          = 0x80070005
E_FAIL                  = 0x80004005
E_HANDLE                = 0x80070006
E_OUTOFMEMORY           = 0x8007000E
E_INVALIDARG            = 0x80070057
E_NOINTERFACE           = 0x80004002
E_NOTIMPL               = 0x80004001
E_OUTOFMEMORY           = 0x8007000E
E_PENDING               = 0x8000000A
E_POINTER               = 0x80004003
E_UNEXPECTED            = 0x8000FFFF
S_FALSE                 = 0x00000001
S_OK                    = 0x00000000


# from rapitypes.h
# registry value types
REG_NONE                = 0
REG_SZ                  = 1
REG_EXPAND_SZ           = 2
REG_BINARY              = 3
REG_DWORD               = 4
REG_DWORD_LITTLE_ENDIAN = 4
REG_DWORD_BIG_ENDIAN    = 5
REG_LINK                = 6
REG_MULTI_SZ            = 7

# registry key dispositions
REG_CREATED_NEW_KEY     = 1
REG_OPENED_EXISTING_KEY = 2

# battery status
BATTERY_FLAG_HIGH          =    0x01
BATTERY_FLAG_LOW           =    0x02
BATTERY_FLAG_CRITICAL      =    0x04
BATTERY_FLAG_CHARGING      =    0x08
BATTERY_FLAG_NO_BATTERY    =    0x80
BATTERY_FLAG_UNKNOWN       =    0xFF

# flags for finding file info
FAF_ATTRIBUTES               = 0x00001
FAF_CREATION_TIME            = 0x00002
FAF_LASTACCESS_TIME          = 0x00004
FAF_LASTWRITE_TIME           = 0x00008

FAF_SIZE_HIGH                = 0x00010
FAF_SIZE_LOW                 = 0x00020
FAF_OID                      = 0x00040
FAF_NAME                     = 0x00080

FAF_ATTRIB_CHILDREN          = 0x01000
FAF_ATTRIB_NO_HIDDEN         = 0x02000
FAF_FOLDERS_ONLY             = 0x04000
FAF_NO_HIDDEN_SYS_ROMMODULES = 0x08000

CSIDL_PROGRAMS               = 0x0002
CSIDL_PERSONAL               = 0x0005
CSIDL_FAVORITES_GRYPHON      = 0x0006
CSIDL_STARTUP                = 0x0007
CSIDL_RECENT                 = 0x0008
CSIDL_STARTMENU              = 0x000b
CSIDL_DESKTOPDIRECTORY       = 0x0010
CSIDL_FONTS                  = 0x0014
CSIDL_FAVORITES              = 0x0016

#dwShareMode
FILE_SHARE_READ              = 0x00000001

# dwMoveMethod
FILE_BEGIN                   = 0
FILE_CURRENT                 = 1
FILE_END                     = 2

#dwFlagsAndAttributes
FILE_ATTRIBUTE_READONLY      = 0x00000001
FILE_ATTRIBUTE_HIDDEN        = 0x00000002
FILE_ATTRIBUTE_SYSTEM        = 0x00000004
FILE_ATTRIBUTE_1             = 0x00000008

FILE_ATTRIBUTE_DIRECTORY     = 0x00000010
FILE_ATTRIBUTE_ARCHIVE       = 0x00000020
FILE_ATTRIBUTE_INROM         = 0x00000040
FILE_ATTRIBUTE_NORMAL        = 0x00000080

FILE_ATTRIBUTE_TEMPORARY     = 0x00000100
FILE_ATTRIBUTE_2             = 0x00000200
FILE_ATTRIBUTE_3             = 0x00000400
FILE_ATTRIBUTE_COMPRESSED    = 0x00000800

FILE_ATTRIBUTE_ROMSTATICREF  = 0x00001000
FILE_ATTRIBUTE_ROMMODULE     = 0x00002000
FILE_ATTRIBUTE_4             = 0x00004000
FILE_ATTRIBUTE_5             = 0x00008000

FILE_ATTRIBUTE_HAS_CHILDREN  = 0x00010000
FILE_ATTRIBUTE_SHORTCUT      = 0x00020000
FILE_ATTRIBUTE_6             = 0x00040000
FILE_ATTRIBUTE_7             = 0x00080000

# declaration of RAPISession for other classes
cdef class RAPISession

#
# RAPIError is a subclass of Exception

class RAPIError(Exception):
    "An error resulting from a RAPI call"
    def __init__(self, err_code=None, hresult=None, RAPISession session=None, message=None):
        cdef HRESULT hr
        cdef DWORD last_error

        if err_code != None:
            self.err_code = err_code
        elif hresult != None:
            self.err_code = ((hresult) & 0xFFFF)
        elif session != None:
            hr = IRAPISession_CeRapiGetError(session.rapi_conn)
            if hr < 0:
                self.err_code = ((hr) & 0xFFFF)
            else:
                last_error = IRAPISession_CeGetLastError(session.rapi_conn)
                self.err_code = last_error
        else:
                self.err_code = 0
        self.message = message

    def __str__(self):
        if self.message == None:
            return str(self.err_code)+": "+str(synce_strerror(self.err_code))
        else:
            return self.message+": "+str(self.err_code)+": "+str(synce_strerror(self.err_code))


class RegKey(object):
    """Registry key entry from a Windows Mobile Device"""
    def __init__(self, RAPISession rapi_session not None, handle, disposition=REG_OPENED_EXISTING_KEY):
        self.rapi_session = rapi_session
        self.handle = handle
        self.disposition = disposition
        self._host_le = (sys.byteorder == "little")

    def __del__(self):
        self.close()

    def keys(self):
        """Returns an array containing all the subkeys"""
        result = []

        cdef DWORD index
        cdef LPWSTR name
        cdef DWORD name_len
        cdef RAPISession session

        session = self.rapi_session

        try:
            name = <LPWSTR> malloc(255)

            finished = False

            index = 0

            while not finished:
                name_len = 254

                retval = IRAPISession_CeRegEnumKeyEx(session.rapi_conn, self.handle, index, name, &name_len, NULL, NULL, NULL, NULL)

                if retval == 259L:
                    finished = True


                if not finished:
                    if retval != ERROR_SUCCESS:
                        raise RAPIError(retval)

                    result.append( wstr_to_utf8(name) )

                    index = index + 1

            return result

        finally:
            if name != NULL:
                free(name)

    def keys_and_childcount(self, query_child_count=False ):
        """Returns an array containing all the subkeys"""
        result = []

        cdef DWORD index
        cdef LPWSTR name
        cdef DWORD name_len
        cdef DWORD key_count
        cdef RAPISession session

        session = self.rapi_session

        try:
            name = <LPWSTR> malloc(255)

            finished = False

            index = 0

            #-1 means that no child count information is requested.
            #0 cannot be used, because this would collide with nodes having 0 childs
            key_count = -1

            while not finished:
                name_len = 254

                retval = IRAPISession_CeRegEnumKeyEx(session.rapi_conn, self.handle, index, name, &name_len, NULL, NULL, NULL, NULL)

                if retval == 259L:
                    finished = True


                if not finished:
                    if retval != ERROR_SUCCESS:
                            raise RAPIError(retval)

                    if query_child_count:
                        sub_key = self.open_sub_key( wstr_to_utf8(name) )
                        (key_count ,_) = sub_key.number_of_keys_and_values()
                        sub_key.close()

                    result.append( (wstr_to_utf8(name) , key_count ) )

                    index = index + 1

            return result

        finally:
            if name != NULL:
                free(name)




    def values(self):
        """Returns a list of tuples (value_name, value_type, value_data) of
        values stored in the current key"""


        #First determine the maximum size for the buffer that we need
        cdef DWORD data_max_size
        cdef RAPISession session

        session = self.rapi_session
        retval = IRAPISession_CeRegQueryInfoKey(session.rapi_conn, self.handle, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &data_max_size , NULL, NULL)

        result = []

        cdef DWORD index
        cdef LPWSTR name
        cdef DWORD name_len

        cdef DWORD type
        cdef DWORD data_size
        cdef LPBYTE data
        cdef LPDWORD dw_ptr

        try:
            name = <LPWSTR> malloc(255)

            data = <LPBYTE> malloc(data_max_size)


            finished = False

            index = 0

            while not finished:
                name_len = 254
                data_size = data_max_size

                retval = IRAPISession_CeRegEnumValue(session.rapi_conn, self.handle, index, name, &name_len, NULL, &type, data, &data_size)

                #If return value is 259L, then we are finished
                if retval == 259L:
                    finished = True


                if not finished:
                    if retval != ERROR_SUCCESS:
                            raise RAPIError(retval)


                    if type == REG_NONE:
                        value = PyString_FromStringAndSize(<char *>data, data_size)
                    elif type == REG_SZ or type == REG_EXPAND_SZ:
                        value = wstr_to_utf8(<LPCWSTR> data)
                    elif type == REG_BINARY:
                        value = PyString_FromStringAndSize(<char *>data, data_size)
                    elif type == REG_DWORD:
                        dw_ptr = <LPDWORD> data
                        value = self._dword_le_to_host(dw_ptr[0])
                    elif type == REG_DWORD_BIG_ENDIAN:
                        dw_ptr = <LPDWORD> data
                        value = self._dword_be_to_host(dw_ptr[0])
                    elif type == REG_LINK:
                        value = PyString_FromStringAndSize(<char *>data, data_size)
                    elif type == REG_MULTI_SZ:
                        # FIXME - this is doable
                        value = PyString_FromStringAndSize(<char *>data, data_size)
                    else:
                        value = PyString_FromStringAndSize(<char *>data, data_size)

                    result.append( (wstr_to_utf8(name) , type,value)    )

                    index = index + 1

            return result

        finally:
            if name != NULL:
                free(name)
                free(data)

    def rename_value(self, old_value_name, new_value_name):
        """Rename a value contained in this key"""
        self.copy_value(old_value_name, self, new_value_name)
        self.delete_value( old_value_name )


    def copy_key_recursively(self, new_key ):
        """Copy the contents of the key denoted by the RegKey object
        old_key recursively to the key denoted by the new_key RegKey object"""

        self.copy_all_values( new_key )

        for (sub_key_name) in self.keys():
            old_sub_key = self.open_sub_key( sub_key_name )
            new_sub_key = new_key.create_sub_key(sub_key_name)

            old_sub_key.copy_key_recursively( new_sub_key )

            old_sub_key.close()
            new_sub_key.close()


    def rename_key(self, old_key_name, new_key_name):
        """Rename a sub key of the current key

        This is done by creating a new key and recursively copy the
        contents of the original key to this new key"""

        old_key = self.open_sub_key( old_key_name )

        new_key = self.create_sub_key( new_key_name )

        old_key.copy_key_recursively( new_key )

        old_key.close()
        new_key.close()

        self.delete_sub_key( old_key_name )


    def copy_value(self, value_name, destination_key, destination_value_name=None):
        """Copy the value value_name to the registry key denoted by the
        RegKey object destination_key.

        If the destination_value_name is not given, the same name is used"""

        cdef LPWSTR value_name_w
        cdef LPWSTR destination_value_name_w
        cdef LPBYTE data
        cdef DWORD data_size
        cdef DWORD data_type
        cdef RAPISession session

        session = self.rapi_session

        value_name_w = NULL
        destination_value_name_w = NULL
        data = NULL

        try:
            if destination_value_name is None:
                destination_value_name = value_name


            value_name_w = wstr_from_utf8(value_name)

            data_size = 0

            #First determine the size of the buffer we need to allocate
            retval = IRAPISession_CeRegQueryValueEx(session.rapi_conn, self.handle, value_name_w , NULL, NULL, NULL, &data_size)

            if retval != ERROR_SUCCESS:
                raise RAPIError(retval)


            #Now create buffer for the data and query the data

            data = <LPBYTE> malloc(data_size)

            retval = IRAPISession_CeRegQueryValueEx(session.rapi_conn, self.handle, value_name_w , NULL,
                                       &data_type, data, &data_size)

            if retval != ERROR_SUCCESS:
                raise RAPIError(retval)


            destination_value_name_w = wstr_from_utf8(destination_value_name)



            #Create a value in the destination_key
            retval = IRAPISession_CeRegSetValueEx(session.rapi_conn, destination_key.handle , destination_value_name_w , 0 , data_type, data, data_size)

            if retval != ERROR_SUCCESS:
                raise RAPIError(retval)

            return

        finally:
            if value_name_w != NULL:
                wstr_free_string(value_name_w)
            if destination_value_name_w != NULL:
                wstr_free_string(destination_value_name_w)
            if data != NULL:
                free(data)
        pass

    def copy_all_values(self, destination_key):
        """Copy all values that are within the current key to the key denoted
        by the RegKey object destination_key"""
        for (value_name,_,_) in self.values():
            self.copy_value( value_name, destination_key )


    def number_of_keys_and_values(self):
        """Returns the number of child keys and number of values stored in
        this key."""

        cdef DWORD number_of_keys
        cdef DWORD number_of_values
        cdef RAPISession session

        session = self.rapi_session
        retval = IRAPISession_CeRegQueryInfoKey(session.rapi_conn, self.handle, NULL, NULL, NULL, &number_of_keys, NULL, NULL, &number_of_values, NULL, NULL, NULL, NULL)

        if retval != ERROR_SUCCESS:
            raise RAPIError(retval)

        return (number_of_keys, number_of_values)


    def open_sub_key(self, sub_key):
        """Open an existing sub key of this key.

        Takes as an argument the name of the subkey relative to this key. The path
        to a key more than one level down should be separated by backslashes. Returns
        a new RegKey object on success."""

        cdef LPWSTR sub_key_w
        cdef HKEY opened_key
        cdef RAPISession session

        session = self.rapi_session

        if sub_key != None:
            sub_key_w = wstr_from_utf8(sub_key)
        else:
            sub_key_w = NULL

        retval = IRAPISession_CeRegOpenKeyEx(session.rapi_conn, self.handle, sub_key_w, 0, 0, &opened_key)

        if sub_key_w != NULL:
            wstr_free_string(sub_key_w)

        if retval != ERROR_SUCCESS:
            raise RAPIError(retval)

        return RegKey(self.rapi_session, opened_key)

    def create_sub_key(self, sub_key, key_class=""):
        """Creates a sub key of this key, or opens an existing key.

        Takes as an argument the name of the subkey to create relative to this key.
        The path to a key more than one level down should be separated by backslashes.
        The optional key_class argument can be used to specify the class of the new key.
        Returns a new RegKey object on success."""

        cdef HKEY new_key
        cdef DWORD disposition
        cdef LPWSTR sub_key_w
        cdef LPWSTR key_class_w
        cdef RAPISession session

        session = self.rapi_session

        if sub_key != None:
            sub_key_w = wstr_from_utf8(sub_key)
        else:
            sub_key_w = NULL

        key_class_w = wstr_from_utf8(key_class)

        retval = IRAPISession_CeRegCreateKeyEx(session.rapi_conn, self.handle, sub_key_w, 0, key_class_w,
                                  0, 0, NULL, &new_key, &disposition)

        if sub_key_w != NULL:
            wstr_free_string(sub_key_w)

        wstr_free_string(key_class_w)

        if retval != ERROR_SUCCESS:
            raise RAPIError(retval)

        return RegKey(self.rapi_session, new_key, disposition)

    def _dword_swap(self, dw):
        return (dw & 0x000000FF) << 24 | \
               (dw & 0x0000FF00) << 8  | \
               (dw & 0x00FF0000) >> 8  | \
               (dw & 0xFF000000) >> 24

    def _dword_le_to_host(self, dw):
        if self._host_le:
            return dw
        else:
            return self._dword_swap(dw)

    def _dword_le_from_host(self, dw):
        if self._host_le:
            return dw
        else:
            return self._dword_swap(dw)

    def _dword_be_to_host(self, dw):
        if self._host_le:
            return self._dword_swap(dw)
        else:
            return dw

    def _dword_be_from_host(self, dw):
        if self._host_le:
            return self._dword_swap(dw)
        else:
            return dw




    def get_value(self, value_name):
        """Obtain a tuple (value_type, value) from a value contained in this
        key

        Takes as an argument the name of the value required, and returns
        a tuple of the type of the value and the value itself, converted
        to a python type
        """
        cdef LPWSTR name_w
        cdef DWORD type
        cdef LPBYTE data
        cdef DWORD data_size
        cdef LPDWORD dw_ptr
        cdef RAPISession session

        session = self.rapi_session

        name_w = NULL
        data = NULL

        try:
            if value_name != None:
                name_w = wstr_from_utf8(value_name)
            else:
                name_w = NULL

            data_size = 0
            retval = IRAPISession_CeRegQueryValueEx(session.rapi_conn, self.handle, name_w, NULL,
                                       &type, NULL, &data_size)

            if retval != ERROR_SUCCESS:
                raise RAPIError(retval)

            data = <LPBYTE> malloc(data_size)

            retval = IRAPISession_CeRegQueryValueEx(session.rapi_conn, self.handle, name_w, NULL,
                                       &type, data, &data_size)


            REG_MULTI_SZ
            if retval != ERROR_SUCCESS:
                raise RAPIError(retval)

            if type == REG_NONE:
                value = PyString_FromStringAndSize(<char *>data, data_size)
            elif type == REG_SZ or type == REG_EXPAND_SZ:
                value = wstr_to_utf8(<LPCWSTR> data)
            elif type == REG_BINARY:
                value = PyString_FromStringAndSize(<char *>data, data_size)
            elif type == REG_DWORD:
                dw_ptr = <LPDWORD> data
                value = self._dword_le_to_host(dw_ptr[0])
            elif type == REG_DWORD_BIG_ENDIAN:
                dw_ptr = <LPDWORD> data
                value = self._dword_be_to_host(dw_ptr[0])
            elif type == REG_LINK:
                value = PyString_FromStringAndSize(<char *>data, data_size)
            elif type == REG_MULTI_SZ:
                # FIXME - this is doable
                value = PyString_FromStringAndSize(<char *>data, data_size)
            else:
                value = PyString_FromStringAndSize(<char *>data, data_size)

            return (type,value)
        finally:
            if name_w != NULL:
                wstr_free_string(name_w)

            if data != NULL:
                free(data)




    def query_value(self, value_name):
        """Obtain a value contained in this key

        Takes as an argument the name of the value required, and returns
        the value. Currently the registry types DWORD, DWORD_BIG_ENDIAN,
        SZ, EXPAND_SZ are fully supported."""

        cdef LPWSTR name_w
        cdef DWORD type
        cdef LPBYTE data
        cdef DWORD data_size
        cdef LPDWORD dw_ptr
        cdef RAPISession session

        session = self.rapi_session

        name_w = NULL
        data = NULL

        try:
            if value_name != None:
                name_w = wstr_from_utf8(value_name)
            else:
                name_w = NULL

            data_size = 0
            retval = IRAPISession_CeRegQueryValueEx(session.rapi_conn, self.handle, name_w, NULL,
                                       &type, NULL, &data_size)

            if retval != ERROR_SUCCESS:
                raise RAPIError(retval)

            data = <LPBYTE> malloc(data_size)

            retval = IRAPISession_CeRegQueryValueEx(session.rapi_conn, self.handle, name_w, NULL,
                                       &type, data, &data_size)

            if retval != ERROR_SUCCESS:
                raise RAPIError(retval)

            if type == REG_NONE:
                value = PyString_FromStringAndSize(<char *>data, data_size)
            elif type == REG_SZ or type == REG_EXPAND_SZ:
                value = wstr_to_utf8(<LPCWSTR> data)
            elif type == REG_BINARY:
                value = PyString_FromStringAndSize(<char *>data, data_size)
            elif type == REG_DWORD:
                dw_ptr = <LPDWORD> data
                value = self._dword_le_to_host(dw_ptr[0])
            elif type == REG_DWORD_BIG_ENDIAN:
                dw_ptr = <LPDWORD> data
                value = self._dword_be_to_host(dw_ptr[0])
            elif type == REG_LINK:
                value = PyString_FromStringAndSize(<char *>data, data_size)
            elif type == REG_MULTI_SZ:
                # FIXME - this is doable
                value = PyString_FromStringAndSize(<char *>data, data_size)
            else:
                value = PyString_FromStringAndSize(<char *>data, data_size)

            return value
        finally:
            if name_w != NULL:
                wstr_free_string(name_w)

            if data != NULL:
                free(data)





    def set_value(self, value_name, value_data, value_type=None):
        """Set a value contained in this key

        Takes as arguments the name of the value, the data itself, and
        optionally the value type. Currently only the registry types
        DWORD and SZ (string) are fully supported."""

        cdef LPWSTR name_w
        cdef LPBYTE data
        cdef DWORD data_size
        cdef RAPISession session

        session = self.rapi_session

        name_w = NULL
        data = NULL

        try:
            if value_type == None:
                if isinstance(value_data, basestring):
                    value_type = REG_SZ
                else:
                    value_type = REG_DWORD

            if value_name != None:
                name_w = wstr_from_utf8(value_name)
            else:
                name_w = NULL

            if value_type == REG_SZ:
                data = <LPBYTE> wstr_from_utf8(value_data)
                data_size = (len(value_data) + 1) * 2
            elif value_type == REG_DWORD:
                data = <LPBYTE> malloc(4)
                (<LPDWORD> data)[0] = self._dword_le_from_host(value_data)
                data_size = 4
            else:
                raise RAPIError(E_NOTIMPL)

            retval = IRAPISession_CeRegSetValueEx(session.rapi_conn, self.handle, name_w, 0, value_type, data, data_size)

            if retval != ERROR_SUCCESS:
                raise RAPIError(retval)

            return
        finally:
            if name_w != NULL:
                wstr_free_string(name_w)
            if data != NULL:
                if type == REG_SZ:
                    wstr_free_string(data)
                else:
                    free(data)



    def delete_sub_key(self, sub_key):
        """Delete a sub key of this key.

        Takes as an argument the name of the subkey to delete relative to this key.
        The path to a key more than one level down should be separated by backslashes.
        There is no return value."""

        cdef LPWSTR key_w
        cdef RAPISession session

        session = self.rapi_session

        key_w = wstr_from_utf8(sub_key)
        retval = IRAPISession_CeRegDeleteKey(session.rapi_conn, self.handle, key_w)
        wstr_free_string(key_w)

        if retval != ERROR_SUCCESS:
            raise RAPIError(retval)

        return

    def delete_value(self, value_name=None):
        """Delete a value contained in this key

        Takes as an argument the name of the value to delete. There is
        no return value."""

        cdef LPWSTR name_w
        cdef RAPISession session

        session = self.rapi_session

        if value_name != None:
            name_w = wstr_from_utf8(value_name)
        else:
            name_w = NULL

        retval = IRAPISession_CeRegDeleteValue(session.rapi_conn, self.handle, name_w)

        if name_w != NULL:
            wstr_free_string(name_w)

        if retval != ERROR_SUCCESS:
            raise RAPIError(retval)

        return

    def close(self):
        """Close the handle to this key."""

        # we probably dont want to close root keys
        if self.handle == 0x80000000 or self.handle == 0x80000001 or self.handle == 0x80000002 or self.handle == 0x80000003 or self.handle == 0:
            return

        cdef RAPISession session

        session = self.rapi_session

        retval = IRAPISession_CeRegCloseKey(session.rapi_conn, self.handle)
        self.handle = 0
        if retval != ERROR_SUCCESS:
            raise RAPIError(retval)


class RAPIFile(object):
    """File object from a Windows Mobile Device

    This attempts to be as close as possible to a standard Python file-like object"""

    def __init__(self, RAPISession rapi_session not None, handle, filename, mode):
        self.rapi_session = rapi_session
        self.handle = handle
        self.name = filename
        self.mode = mode

    def __del__(self):
        self.close()

    def read(self, size=-1):
        """Read from the file.

        Reads at most size bytes from the file, or the complete file if size is not given"""

        cdef DWORD bytes_read
        cdef DWORD bytes_to_read
        cdef LPBYTE readbuf
        cdef LPBYTE buffer
        cdef BOOL retval
        cdef DWORD total_read
        cdef RAPISession session

        session = self.rapi_session

        total_read = 0

        if size >= 0:
            bytes_to_read = size
        else:
            bytes_to_read = 1024*1024

        readbuf = <LPBYTE> malloc (bytes_to_read)
        buffer = NULL

        while TRUE:
            retval = IRAPISession_CeReadFile(session.rapi_conn, self.handle, readbuf, bytes_to_read, &bytes_read, NULL)

            if retval == FALSE:
                free(readbuf)
                free(buffer)
                raise RAPIError(session=self.rapi_session)

            if bytes_read == 0:
                break

            buffer = <LPBYTE> realloc(buffer, (total_read + bytes_read))
            memcpy(buffer+(sizeof(BYTE)*total_read), readbuf, bytes_read)
            total_read = total_read + bytes_read

            if bytes_read < bytes_to_read:
                bytes_to_read = bytes_to_read - bytes_read
                continue

            if size >= 0:
                break

            bytes_to_read = 1024*1024

        free(readbuf)

        cdef object returnstring
        returnstring = PyString_FromStringAndSize(<char *>buffer, total_read)
        free(buffer)
        return returnstring


    def write(self, buffer):
        """Write to the file.

        Writes the given buffer to the file."""

        cdef HANDLE hFile
        cdef char * lpBuffer
        cdef DWORD bytes_to_write
        cdef DWORD bytes_written
        cdef BOOL retval
        cdef RAPISession session

        session = self.rapi_session

        hFile = self.handle
        lpBuffer = buffer
        bytes_to_write = len(buffer)

        with nogil:
            retval = IRAPISession_CeWriteFile(session.rapi_conn, hFile, lpBuffer, bytes_to_write, &bytes_written, NULL)

        if retval == FALSE:
            raise RAPIError(session=self.rapi_session)

        return bytes_written

    def tell(self):
        """Return the position of the file pointer."""

        cdef DWORD retval
        cdef RAPISession session

        session = self.rapi_session

        retval = IRAPISession_CeSetFilePointer(session.rapi_conn, self.handle, 0, NULL, FILE_CURRENT);
        if retval == 0xFFFFFFFF:
            raise RAPIError(session=self.rapi_session)

        return retval

    def seek(self, offset, whence=0):
        """ Set the position of the file pointer."""

        cdef DWORD retval
        cdef RAPISession session

        session = self.rapi_session

        retval = IRAPISession_CeSetFilePointer(session.rapi_conn, self.handle, offset, NULL, whence);
        if retval == 0xFFFFFFFF:
            raise RAPIError(session=self.rapi_session)

    def close(self):
        """Close the file handle."""

        cdef RAPISession session

        session = self.rapi_session

        if self.handle == 0:
            return

        retval = IRAPISession_CeCloseHandle(session.rapi_conn, self.handle)
        self.handle = 0
        if retval == FALSE:
            raise RAPIError(session=self.rapi_session)


cdef class RAPISession:
    """A connection to a Windows Mobile device."""

    cdef IRAPIDevice *rapi_dev
    cdef IRAPISession *rapi_conn

    def __cinit__(self, device=None, log_level=SYNCE_LOG_LEVEL_LOWEST, *args, **keywords):
        cdef IRAPIDesktop *desktop
        cdef IRAPIEnumDevices *enumdev
        cdef IRAPIDevice *dev
        cdef IRAPISession *session
        cdef RAPI_DEVICEINFO devinfo
        cdef HRESULT hr

        synce_log_set_level(log_level)

        hr = IRAPIDesktop_Get(&desktop)
        if hr < 0:
            raise RAPIError(hresult=hr, message="failed to initialise RAPI")

        hr = IRAPIDesktop_EnumDevices(desktop, &enumdev)
        if hr < 0:
            IRAPIDesktop_Release(desktop)
            raise RAPIError(hresult=hr, message="failed to get connected devices")

        hr = IRAPIEnumDevices_Next(enumdev, &dev)
        while hr >= 0:
            if device == None:
                break

            hr = IRAPIDevice_GetDeviceInfo(dev, &devinfo)
            if hr < 0:
                IRAPIEnumDevices_Release(enumdev)
                IRAPIDesktop_Release(desktop)
                raise RAPIError(hresult=hr, message="failure to get device info")

            if device == devinfo.bstrName:
                break;

            hr = IRAPIEnumDevices_Next(enumdev, &dev)

        if hr < 0:
            IRAPIEnumDevices_Release(enumdev)
            IRAPIDesktop_Release(desktop)
            if device == None:
                raise RAPIError(hresult=hr, message="Could not find default device")
            else:
                raise RAPIError(hresult=hr, message="Could not find device "+device)

        IRAPIDevice_AddRef(dev)
        IRAPIEnumDevices_Release(enumdev)
        enumdev = NULL

        hr = IRAPIDevice_CreateSession(dev, &session)
        if hr < 0:
            IRAPIDevice_Release(dev)
            IRAPIDesktop_Release(desktop)
            raise RAPIError(hresult=hr, message="Could not create a session to device")

        hr = IRAPISession_CeRapiInit(session)
        if hr < 0:
            IRAPISession_Release(session)
            IRAPIDevice_Release(dev)
            IRAPIDesktop_Release(desktop)
            raise RAPIError(hresult=hr, message="Unable to initialize connection to device")

        self.rapi_dev = dev
        self.rapi_conn = session


    def __init__(self, device=None, log_level=SYNCE_LOG_LEVEL_LOWEST):
        return


    def __dealloc__(self):
        if self.rapi_conn != NULL:
            IRAPISession_CeRapiUninit(self.rapi_conn)
            IRAPISession_Release(self.rapi_conn)
            self.rapi_conn = NULL
        if self.rapi_dev != NULL:
            IRAPIDevice_Release(self.rapi_dev)


    def __getattr__(self, name):
        cdef unsigned int os_major
        cdef unsigned int os_minor
        cdef int retval

        if name == "HKEY_CLASSES_ROOT" or name == "HKCR":
            return RegKey(self, 0x80000000)
        elif name == "HKEY_CURRENT_USER" or name == "HKCU":
            return RegKey(self, 0x80000001)
        elif name == "HKEY_LOCAL_MACHINE" or name == "HKLM":
            return RegKey(self, 0x80000002)
        elif name == "HKEY_USERS" or name == "HKU":
            return RegKey(self, 0x80000003)
        elif name == "name":
            return IRAPIDevice_get_name(self.rapi_dev)
        elif name == "os_version":
            retval = IRAPIDevice_get_os_version(self.rapi_dev, &os_major, &os_minor)
            if retval == 0:
                return None
            else:
                return (os_major, os_minor)
        elif name == "build_number":
            return IRAPIDevice_get_build_number(self.rapi_dev)
        elif name == "processor_type":
            return IRAPIDevice_get_processor_type(self.rapi_dev)
        elif name == "os_name":
            return IRAPIDevice_get_os_name(self.rapi_dev)
        elif name == "model":
            return IRAPIDevice_get_model(self.rapi_dev)
        elif name == "device_ip":
            return IRAPIDevice_get_device_ip(self.rapi_dev)
        elif name == "local_ip":
            return IRAPIDevice_get_local_ip(self.rapi_dev)

        # this gives access to the IRapisession C object wrapped in a
        # python CObject, be careful !!!
        elif name == "rapi_connection":
            return PyCObject_FromVoidPtr(<void *>self.rapi_conn, NULL)
        else:
            raise AttributeError("%s instance has no attribute '%s'" % (self.__class__.__name__, name))


    def process_config(self, config, flags):
        """Process the supplied configuration xml.

        Returns the reply document if there is one, otherwise
        an empty string."""

        cdef LPWSTR config_w
        cdef LPWSTR reply_w
        cdef DWORD flags_c
        cdef HRESULT hr
        cdef char *reply

        reply_w = NULL;

        config_w = wstr_from_utf8(config)
        flags_c = flags

        with nogil:
            hr = IRAPISession_CeProcessConfig(self.rapi_conn, config_w, flags_c, &reply_w)
            wstr_free_string(config_w)

            reply = wstr_to_utf8(reply_w)
            wstr_free_string(reply_w)

        if hr < 0:
            raise RAPIError(hresult=hr)

        if reply == NULL:
            return ""
        else:
            return reply

    def start_replication(self):
        retval = IRAPISession_CeStartReplication(self.rapi_conn)
        if retval != TRUE:
            raise RAPIError(retval)

    def sync_start(self, params):
        cdef LPWSTR params_w

        params_w = wstr_from_utf8(params)

        retval = IRAPISession_CeSyncStart(self.rapi_conn, params_w)
        wstr_free_string(params_w)
        if retval != 0:
            raise RAPIError(session=self)

    def sync_resume(self):
        retval = IRAPISession_CeSyncResume(self.rapi_conn)
        if retval != 0:
            raise RAPIError(session=self)

    def sync_pause(self):
        retval = IRAPISession_CeSyncPause(self.rapi_conn)
        if retval != 0:
            raise RAPIError(session=self)

    def SyncTimeToPc(self):
        return IRAPISession_CeSyncTimeToPc(self.rapi_conn)

    def getDiskFreeSpaceEx(self, path):
        """Retrieve the amount of space on a disk volume.

        Takes as an argument a path to a storage location. Returns a tuple
        consisting of free bytes available to the calling user, total number
        of bytes available to the user, and total free bytes."""

        cdef ULARGE_INTEGER freeBytesAvailable
        cdef ULARGE_INTEGER totalNumberOfBytes
        cdef ULARGE_INTEGER totalNumberOfFreeBytes

        retval = IRAPISession_CeGetDiskFreeSpaceEx(self.rapi_conn, path, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)

        #This functions returns 0 if something went wrong....
        if retval == 0:
            raise RAPIError(session=self)

        #return a tuple at the moment, maybe later make this a list
        return (freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes)

    def getSystemPowerStatus(self, refresh):
        """Retrieves the system power status.

        Returns a dictionary containing various details of the power status"""

        cdef SYSTEM_POWER_STATUS_EX powerStatus
        cdef BOOL retval

        with nogil:
            retval = IRAPISession_CeGetSystemPowerStatusEx(self.rapi_conn, &powerStatus, 0 )
        #In contrast to other functions, this returns a boolean,
        #denoting whether the call was succesfull
        if retval == FALSE:
            raise RAPIError(session=self)

        #Now construct the dictionary:
        result = dict()
        result["ACLineStatus"] = powerStatus.ACLineStatus
        result["BatteryFlag"]               = powerStatus.BatteryFlag
        result["BatteryLifePercent"]        = powerStatus.BatteryLifePercent
        result["Reserved1"]                 = powerStatus.Reserved1
        result["BatteryLifeTime"]           = powerStatus.BatteryLifeTime
        result["BatteryFullLifeTime"]       = powerStatus.BatteryFullLifeTime
        result["Reserved2"]                 = powerStatus.Reserved2
        result["BackupBatteryFlag"]         = powerStatus.BackupBatteryFlag
        result["BackupBatteryLifePercent"]  = powerStatus.BackupBatteryLifePercent
        result["Reserved3"]                 = powerStatus.Reserved3
        result["BackupBatteryLifeTime"]     = powerStatus.BackupBatteryLifeTime
        result["BackupBatteryFullLifeTime"] = powerStatus.BackupBatteryFullLifeTime
        return result


    def findAllFiles( self, query, flags ):
        """Retrieves information about files and directories.

        Takes as arguments the path to search in, and a combination of
        filter and search flags. Returns a list containing dictionaries
        of attributes for each entry."""

        cdef LPWSTR query_w
        query_w = wstr_from_utf8(query)

        cdef LPCE_FIND_DATA find_data
        cdef DWORD numberOfFiles

        cdef CE_FIND_DATA found_file

        retval = IRAPISession_CeFindAllFiles(self.rapi_conn, query_w, flags , &numberOfFiles, &find_data )
        wstr_free_string(query_w)
        if retval == 0:
            raise RAPIError(session=self)

        #Now create a list of dictionaries
        result = []
        i=0
        while i < numberOfFiles:
            found_file = find_data[ i ]

            this_file = dict()

            if flags & FAF_ATTRIBUTES:
                this_file["Attributes"] = found_file.dwFileAttributes

            if flags & FAF_CREATION_TIME:
                this_file["CreationLowDateTime"] = found_file.ftCreationTime.dwLowDateTime
                this_file["CreationHighDateTime"] = found_file.ftCreationTime.dwHighDateTime

            if flags & FAF_LASTACCESS_TIME:
                this_file["LastAccessLowDateTime"] = found_file.ftLastAccessTime.dwLowDateTime
                this_file["LastAccessHighDateTime"] = found_file.ftLastAccessTime.dwHighDateTime

            if flags & FAF_LASTWRITE_TIME:
                this_file["LastWriteLowDateTime"] = found_file.ftCreationTime.dwLowDateTime
                this_file["LastWriteHighDateTime"] = found_file.ftCreationTime.dwHighDateTime

            if flags & FAF_SIZE_HIGH:
                this_file["SizeHigh"] = found_file.nFileSizeHigh

            if flags & FAF_SIZE_LOW:
                this_file["SizeLow"] = found_file.nFileSizeLow

            if flags & FAF_OID:
                this_file["OID"] = found_file.dwOID

            if flags & FAF_NAME:
                this_file["Name"] = wstr_to_utf8(found_file.cFileName)


            result.append( this_file )
            i = i + 1

        IRAPISession_CeRapiFreeBuffer(self.rapi_conn, find_data);

        return result


    def file_open(self, filename, mode="r", shareMode=0, flagsAndAttributes=0):
        """Open a file.

        Takes as an argument the name of the file, and the open mode. Returns
        a RAPIFile object."""

        cdef LPWSTR filename_w
        cdef HANDLE fileHandle
        cdef DWORD desiredAccess
        cdef DWORD createDisposition
        cdef DWORD seek_ret

        if mode[0] not in "rwa":
            raise ValueError("mode string must begin with one of 'r', 'w' or 'a', not '%s'" % mode[0])

        filename_w = wstr_from_utf8(filename)

        if mode[0] == "r":
            desiredAccess = GENERIC_READ
            createDisposition = OPEN_EXISTING
        elif mode[0] == "w":
            desiredAccess = GENERIC_WRITE
            createDisposition = CREATE_ALWAYS
        else:
            desiredAccess = GENERIC_WRITE
            createDisposition = OPEN_ALWAYS

        if "+" in mode:
            desiredAccess = desiredAccess|GENERIC_WRITE

        fileHandle = IRAPISession_CeCreateFile(self.rapi_conn, filename_w, desiredAccess, shareMode, NULL, createDisposition, flagsAndAttributes, 0)

        wstr_free_string(filename_w)
        if fileHandle == <HANDLE> -1:
            raise RAPIError(session=self)

        if mode[0] == "a":
            seek_ret = IRAPISession_CeSetFilePointer(self.rapi_conn, fileHandle, 0, NULL, FILE_END)
            if seek_ret == 0xFFFFFFFF:
                raise RAPIError(session=self)

        return RAPIFile(self, fileHandle, filename, mode)


    #TODO: Provide the user with the processInformation
    def createProcess(self, applicationName, applicationParams):
        """Start a remote process.

        Arguments are the name of the application to run, and a
        string of the arguments to the application."""

        cdef PROCESS_INFORMATION processInformation
        cdef LPWSTR applicationName_w
        cdef LPWSTR applicationParams_w

        applicationName_w = wstr_from_utf8(applicationName)
        applicationParams_w = wstr_from_utf8(applicationParams)

        retval = IRAPISession_CeCreateProcess(self.rapi_conn, applicationName_w, applicationParams_w, NULL, NULL, False, 0, NULL, NULL, NULL, &processInformation)

        wstr_free_string(applicationName_w)
        wstr_free_string(applicationParams_w)

        if retval == 0:
            raise RAPIError(session=self)

        return retval


    def getVersion(self):
        """Obtain operating system version information.

        Returns a dictionary of version information."""

        cdef CEOSVERSIONINFO osVersionInfo

        retval = IRAPISession_CeGetVersionEx(self.rapi_conn, &osVersionInfo)

        if retval == 0:
            raise RAPIError(session=self)

		#Now construct the dictionary:
        result = dict()
        result["OSVersionInfoSize"] = osVersionInfo.dwOSVersionInfoSize
        result["MajorVersion"]      = osVersionInfo.dwMajorVersion
        result["MinorVersion"]      = osVersionInfo.dwMinorVersion
        result["BuildNumber"]       = osVersionInfo.dwBuildNumber
        result["PlatformId"]        = osVersionInfo.dwPlatformId

        return result

    def getSystemInfo(self):
        """Obtain information about the system.

        Returns a dictionary containing the system information."""

        cdef SYSTEM_INFO systemInfo

        IRAPISession_CeGetSystemInfo(self.rapi_conn, &systemInfo )

        result = dict()
        result["ProcessorArchitecture"]                 = systemInfo.wProcessorArchitecture
        result["Reserved"]                              = systemInfo.wReserved
        result["PageSize"]                              = systemInfo.dwPageSize
        result["MinimumApplicationAddress"]             = systemInfo.lpMinimumApplicationAddress
        result["MaximumApplicationAddress"]             = systemInfo.lpMaximumApplicationAddress
        result["ActiveProcessorMask"]                   = systemInfo.dwActiveProcessorMask
        result["NumberOfProcessors"]                    = systemInfo.dwNumberOfProcessors
        result["ProcessorType"]                         = systemInfo.dwProcessorType
        result["AllocationGranularity"]                 = systemInfo.dwAllocationGranularity
        result["ProcessorLevel"]                        = systemInfo.wProcessorLevel
        result["ProcessorRevision"]                     = systemInfo.wProcessorRevision

        return result
