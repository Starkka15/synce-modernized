/* $Id: registry.c 2355 2006-04-07 18:47:20Z voc $ */
#include "backend_ops_2.h"
#include "rapi_context.h"
#include <stdlib.h>
#include <stdio.h>


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
		LPDWORD lpdwDisposition)
{
	LONG return_value = ERROR_GEN_FAILURE;
	HKEY result = 0;
	DWORD disposition = 0;

	rapi_context_begin_command(context, 0x31);
	rapi_buffer_write_uint32(context->send_buffer, hKey);
	rapi2_buffer_write_string(context->send_buffer, lpszSubKey);
	rapi2_buffer_write_string(context->send_buffer, lpszClass);

	if ( !rapi2_context_call(context) )
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
}

LONG _CeRegOpenKeyEx2(
		RapiContext *context,
		HKEY hKey,
		LPCWSTR lpszSubKey,
		DWORD ulOptions,
		REGSAM samDesired,
		PHKEY phkResult)
{
	LONG return_value = ERROR_GEN_FAILURE;

	rapi_context_begin_command(context, 0x2f);
	rapi_buffer_write_uint32(context->send_buffer, hKey);
	rapi2_buffer_write_string(context->send_buffer, lpszSubKey);

	if ( !rapi2_context_call(context) )
		return ERROR_GEN_FAILURE;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

	if (ERROR_SUCCESS == return_value)
	{
		if (phkResult)
			rapi_buffer_read_int32(context->recv_buffer, phkResult);
	}

	return return_value;
}


LONG _CeRegQueryValueEx2(
		RapiContext *context,
		HKEY hKey,
		LPCWSTR lpValueName,
		LPDWORD lpReserved,
		LPDWORD lpType,
		LPBYTE lpData,
		LPDWORD lpcbData)
{
	LONG return_value = ERROR_GEN_FAILURE;

        if (lpData && (!lpcbData))
                return ERROR_INVALID_PARAMETER;

	rapi_context_begin_command(context, 0x37);
	rapi_buffer_write_uint32(context->send_buffer, hKey);
	rapi2_buffer_write_string(context->send_buffer, lpValueName);
    rapi_buffer_write_uint32(context->send_buffer, *lpcbData);

	if ( !rapi2_context_call(context) )
	{
		synce_trace("rapi2_context_call failed");
		return return_value;
	}

	if (!rapi_buffer_read_uint32(context->recv_buffer, &context->last_error))
	{
		synce_trace("rapi_buffer_read_uint32 failed");
		return return_value;
	}
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

        if (ERROR_SUCCESS == return_value) {
                DWORD tmp_dword = 0;
                DWORD data_buf_size = 0;

                rapi_buffer_read_uint32(context->recv_buffer, &tmp_dword);
                if (lpType)
                        *lpType = tmp_dword;

                rapi_buffer_read_uint32(context->recv_buffer, &tmp_dword);
                if (lpcbData) {
                        data_buf_size = *lpcbData;
			*lpcbData = tmp_dword;
                }

                if (lpData) {
                        rapi_buffer_read_data(context->recv_buffer, lpData, *lpcbData > data_buf_size ? data_buf_size : *lpcbData);
                }
        }
#if 0
	else
	{
		synce_trace("CeRegQueryValueEx returning %i", return_value);
	}
#endif

	return return_value;
}


LONG _CeRegCloseKey2(
		RapiContext *context,
		HKEY hKey)
{
	LONG return_value = ERROR_GEN_FAILURE;

	rapi_context_begin_command(context, 0x32);
	rapi_buffer_write_uint32(context->send_buffer, hKey);

	if ( !rapi2_context_call(context) )
		return ERROR_GEN_FAILURE;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

	return return_value;
}


LONG _CeRegDeleteKey2(
        RapiContext *context,
        HKEY hKey,
        LPCWSTR lpszSubKey)
{
    LONG return_value = ERROR_GEN_FAILURE;

    rapi_context_begin_command(context, 0x33);
    rapi_buffer_write_uint32(context->send_buffer, hKey);
    rapi2_buffer_write_string(context->send_buffer, lpszSubKey);

    if ( !rapi2_context_call(context) )
      return ERROR_GEN_FAILURE;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_int32(context->recv_buffer, &return_value);

    return return_value;
}


