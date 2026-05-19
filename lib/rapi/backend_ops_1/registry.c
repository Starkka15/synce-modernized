/* $Id: registry.c 3807 2009-07-19 12:11:08Z mark_ellis $ */
#include "backend_ops_1.h"
#include "rapi_context.h"
#include <stdlib.h>

LONG _CeRegCreateKeyEx( /*{{{*/
                RapiContext *context,
		HKEY hKey,
		LPCWSTR lpszSubKey,
		DWORD Reserved,
		LPWSTR lpszClass,
		DWORD ulOptions,
		REGSAM samDesired,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		PHKEY phkResult,
		LPDWORD lpdwDisposition)
{
	LONG return_value = ERROR_GEN_FAILURE;
	HKEY result = 0;
	DWORD disposition = 0;

	rapi_context_begin_command(context, 0x20);
	rapi_buffer_write_uint32(context->send_buffer, hKey);
	rapi_buffer_write_string(context->send_buffer, lpszSubKey);
	rapi_buffer_write_string(context->send_buffer, lpszClass);
	/* remaining parameters are ignored */

	if ( !rapi_context_call(context) )
		return ERROR_GEN_FAILURE;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

	if (ERROR_SUCCESS == return_value)
	{
		rapi_buffer_read_int32(context->recv_buffer, &result);
		rapi_buffer_read_uint32(context->recv_buffer, &disposition);

		if (phkResult)
			*phkResult = result;

		if (lpdwDisposition)
			*lpdwDisposition = disposition;
	}

	return return_value;
}/*}}}*/

LONG _CeRegOpenKeyEx(/*{{{*/
                RapiContext *context,
		HKEY hKey,
		LPCWSTR lpszSubKey,
		DWORD ulOptions,
		REGSAM samDesired,
		PHKEY phkResult)
{
	LONG return_value = ERROR_GEN_FAILURE;

	rapi_context_begin_command(context, 0x1e);
	rapi_buffer_write_uint32(context->send_buffer, hKey);
	rapi_buffer_write_string(context->send_buffer, lpszSubKey);
	/* remaining parameters are ignored */

	if ( !rapi_context_call(context) )
		return ERROR_GEN_FAILURE;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

	if (ERROR_SUCCESS == return_value)
	{
		if (phkResult)
			rapi_buffer_read_int32(context->recv_buffer, phkResult);
	}

	return return_value;
}/*}}}*/

LONG _CeRegQueryValueEx( /*{{{*/
                RapiContext *context,
		HKEY hKey,
		LPCWSTR lpValueName,
		LPDWORD lpReserved,
		LPDWORD lpType,
		LPBYTE lpData,
		LPDWORD lpcbData)
{
	LONG return_value = ERROR_GEN_FAILURE;

	rapi_context_begin_command(context, 0x26);
	rapi_buffer_write_uint32(context->send_buffer, hKey);
	rapi_buffer_write_optional_string(context->send_buffer, lpValueName);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpReserved, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpType, false);
	rapi_buffer_write_optional       (context->send_buffer, lpData, lpcbData ? *lpcbData : 0, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcbData, true);

	if ( !rapi_context_call(context) )
	{
		synce_trace("rapi_context_call failed");
		return return_value;
	}

	if (!rapi_buffer_read_uint32(context->recv_buffer, &context->last_error))
	{
		synce_trace("rapi_buffer_read_uint32 failed");
		return return_value;
	}
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

	if (ERROR_SUCCESS == return_value)
	{
    DWORD type = 0;

    rapi_buffer_read_optional_uint32(context->recv_buffer, &type);
    if (lpType)
      *lpType = type;

		if (REG_DWORD == type)
			rapi_buffer_read_optional_uint32(context->recv_buffer, (uint32_t*)lpData);
		else
      rapi_buffer_read_optional(context->recv_buffer, lpData, lpcbData ? *lpcbData * sizeof(WCHAR) : 0);

		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcbData);
	}
#if 0
	else
	{
		synce_trace("CeRegQueryValueEx returning %i", return_value);
	}
#endif

	return return_value;
}/*}}}*/

