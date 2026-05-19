/* $Id: file_access2.c 3807 2009-07-19 12:11:08Z mark_ellis $ */
#include "rapi_internal.h"
#include "backend_ops_2.h"
#include "rapi_buffer.h"
#include "rapi_context.h"
#include <string.h>
#include <stdlib.h>


HANDLE _CeCreateFile2(
        RapiContext *context,
        LPCWSTR lpFileName,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile)
{
    HANDLE handle = INVALID_HANDLE_VALUE;

    synce_trace("begin");

    rapi_context_begin_command(context, 0x16);
    rapi2_buffer_write_string(context->send_buffer, lpFileName);
    rapi_buffer_write_uint32(context->send_buffer, dwDesiredAccess);
    rapi_buffer_write_uint32(context->send_buffer, dwShareMode);
    rapi_buffer_write_uint32(context->send_buffer, dwCreationDisposition);
    rapi_buffer_write_uint32(context->send_buffer, dwFlagsAndAttributes);
    rapi_buffer_write_uint32(context->send_buffer, hTemplateFile);

    if ( !rapi2_context_call(context) )
        return INVALID_HANDLE_VALUE;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &handle);

    return handle;
}


BOOL _CeReadFile2(
        RapiContext *context,
        HANDLE hFile,
        LPVOID lpBuffer,
        DWORD nNumberOfBytesToRead,
        LPDWORD lpNumberOfBytesRead,
        LPOVERLAPPED lpOverlapped)
{
    BOOL return_value = 0;
    uint32_t bytes_read = 0;

    synce_trace("begin");

    rapi_context_begin_command(context, 0x17);
    rapi_buffer_write_uint32(context->send_buffer, hFile);
    rapi_buffer_write_uint32(context->send_buffer, nNumberOfBytesToRead);
/*    rapi_buffer_write_optional_out(context->send_buffer, lpBuffer, nNumberOfBytesToRead);
    rapi_buffer_write_optional_in(context->send_buffer, NULL, 0); *//* lpOverlapped */

    if ( !rapi2_context_call(context) )
    {
        synce_error("rapi2_context_call failed");
        return false;
    }

    if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
        return false;
    synce_trace("context->last_error=0x%08x", context->last_error);

    if ( !rapi_buffer_read_uint32(context->recv_buffer, &return_value) )
        return false;
    synce_trace("return_value=0x%08x", return_value);

    if ( !rapi_buffer_read_uint32(context->recv_buffer, &bytes_read) )
        return false;

    if (lpNumberOfBytesRead)
        *lpNumberOfBytesRead = bytes_read;

    if (lpBuffer)
        if ( !rapi_buffer_read_data(context->recv_buffer, lpBuffer, bytes_read) )
            return false;

    return return_value;
}


BOOL _CeWriteFile2(
        RapiContext *context,
        HANDLE hFile,
        LPCVOID lpBuffer,
        DWORD nNumberOfBytesToWrite,
        LPDWORD lpNumberOfBytesWritten,
        LPOVERLAPPED lpOverlapped)
{
    BOOL return_value = 0;
    uint32_t bytes_written = 0;

    synce_trace("begin");

    rapi_context_begin_command(context, 0x18);
    rapi_buffer_write_uint32(context->send_buffer, hFile);
    rapi_buffer_write_uint32(context->send_buffer, nNumberOfBytesToWrite);
    rapi_buffer_write_data(context->send_buffer, lpBuffer, nNumberOfBytesToWrite);
/*    rapi_buffer_write_optional_in(context->send_buffer, lpBuffer, nNumberOfBytesToWrite);
    rapi_buffer_write_optional_in(context->send_buffer, NULL, 0);*/ /* lpOverlapped */

    if ( !rapi2_context_call(context) )
        return false;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &return_value);

    rapi_buffer_read_uint32(context->recv_buffer, &bytes_written);
    if (lpNumberOfBytesWritten)
        *lpNumberOfBytesWritten = bytes_written;

    return return_value;
}


DWORD _CeSetFilePointer2(
        RapiContext *context,
        HANDLE hFile,
        LONG lDistanceToMove,
        PLONG lpDistanceToMoveHigh,
        DWORD dwMoveMethod)
{
    DWORD return_value = 0;

    synce_trace("begin");

    rapi_context_begin_command(context, 0x26);
    rapi_buffer_write_uint32(context->send_buffer, hFile);
    rapi_buffer_write_uint32(context->send_buffer, lDistanceToMove);

    rapi_buffer_write_uint32(context->send_buffer,
        (lpDistanceToMoveHigh != NULL));

    if ( lpDistanceToMoveHigh != NULL )
    {
        rapi_buffer_write_uint32(context->send_buffer, *lpDistanceToMoveHigh);
    }

    rapi_buffer_write_uint32(context->send_buffer, dwMoveMethod);

    if ( !rapi2_context_call(context) )
        return false;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &return_value);

    if ( lpDistanceToMoveHigh != NULL )
    {
        rapi_buffer_read_int32(context->recv_buffer, lpDistanceToMoveHigh);
    }

    return return_value;
}

