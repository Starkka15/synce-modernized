/* $Id: misc.c 4141 2012-11-23 14:09:17Z mark_ellis $ */
#undef __STRICT_ANSI__
#define _GNU_SOURCE
#include "rapi_context.h"
#include "rapi.h"
#include <stdlib.h>
#include <synce_log.h>

/*
 * non-standard rapi-calls valid for PocketPC 2002/2003 and Windows Mobile 5
 */

bool rapi_reg_create_key(/*{{{*/
        HKEY parent, const char* name, HKEY* key)
{
    WCHAR* name_wide = wstr_from_current(name);
    if (!name_wide)
            return FALSE;

    LONG result = CeRegCreateKeyEx(
            parent,
    name_wide,
    0,
    NULL,
    0,
    0,
    NULL,
    key,
    NULL
                                  );

    wstr_free_string(name_wide);

    return ERROR_SUCCESS == result;
}/*}}}*/

bool rapi_reg_open_key(/*{{{*/
        HKEY parent, const char* name, HKEY* key)
{
    WCHAR* name_wide = wstr_from_current(name);
    if (!name_wide)
            return FALSE;

    LONG result = CeRegOpenKeyEx(
            parent,
    name_wide,
    0,
    0,
    key
                                );

    wstr_free_string(name_wide);

    return ERROR_SUCCESS == result;
}/*}}}*/

bool rapi_reg_query_dword(/*{{{*/
        HKEY key, const char* name, DWORD* value)
{
    DWORD type;
    DWORD size = sizeof(DWORD);
    WCHAR* name_wide = wstr_from_current(name);
    if (!name_wide)
            return FALSE;

    LONG result = CeRegQueryValueEx(
            key,
    name_wide,
    NULL,
    &type,
    (LPBYTE)value,
    &size);

    wstr_free_string(name_wide);

    return
            ERROR_SUCCESS == result &&
            REG_DWORD == type &&
            sizeof(DWORD) == size;
}/*}}}*/

bool rapi_reg_query_string(/*{{{*/
        HKEY key, const char* name, char** value)
{
    bool success = false;
    DWORD type;
    DWORD size = 0;
    WCHAR* unicode = NULL;
    WCHAR* name_wide = wstr_from_current(name);
    if (!name_wide)
            return FALSE;

    LONG result = CeRegQueryValueEx(
            key,
    name_wide,
    NULL,
    &type,
    NULL,
    &size);

    if (ERROR_SUCCESS == result && REG_SZ == type)
    {
        unicode = calloc(1, size);

        result = CeRegQueryValueEx(
                key,
        name_wide,
        NULL,
        &type,
        (LPBYTE)unicode,
        &size);

    }

    if (ERROR_SUCCESS == result && REG_SZ == type)
    {
        *value = wstr_to_current(unicode);
        success = true;
    }

    free(unicode);
    wstr_free_string(name_wide);

    return success;
}/*}}}*/

bool rapi_reg_set_dword(/*{{{*/
        HKEY key, const char* name, DWORD value)
{
    WCHAR* name_wide = wstr_from_current(name);
    if (!name_wide)
            return FALSE;

    LONG result = CeRegSetValueEx(
            key,
    name_wide,
    0,
    REG_DWORD,
    (BYTE*)&value,
    sizeof(DWORD));

    wstr_free_string(name_wide);

    return ERROR_SUCCESS == result;
}/*}}}*/

bool rapi_reg_set_string(/*{{{*/
        HKEY key, const char* name, const char *value)
{
    WCHAR* name_wide = wstr_from_current(name);
    if (!name_wide)
            return FALSE;
    WCHAR* value_wide = wstr_from_current(value);
    if (!value_wide) {
            wstr_free_string(name_wide);
            return FALSE;
    }
    DWORD size = wstrlen(value_wide);

    LONG result = CeRegSetValueEx(
            key,
    name_wide,
    0,
    REG_SZ,
    (BYTE*)value_wide,
    (size * 2) + 2);

    wstr_free_string(name_wide);
    wstr_free_string(value_wide);

    return ERROR_SUCCESS == result;
}/*}}}*/

/**
 * This function copies an existing file to a new file.
 */
bool rapi_copy_file(
		const char *source_file_name,
		const char *dest_file_name,
		bool fail_if_exists)
{
    BOOL return_value = 0;
    LPWSTR lpExistingFileNameW = NULL;
    LPWSTR lpNewFileNameW = NULL;

    lpExistingFileNameW = wstr_from_current(source_file_name);
    lpNewFileNameW      = wstr_from_current(dest_file_name);

    if (source_file_name && !lpExistingFileNameW)
        goto fail;

    if (dest_file_name && !lpNewFileNameW)
        goto fail;

    return_value = CeCopyFile(lpExistingFileNameW, lpNewFileNameW, fail_if_exists);

fail:
    wstr_free_string(lpExistingFileNameW);
    wstr_free_string(lpNewFileNameW);

    return return_value;
}

/**
 * This function copies an existing file to a new file.
 *
 * Ascii version. This is deprecated, use rapi_copy_file()
 */
BOOL CeCopyFileA(
		LPCSTR lpExistingFileName,
		LPCSTR lpNewFileName,
		BOOL bFailIfExists)
{
    synce_info("This function is deprecated. Use rapi_copy_file() instead.");
    return rapi_copy_file(lpExistingFileName, lpNewFileName, bFailIfExists);
}


/**
 * Ascii / utf8 version of CeRapiInvoke.
 */
HRESULT rapi_invoke( /*{{{*/
    const char *dll_path,
    const char *function_name,
    DWORD cbInput,
    const BYTE *pInput,
    DWORD *pcbOutput,
    BYTE **ppOutput,
    IRAPIStream **ppIRAPIStream,
    DWORD dwReserved)
{
    HRESULT hr;
    WCHAR* wide_dll_path       = wstr_from_current(dll_path);
    WCHAR* wide_function_name  = wstr_from_current(function_name);

    if ((!wide_dll_path) || (!wide_function_name)) {
            wstr_free_string(wide_dll_path);
            wstr_free_string(wide_function_name);
            return E_INVALIDARG;
    }

    hr = CeRapiInvoke( wide_dll_path, wide_function_name, cbInput, pInput,
        pcbOutput, ppOutput, ppIRAPIStream, dwReserved);

    wstr_free_string(wide_dll_path);
    wstr_free_string(wide_function_name);

    return hr;
}

/**
 * Ascii / utf8 version of CeRapiInvoke.
 * This is deprecated, use rapi_invoke()
 */
HRESULT CeRapiInvokeA( /*{{{*/
    LPCSTR pDllPath,
    LPCSTR pFunctionName,
    DWORD cbInput,
    const BYTE *pInput,
    DWORD *pcbOutput,
    BYTE **ppOutput,
    IRAPIStream **ppIRAPIStream,
    DWORD dwReserved)
{
    synce_info("This function is deprecated. Use rapi_invoke() instead.");
    return rapi_invoke( pDllPath, pFunctionName, cbInput, pInput,
                        pcbOutput, ppOutput, ppIRAPIStream, dwReserved);
}