LONG _CeRegCloseKey(/*{{{*/
                RapiContext *context,
		HKEY hKey)
{
	LONG return_value = ERROR_GEN_FAILURE;

	rapi_context_begin_command(context, 0x21);
	rapi_buffer_write_uint32(context->send_buffer, hKey);

	if ( !rapi_context_call(context) )
		return ERROR_GEN_FAILURE;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

	return return_value;
}/*}}}*/

LONG _CeRegQueryInfoKey( /*{{{*/
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
		PFILETIME lpftLastWriteTime)
{
	LONG return_value = ERROR_GEN_FAILURE;

	rapi_context_begin_command(context, 0x25);
	rapi_buffer_write_uint32         (context->send_buffer, hKey);
	rapi_buffer_write_optional       (context->send_buffer, lpClass, lpcbClass ? *lpcbClass * sizeof(WCHAR) : 0, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcbClass, true);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpReserved, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcSubKeys, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcbMaxSubKeyLen, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcbMaxClassLen, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcValues, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcbMaxValueNameLen, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcbMaxValueLen, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcbSecurityDescriptor, false);
	rapi_buffer_write_optional       (context->send_buffer, lpftLastWriteTime, sizeof(FILETIME), false);

	if ( !rapi_context_call(context) )
		return ERROR_GEN_FAILURE;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

	if (ERROR_SUCCESS == return_value)
	{
		rapi_buffer_read_optional       (context->recv_buffer, lpClass, lpcbClass ? *lpcbClass * sizeof(WCHAR) : 0);
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcbClass);
		if (lpReserved)
		  *lpReserved = 0;
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcSubKeys);
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcbMaxSubKeyLen);
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcbMaxClassLen);
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcValues);
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcbMaxValueNameLen);
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcbMaxValueLen);
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcbSecurityDescriptor);
		rapi_buffer_read_optional_filetime(context->recv_buffer, lpftLastWriteTime);
	}

	return return_value;
}/*}}}*/

LONG _CeRegEnumValue( /*{{{*/
                RapiContext *context,
		HKEY hKey,
		DWORD dwIndex,
		LPWSTR lpszValueName,
		LPDWORD lpcbValueName,
		LPDWORD lpReserved,
		LPDWORD lpType,
		LPBYTE lpData,
		LPDWORD lpcbData)
{
	LONG return_value = ERROR_GEN_FAILURE;

	rapi_context_begin_command(context, 0x23);
	rapi_buffer_write_uint32         (context->send_buffer, hKey);
	rapi_buffer_write_uint32         (context->send_buffer, dwIndex);
	rapi_buffer_write_optional       (context->send_buffer, lpszValueName, lpcbValueName ? *lpcbValueName * sizeof(WCHAR) : 0, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcbValueName, true);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpReserved, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpType, false);
	rapi_buffer_write_optional       (context->send_buffer, lpData, lpcbData ? *lpcbData : 0, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcbData, true);

	if ( !rapi_context_call(context) )
		return ERROR_GEN_FAILURE;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

	if (ERROR_SUCCESS == return_value)
	{
		rapi_buffer_read_optional       (context->recv_buffer, lpszValueName, lpcbValueName ? *lpcbValueName * sizeof(WCHAR) : 0);
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcbValueName);
		if (lpReserved)
		  *lpReserved = 0;
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpType);
		rapi_buffer_read_optional       (context->recv_buffer, lpData, lpcbData ? *lpcbData : 0);
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcbData);
	}

	return return_value;
}/*}}}*/