LONG _CeRegDeleteValue2(
        RapiContext *context,
        HKEY hKey,
        LPCWSTR lpszValueName)
{
    LONG return_value = ERROR_GEN_FAILURE;

    rapi_context_begin_command(context, 0x35);
    rapi_buffer_write_uint32(context->send_buffer, hKey);
    rapi2_buffer_write_string(context->send_buffer, lpszValueName);

    if ( !rapi2_context_call(context) )
      return ERROR_GEN_FAILURE;

    rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
    rapi_buffer_read_int32(context->recv_buffer, &return_value);

    return return_value;
}



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
		PFILETIME lpftLastWriteTime)
{
	LONG return_value = ERROR_GEN_FAILURE;

        if (lpClass && (!lpcbClass))
                return ERROR_INVALID_PARAMETER;

	rapi_context_begin_command(context, 0x36);
	rapi_buffer_write_uint32        (context->send_buffer, hKey);
	rapi_buffer_write_uint32        (context->send_buffer, lpcbClass ? *lpcbClass : 0 );

	//lpftLastWriteTime, this should be put to 0
	rapi_buffer_write_uint32        (context->send_buffer, 0);

	if ( !rapi2_context_call(context) )
		return ERROR_GEN_FAILURE;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

       	if (ERROR_SUCCESS == return_value)
	{
		DWORD foo = 0 ; 
		rapi_buffer_read_uint32 (context->recv_buffer, &foo ) ; 
		if (lpcbClass)
			*lpcbClass = foo ; 
		if (lpClass)
		{
			rapi_buffer_read_string(context->recv_buffer, lpClass, lpcbClass );
		}

		rapi_buffer_read_uint32 (context->recv_buffer, &foo ) ; 
		if (lpcSubKeys)
			*lpcSubKeys = foo ; 
		rapi_buffer_read_uint32 (context->recv_buffer, &foo ) ; 
		if (lpcbMaxSubKeyLen)
			*lpcbMaxSubKeyLen = foo ; 
		rapi_buffer_read_uint32 (context->recv_buffer, &foo ) ; 
		if (lpcbMaxClassLen)
			*lpcbMaxClassLen = foo ; 
		rapi_buffer_read_uint32 (context->recv_buffer, &foo ) ; 
		if (lpcValues)
			*lpcValues = foo ; 
		rapi_buffer_read_uint32 (context->recv_buffer, &foo ) ; 
		if (lpcbMaxValueNameLen)
			*lpcbMaxValueNameLen = foo ; 
		rapi_buffer_read_uint32 (context->recv_buffer, &foo ) ; 
		if (lpcbMaxValueLen)
			*lpcbMaxValueLen = foo ; 
	}

	return return_value;
}


LONG _CeRegEnumValue2(
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

	rapi_context_begin_command(context, 0x34);
	//These two values are NOT allowed to be NULL, therefore, just send them
	//right away
	rapi_buffer_write_uint32         (context->send_buffer, hKey);
	rapi_buffer_write_uint32         (context->send_buffer, dwIndex);

        /* a NULL lpszValueName is an error */
        if (!lpszValueName)
                return ERROR_INVALID_PARAMETER;

        /* The value name buffer is not optional, if we are given zero as the size let the device flag the error */
	rapi_buffer_write_uint32(context->send_buffer, lpcbValueName ? *lpcbValueName : 0);

	//Furthermore, DON'T use the write_optional, for some reason 
	//that does not work, at writes too many things to the send buffer

        if (lpData && (!lpcbData))
                return ERROR_INVALID_PARAMETER;

	rapi_buffer_write_uint32(context->send_buffer, lpcbData ? *lpcbData : 0 );

	if ( !rapi2_context_call(context) )
		return false;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

	//
	//The device reports |lenght|string
	//

	if (ERROR_SUCCESS == return_value)
	{
		DWORD tmp_dword;
                DWORD data_buf_size = 0;

                /* read the valuename */
                rapi_buffer_read_string(context->recv_buffer, lpszValueName, lpcbValueName ) ; 

		//Then read the type, this is always returned by the device
		//Just skip the value if it is not needed.
                tmp_dword = 0;
                rapi_buffer_read_uint32(context->recv_buffer, &tmp_dword);
                if (lpType)
                        *lpType = tmp_dword;

		//Then read the data if needed
                rapi_buffer_read_uint32(context->recv_buffer, &tmp_dword);
                if (lpcbData) {
                        data_buf_size = *lpcbData;
			*lpcbData = tmp_dword;
                }

                if (lpData) {
                        rapi_buffer_read_data(context->recv_buffer, lpData, *lpcbData > data_buf_size ? data_buf_size : *lpcbData);
                }
	}

	return return_value;
}


LONG _CeRegEnumKeyEx2(
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

        if (lpClass && (!lpcbClass))
                return ERROR_INVALID_PARAMETER;

	rapi_context_begin_command(context, 0x30);
	rapi_buffer_write_uint32         (context->send_buffer, hKey);
	rapi_buffer_write_uint32         (context->send_buffer, dwIndex);

        /* lpcbName is not optional, but let the device catch the problem */
        rapi_buffer_write_uint32(context->send_buffer, lpcbName ? *lpcbName : 0);
	rapi_buffer_write_uint32(context->send_buffer, lpcbClass ? *lpcbClass : 0 );
	rapi_buffer_write_uint32(context->send_buffer, 0);


	if ( !rapi2_context_call(context) )
		return ERROR_GEN_FAILURE;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_int32(context->recv_buffer, &return_value);

	if (ERROR_SUCCESS == return_value)
	{
		if (lpName)
		{
			rapi_buffer_read_string(context->recv_buffer, lpName, lpcbName );
		}
		if (lpClass)
		{
			rapi_buffer_read_string(context->recv_buffer, lpClass, lpcbClass );
		}
	}

	return return_value;
}

LONG _CeRegSetValueEx2(
		RapiContext *context,
		HKEY hKey,
		LPCWSTR lpValueName,
		DWORD Reserved,
		DWORD dwType,
		const BYTE *lpData,
		DWORD cbData)
{
	LONG return_value = ERROR_GEN_FAILURE;

	rapi_context_begin_command(context, 0x38);
	rapi_buffer_write_uint32(context->send_buffer, hKey);
	rapi2_buffer_write_string(context->send_buffer, lpValueName);
	rapi_buffer_write_uint32(context->send_buffer, dwType);
        rapi_buffer_write_uint32(context->send_buffer, cbData);
        rapi_buffer_write_data(context->send_buffer, lpData, cbData);

	if ( !rapi2_context_call(context) )
		return ERROR_GEN_FAILURE;

	if (!rapi_buffer_read_uint32(context->recv_buffer, &context->last_error))
		return ERROR_GEN_FAILURE;
	if (!rapi_buffer_read_int32(context->recv_buffer, &return_value))
		return ERROR_GEN_FAILURE;

	return return_value;
}

