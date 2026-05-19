/* $Id: file_management2.c 3807 2009-07-19 12:11:08Z mark_ellis $ */
#include "rapi_internal.h"
#include "backend_ops_2.h"
#include "rapi_buffer.h"
#include "rapi_context.h"
#include <string.h>
#include <stdlib.h>

DWORD _CeGetSpecialFolderPath2(
        RapiContext *context,
        int nFolder,
        DWORD nBufferLength,
        LPWSTR lpBuffer)
{
    DWORD string_length = nBufferLength;

    rapi_context_begin_command(context, 0x4b);
    rapi_buffer_write_uint32(context->send_buffer, (uint32_t)nFolder);
    rapi_buffer_write_uint32(context->send_buffer, nBufferLength);

    if ( !rapi2_context_call(context) )
        return 0;

    if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
        return 0;
    synce_trace("last_error = %i", context->last_error);

    if ( !rapi_buffer_read_string(context->recv_buffer, lpBuffer, &string_length) )
        return 0;

    return string_length;
}


BOOL _CeFindAllFiles2(
        RapiContext *context,
        LPCWSTR szPath,
        DWORD dwFlags,
        LPDWORD lpdwFoundCount,
        LPLPCE_FIND_DATA ppFindDataArray)
{
    uint32_t count = 0;

    rapi_context_begin_command(context, 0x1a);
    rapi2_buffer_write_string(context->send_buffer, szPath);
    rapi_buffer_write_uint32(context->send_buffer, dwFlags);

    if ( !rapi2_context_call(context) )
        return false;

    /* TODO Don't know this two uint32_t fields */
    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    uint32_t unknown;
    rapi_buffer_read_uint32(context->recv_buffer, &unknown);

    rapi_buffer_read_uint32(context->recv_buffer, &count);

    synce_trace("found %i files", count);

    if (count)
    {
        unsigned i;
        uint32_t name_size;
        CE_FIND_DATA* array = calloc(count, sizeof(CE_FIND_DATA));

        if (!array)
            return false;

        for (i = 0; i < count; i++)
        {
            if (dwFlags & FAF_NAME)
                rapi_buffer_read_uint32(context->recv_buffer, &name_size);

            if (dwFlags & FAF_ATTRIBUTES)
                rapi_buffer_read_uint32(context->recv_buffer, &array[i].dwFileAttributes);

            if (dwFlags & FAF_CREATION_TIME)
            {
                rapi_buffer_read_uint32(context->recv_buffer, &array[i].ftCreationTime.dwLowDateTime);
                rapi_buffer_read_uint32(context->recv_buffer, &array[i].ftCreationTime.dwHighDateTime);
            }

            if (dwFlags & FAF_LASTACCESS_TIME)
            {
                rapi_buffer_read_uint32(context->recv_buffer, &array[i].ftLastAccessTime.dwLowDateTime);
                rapi_buffer_read_uint32(context->recv_buffer, &array[i].ftLastAccessTime.dwHighDateTime);
            }

            if (dwFlags & FAF_LASTWRITE_TIME)
            {
                rapi_buffer_read_uint32(context->recv_buffer, &array[i].ftLastWriteTime.dwLowDateTime);
                rapi_buffer_read_uint32(context->recv_buffer, &array[i].ftLastWriteTime.dwHighDateTime);
            }

            if (dwFlags & FAF_SIZE_HIGH)
                rapi_buffer_read_uint32(context->recv_buffer, &array[i].nFileSizeHigh);

            if (dwFlags & FAF_SIZE_LOW)
                rapi_buffer_read_uint32(context->recv_buffer, &array[i].nFileSizeLow);

            if (dwFlags & FAF_OID)
                rapi_buffer_read_uint32(context->recv_buffer, &array[i].dwOID);

            if (dwFlags & FAF_NAME)
            {
                rapi_buffer_read_data(context->recv_buffer, array[i].cFileName, name_size );
                synce_trace_wstr(array[i].cFileName);
            }
        }

        if (ppFindDataArray)
            *ppFindDataArray = array;
    }

    if (lpdwFoundCount)
        *lpdwFoundCount = count;

    return true;
}


HANDLE _CeFindFirstFile2(
        RapiContext *context,
        LPCWSTR lpFileName,
        LPCE_FIND_DATA lpFindFileData)
{
    HANDLE handle = INVALID_HANDLE_VALUE;

    rapi_context_begin_command(context, 0x11);
    rapi2_buffer_write_string(context->send_buffer, lpFileName);

    if ( !rapi2_context_call(context) )
        return INVALID_HANDLE_VALUE;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &handle);

    if ( !rapi_buffer_read_find_data(context->recv_buffer, lpFindFileData) )
        return INVALID_HANDLE_VALUE;

    return handle;
}


BOOL _CeFindNextFile2(
        RapiContext *context,
        HANDLE hFindFile,
        LPCE_FIND_DATA lpFindFileData)
{
    BOOL return_value = false;

    rapi_context_begin_command(context, 0x12);
    rapi_buffer_write_uint32(context->send_buffer, hFindFile);

    if ( !rapi2_context_call(context) )
        return false;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &return_value);

    if ( !rapi_buffer_read_find_data(context->recv_buffer, lpFindFileData) )
        return false;

    return return_value;
}


