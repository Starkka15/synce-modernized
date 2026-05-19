/* $Id: file_access.c 3807 2009-07-19 12:11:08Z mark_ellis $ */
#include "rapi_internal.h"
#include "backend_ops_1.h"
#include "rapi_buffer.h"
#include "rapi_context.h"


BOOL _CeCloseHandle(
                RapiContext *context,
		HANDLE hObject)
{
	BOOL return_value = 0;

	/*synce_trace("begin");*/

	rapi_context_begin_command(context, 0x08);
	rapi_buffer_write_uint32(context->send_buffer, hObject);

	if ( !rapi_context_call(context) )
		return false;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_uint32(context->recv_buffer, &return_value);

	return return_value;
}

HANDLE _CeCreateFile(
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

	rapi_context_begin_command(context, 0x05);
	rapi_buffer_write_uint32(context->send_buffer, dwDesiredAccess);
	rapi_buffer_write_uint32(context->send_buffer, dwShareMode);
	rapi_buffer_write_uint32(context->send_buffer, dwCreationDisposition);
	rapi_buffer_write_uint32(context->send_buffer, dwFlagsAndAttributes);
	rapi_buffer_write_uint32(context->send_buffer, hTemplateFile);
	rapi_buffer_write_string(context->send_buffer, lpFileName);

	if ( !rapi_context_call(context) )
		return INVALID_HANDLE_VALUE;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_uint32(context->recv_buffer, &handle);

	return handle;
}

BOOL _CeReadFile(
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

	rapi_context_begin_command(context, 0x06);
	rapi_buffer_write_uint32(context->send_buffer, hFile);
	rapi_buffer_write_optional_out(context->send_buffer, lpBuffer, nNumberOfBytesToRead);
	rapi_buffer_write_optional_in(context->send_buffer, NULL, 0); /* lpOverlapped */

	if ( !rapi_context_call(context) )
	{
		synce_error("rapi_context_call failed");
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

BOOL _CeWriteFile(
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

	rapi_context_begin_command(context, 0x07);
	rapi_buffer_write_uint32(context->send_buffer, hFile);
	rapi_buffer_write_optional_in(context->send_buffer, lpBuffer, nNumberOfBytesToWrite);
	rapi_buffer_write_optional_in(context->send_buffer, NULL, 0); /* lpOverlapped */

	if ( !rapi_context_call(context) )
		return false;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_uint32(context->recv_buffer, &return_value);

	rapi_buffer_read_uint32(context->recv_buffer, &bytes_written);
	if (lpNumberOfBytesWritten)
		*lpNumberOfBytesWritten = bytes_written;

	return return_value;

}

BOOL _CeSetEndOfFile(
                RapiContext *context,
		HANDLE hFile)
{
	BOOL return_value = 0;
	
	synce_trace("Setting end of file");
	
	rapi_context_begin_command(context,0x16);
	rapi_buffer_write_uint32(context->send_buffer, hFile);

	if ( !rapi_context_call(context) )
		goto exit;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_uint32(context->recv_buffer, &return_value);

exit:

	return return_value;
}

DWORD _CeSetFilePointer(
                RapiContext *context,
		HANDLE hFile,
		LONG lDistanceToMove,
		LONG *lpDistanceToMoveHigh,
		DWORD dwMoveMethod)
{
	BOOL return_value = 0;
	
	synce_trace("Setting file pointer");
	
	rapi_context_begin_command(context, 0x15);
	rapi_buffer_write_uint32(context->send_buffer, hFile);
	rapi_buffer_write_uint32(context->send_buffer, lDistanceToMove);
	rapi_buffer_write_uint32(context->send_buffer, lpDistanceToMoveHigh ? *lpDistanceToMoveHigh : 0);
	rapi_buffer_write_uint32(context->send_buffer, dwMoveMethod);

	if ( !rapi_context_call(context) )
		goto exit;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_uint32(context->recv_buffer, &return_value);

exit:
	return return_value;

}


BOOL _NotImplementedCeGetFileTime(
                RapiContext *context,
                HANDLE hFile,
                LPFILETIME lpCreationTime,
                LPFILETIME lpLastAccessTime,
                LPFILETIME lpLastWriteTime)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return FALSE;
}


BOOL _NotImplementedCeSetFileTime(
                RapiContext *context,
                HANDLE hFile,
                LPFILETIME lpCreationTime,
                LPFILETIME lpLastAccessTime,
                LPFILETIME lpLastWriteTime)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return FALSE;
}