BOOL _CeSetEndOfFile2(
        RapiContext *context,
        HANDLE hFile)
{
    BOOL return_value = 0;

    synce_trace("begin");

    rapi_context_begin_command(context,0x27);
    rapi_buffer_write_uint32(context->send_buffer, hFile);

    if ( !rapi_context_call(context) )
        return false;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &return_value);

    return return_value;
}

BOOL _CeGetFileTime2(
        RapiContext *context,
        HANDLE hFile,
        LPFILETIME lpCreationTime,
        LPFILETIME lpLastAccessTime,
        LPFILETIME lpLastWriteTime)
{
    BOOL return_value = FALSE;
    FILETIME tmp_ft;

    if (lpCreationTime)
    {
        memset(lpCreationTime, 0, sizeof(FILETIME));
    }

    if (lpLastAccessTime)
    {
        memset(lpLastAccessTime, 0, sizeof(FILETIME));
    }

    if (lpLastWriteTime)
    {
        memset(lpLastWriteTime, 0, sizeof(FILETIME));
    }

    rapi_context_begin_command(context, 0x41);
    rapi_buffer_write_uint32(context->send_buffer, hFile);

    if (!rapi2_context_call(context))
        return FALSE;

    if (!rapi_buffer_read_uint32(context->recv_buffer, &context->last_error))
        return FALSE;

    if (!rapi_buffer_read_uint32(context->recv_buffer, &return_value))
        return FALSE;

    if (!rapi_buffer_read_data(context->recv_buffer, &tmp_ft, sizeof(tmp_ft)))
        return return_value;

    if (lpCreationTime)
        memcpy(lpCreationTime, &tmp_ft, sizeof(tmp_ft));

    if (!rapi_buffer_read_data(context->recv_buffer, &tmp_ft, sizeof(tmp_ft)))
        return return_value;

    if (lpLastAccessTime)
        memcpy(lpLastAccessTime, &tmp_ft, sizeof(tmp_ft));

    if (!rapi_buffer_read_data(context->recv_buffer, &tmp_ft, sizeof(tmp_ft)))
        return return_value;

    if (lpLastWriteTime)
        memcpy(lpLastWriteTime, &tmp_ft, sizeof(tmp_ft));

    return return_value;
}


BOOL _CeSetFileTime2(
        RapiContext *context,
        HANDLE hFile,
        LPFILETIME lpCreationTime,
        LPFILETIME lpLastAccessTime,
        LPFILETIME lpLastWriteTime)
{
    BOOL return_value = FALSE;

    rapi_context_begin_command(context, 0x42);
    rapi_buffer_write_uint32(context->send_buffer, hFile);

    rapi_buffer_write_uint32(context->send_buffer,
            (lpCreationTime) ? sizeof(FILETIME) : 0);

    if (lpCreationTime)
    {
        rapi_buffer_write_data(context->send_buffer, lpCreationTime,
                sizeof(FILETIME));
    }

    rapi_buffer_write_uint32(context->send_buffer,
            (lpLastAccessTime) ? sizeof(FILETIME) : 0);

    if (lpLastAccessTime)
    {
        rapi_buffer_write_data(context->send_buffer, lpLastAccessTime,
                sizeof(FILETIME));
    }

    rapi_buffer_write_uint32(context->send_buffer,
            (lpLastWriteTime) ? sizeof(FILETIME) : 0);

    if (lpLastWriteTime)
    {
        rapi_buffer_write_data(context->send_buffer, lpLastWriteTime,
                sizeof(FILETIME));
    }

    if (!rapi2_context_call(context))
        return FALSE;

    if (!rapi_buffer_read_uint32(context->recv_buffer, &context->last_error))
        return FALSE;

    if (!rapi_buffer_read_uint32(context->recv_buffer, &return_value))
        return FALSE;

    return return_value;
}


BOOL _CeCloseHandle2(
        RapiContext *context,
        HANDLE hObject)
{
    BOOL return_value = 0;

    synce_trace("begin");

    rapi_context_begin_command(context, 0x19);
    rapi_buffer_write_uint32(context->send_buffer, hObject);

    if ( !rapi2_context_call(context) )
        return false;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_uint32(context->recv_buffer, &return_value);

    return return_value;
}