BOOL _CeFindClose2(
        RapiContext *context,
        HANDLE hFindFile)
{
    BOOL return_value = 0;

    synce_trace("begin");

    rapi_context_begin_command(context, 0x13);
    rapi_buffer_write_uint32(context->send_buffer, hFindFile);

    if ( !rapi2_context_call(context) )
        return false;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &return_value);

    return return_value;
}


DWORD _CeGetFileAttributes2(
        RapiContext *context,
        LPCWSTR lpFileName)
{
    DWORD return_value = 0xFFFFFFFF;

    rapi_context_begin_command(context, 0x14);
    rapi2_buffer_write_string(context->send_buffer, lpFileName);

    if ( !rapi2_context_call(context) )
        goto exit;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &return_value);

exit:
    return return_value;
}


BOOL _CeDeleteFile2(
        RapiContext *context,
        LPCWSTR lpFileName)
{
    BOOL return_value = 0;

    rapi_context_begin_command(context, 0x2d);
    /*    rapi_buffer_write_optional_string(context->send_buffer, lpFileName);*/
    rapi2_buffer_write_string(context->send_buffer, lpFileName);

    if ( !rapi2_context_call(context) )
        return 0;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &return_value);

    return return_value;
}


BOOL _CeCreateDirectory2(
        RapiContext *context,
        LPCWSTR lpPathName,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    BOOL return_value = 0;

    rapi_context_begin_command(context, 0x28);
    rapi2_buffer_write_string(context->send_buffer, lpPathName);
    /*
    rapi_buffer_write_optional_string(context->send_buffer, lpPathName);
    rapi_buffer_write_optional_in(context->send_buffer, NULL, 0); */ /* lpSecurityAttributes */

    if ( !rapi2_context_call(context) )
        return 0;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &return_value);

    return return_value;
}


BOOL _CeRemoveDirectory2(
        RapiContext *context,
        LPCWSTR lpPathName)
{
    BOOL return_value = 0;

    rapi_context_begin_command(context, 0x29);
    rapi2_buffer_write_string(context->send_buffer, lpPathName);
    /*    rapi_buffer_write_optional_string(context->send_buffer, lpPathName); */

    if ( !rapi2_context_call(context) )
        return 0;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &return_value);

    return return_value;
}


DWORD _CeGetFileSize2(
        RapiContext *context,
        HANDLE hFile,
        LPDWORD lpFileSizeHigh)
{
    DWORD size = INVALID_FILE_SIZE;

    rapi_context_begin_command(context, 0x2e);
    rapi_buffer_write_uint32(context->send_buffer, (uint32_t)hFile);
    rapi_buffer_write_optional_out(context->send_buffer,
    lpFileSizeHigh, sizeof(*lpFileSizeHigh));

    if ( !rapi2_context_call(context) )
        return INVALID_FILE_SIZE;

    if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
        return INVALID_FILE_SIZE;
    synce_trace("last_error = %i", context->last_error);

    if ( !rapi_buffer_read_uint32(context->recv_buffer, &size) )
        return INVALID_FILE_SIZE;

    if ( !rapi_buffer_read_optional_uint32(context->recv_buffer, lpFileSizeHigh) )
        return INVALID_FILE_SIZE;

    return size;
}


/**
 * This function copies an existing file to a new file.
 */
BOOL _CeCopyFile2(
        RapiContext *context,
        LPCWSTR lpExistingFileName,
        LPCWSTR lpNewFileName,
        BOOL bFailIfExists)
{
    BOOL return_value = 0;

    rapi_context_begin_command(context, 0x2c);
    rapi2_buffer_write_string(context->send_buffer, lpExistingFileName);
    rapi2_buffer_write_string(context->send_buffer, lpNewFileName);
    rapi_buffer_write_uint32(context->send_buffer, bFailIfExists);

    if ( !rapi2_context_call(context) )
        return false;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &return_value);

    return return_value;
}

BOOL _CeMoveFile2(
        RapiContext *context,
        LPCWSTR lpExistingFileName,
        LPCWSTR lpNewFileName)
{
    BOOL return_value = 0;

    rapi_context_begin_command(context, 0x2b);
    rapi2_buffer_write_string(context->send_buffer, lpExistingFileName);
    rapi2_buffer_write_string(context->send_buffer, lpNewFileName);

    if ( !rapi2_context_call(context) )
        return 0;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &return_value);

    return return_value;
}

BOOL _NotImplementedCeSetFileAttributes2(
		RapiContext *context,
		LPCWSTR lpFileName,
		DWORD dwFileAttributes)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return FALSE;
}

BOOL _NotImplementedCeSHCreateShortcut2(
  RapiContext *context,
  LPCWSTR lpszShortcut,
  LPCWSTR lpszTarget)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return FALSE;
}
