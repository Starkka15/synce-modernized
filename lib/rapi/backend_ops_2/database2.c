#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "backend_ops_2.h"
#include "rapi_context.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define RAPI_DATABASE_DEBUG 0

#if RAPI_DATABASE_DEBUG
#define rapi_database_trace(args...)    synce_trace(args)
#define rapi_database_trace_wstr(args...)    synce_trace_wstr(args)
#define rapi_database_warning(args...)  synce_warning(args)
#define rapi_database_error(args...)    synce_error(args)
#else
#define rapi_database_trace(args...)
#define rapi_database_trace_wstr(args...)
#define rapi_database_warning(args...)
#define rapi_database_error(args...)
#endif


CEOID _NotImplementedCeCreateDatabase2(
		RapiContext *context,
		LPWSTR lpszName,
		DWORD dwDbaseType,
		WORD wNumSortOrder,
		SORTORDERSPEC *rgSortSpecs)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return 0;
}

BOOL _NotImplementedCeDeleteDatabase2(/*{{{*/
		RapiContext *context,
		CEOID oid)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return 0;
}/*}}}*/

BOOL _NotImplementedCeFindAllDatabases2(/*{{{*/
		RapiContext *context,
		DWORD dwDbaseType,
		WORD wFlags,
		LPWORD cFindData,
		LPLPCEDB_FIND_DATA ppFindData)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return 0;
}/*}}}*/

HANDLE _NotImplementedCeFindFirstDatabase2(/*{{{*/
		RapiContext *context,
		DWORD dwDbaseType)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return INVALID_HANDLE_VALUE;
}/*}}}*/

CEOID _NotImplementedCeFindNextDatabase2(/*{{{*/
		RapiContext *context,
		HANDLE hEnum)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return 0;
}/*}}}*/

HANDLE _NotImplementedCeOpenDatabase2(/*{{{*/
		RapiContext *context,
		PCEOID poid,
		LPWSTR lpszName,
		CEPROPID propid,
		DWORD dwFlags,
		HWND hwndNotify)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return INVALID_HANDLE_VALUE;
}/*}}}*/


CEOID _NotImplementedCeReadRecordProps2(/*{{{*/
		RapiContext *context,
		HANDLE hDbase,
		DWORD dwFlags,
		LPWORD lpcPropID,
		CEPROPID *rgPropID,
		LPBYTE *lplpBuffer,
		LPDWORD lpcbBuffer)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return 0;
}

CEOID _NotImplementedCeWriteRecordProps2(
		RapiContext *context,
		HANDLE hDbase, CEOID oidRecord, WORD cPropID, CEPROPVAL* rgPropVal)/*{{{*/
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return 0;
}


CEOID _NotImplementedCeSeekDatabase2(/*{{{*/
		RapiContext *context,
		HANDLE hDatabase,
		DWORD dwSeekType,
		DWORD dwValue,
		LPDWORD lpdwIndex)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return 0;
}/*}}}*/

BOOL _NotImplementedCeDeleteRecord2(/*{{{*/
    RapiContext *context,
    HANDLE hDatabase,
    CEOID oidRecord)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return 0;
}/*}}}*/

BOOL _NotImplementedCeSetDatabaseInfo2(
    RapiContext *context,
    CEOID oidDbase,
    CEDBASEINFO* pNewInfo)
{
  context->rapi_error = E_NOTIMPL;
  context->last_error = ERROR_SUCCESS;
  return 0;
}