LONG _CeRegEnumKeyEx( /*{{{*/
                RapiContext *context,
		HKEY hKey,
		DWORD dwIndex,
		LPWSTR lpName,
		LPDWORD lpcbName,
		LPDWORD lpReserved,
		LPWSTR lpClass,
		LPDWORD lpcbClass,
		PFILETIME lpftLastWriteTime)
{
	LONG return_value = ERROR_GEN_FAILURE;

	rapi_context_begin_command(context, 0x1f);
	rapi_buffer_write_uint32         (context->send_buffer, hKey);
	rapi_buffer_write_uint32         (context->send_buffer, dwIndex);
	rapi_buffer_write_optional       (context->send_buffer, lpName, lpcbName ? *lpcbName * sizeof(WCHAR) : 0, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcbName, true);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpReserved, false);
	rapi_buffer_write_optional       (context->send_buffer, lpClass, lpcbClass ? *lpcbClass * sizeof(WCHAR) : 0, false);
	rapi_buffer_write_optional_uint32(context->send_buffer, lpcbClass, true);
	rapi_buffer_write_optional       (context->send_buffer, lpftLastWriteTime, sizeof(FILETIME), false);

	if ( !rapi_context_call(context) )
		return ERROR_GEN_FAILURE;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

	if (ERROR_SUCCESS == return_value)
	{
		rapi_buffer_read_optional       (context->recv_buffer, lpName, lpcbName ? *lpcbName * sizeof(WCHAR) : 0);
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcbName);
		if (lpReserved)
		  *lpReserved = 0;
		rapi_buffer_read_optional       (context->recv_buffer, lpClass, lpcbClass ? *lpcbClass * sizeof(WCHAR) : 0);
		rapi_buffer_read_optional_uint32(context->recv_buffer, lpcbClass);

    if (lpftLastWriteTime)
      rapi_buffer_read_optional_filetime(context->recv_buffer, lpftLastWriteTime);
  }

	return return_value;
}/*}}}*/

LONG _CeRegSetValueEx( /*{{{*/
                RapiContext *context,
		HKEY hKey,
		LPCWSTR lpValueName,
		DWORD Reserved,
		DWORD dwType,
		const BYTE *lpData,
		DWORD cbData)
{
	LONG return_value = ERROR_GEN_FAILURE;

	rapi_context_begin_command(context, 0x27);
	rapi_buffer_write_uint32(context->send_buffer, hKey);
	rapi_buffer_write_optional_string(context->send_buffer, lpValueName);
	rapi_buffer_write_uint32(context->send_buffer, dwType);
	if (dwType == REG_DWORD)
	{
		rapi_buffer_write_optional_uint32 (context->send_buffer, (uint32_t*)lpData, true);
	}
	else
	{
	rapi_buffer_write_optional(context->send_buffer, lpData, cbData, true);
	}
	rapi_buffer_write_uint32(context->send_buffer, cbData);

	if ( !rapi_context_call(context) )
		return ERROR_GEN_FAILURE;

	if (!rapi_buffer_read_uint32(context->recv_buffer, &context->last_error))
		return ERROR_GEN_FAILURE;
	if (!rapi_buffer_read_int32(context->recv_buffer, &return_value))
		return ERROR_GEN_FAILURE;

	return return_value;
}/*}}}*/

LONG _CeRegDeleteValue( /*{{{*/
                RapiContext *context,
		HKEY hKey,
		LPCWSTR lpszValueName)
{
	LONG return_value = ERROR_GEN_FAILURE;

	rapi_context_begin_command(context, 0x24);
	rapi_buffer_write_uint32(context->send_buffer, hKey);
	rapi_buffer_write_optional_string(context->send_buffer, lpszValueName);

	if ( !rapi_context_call(context) )
		return ERROR_GEN_FAILURE;

	if (!rapi_buffer_read_uint32(context->recv_buffer, &context->last_error))
		return ERROR_GEN_FAILURE;
	if (!rapi_buffer_read_int32(context->recv_buffer, &return_value))
		return ERROR_GEN_FAILURE;

	return return_value;
}/*}}}*/

LONG _CeRegDeleteKey( /*{{{*/
                RapiContext *context,
		HKEY hKey,
		LPCWSTR lpszSubKey)
{
	LONG return_value = ERROR_GEN_FAILURE;

	rapi_context_begin_command(context, 0x22);
	rapi_buffer_write_uint32(context->send_buffer, hKey);
	rapi_buffer_write_optional_string(context->send_buffer, lpszSubKey);

	if ( !rapi_context_call(context) )
		return ERROR_GEN_FAILURE;

	if (!rapi_buffer_read_uint32(context->recv_buffer, &context->last_error))
		return ERROR_GEN_FAILURE;
	if (!rapi_buffer_read_int32(context->recv_buffer, &return_value))
		return ERROR_GEN_FAILURE;

	return return_value;
}/*}}}*/
