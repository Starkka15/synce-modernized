/* $Id: database.c 4141 2012-11-23 14:09:17Z mark_ellis $ */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "backend_ops_1.h"
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


CEOID _CeCreateDatabase(
                RapiContext *context,
		LPWSTR lpszName,
		DWORD dwDbaseType,
		WORD wNumSortOrder,
		SORTORDERSPEC *rgSortSpecs)
{
	CEOID return_value = 0;
	int i;

	rapi_database_trace("begin");

	rapi_context_begin_command(context, 0x0d);

	/* NOTE: strange marshalling order for parameters */

	rapi_buffer_write_uint32(context->send_buffer, dwDbaseType);
	rapi_buffer_write_uint16(context->send_buffer, wNumSortOrder);

	/* NOTE: We can't write whole rgSortSpecs buffer directly because we need to
	 * think about the byte order */
	for (i = 0; i < wNumSortOrder; i++)
	{
		rapi_buffer_write_uint32(context->send_buffer, rgSortSpecs[i].propid);
		rapi_buffer_write_uint32(context->send_buffer, rgSortSpecs[i].dwFlags);
	}

	rapi_buffer_write_string(context->send_buffer, lpszName);

	if ( !rapi_context_call(context) )
		goto fail;

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
		goto fail;
	rapi_database_trace("context->last_error=0x%08x", context->last_error);

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &return_value) )
		goto fail;

	return return_value;

fail:
	return 0;
}

BOOL _CeDeleteDatabase(/*{{{*/
                RapiContext *context,
		CEOID oid)
{
	BOOL return_value = false;

	rapi_database_trace("begin");

	rapi_context_begin_command(context, 0x0f);
	rapi_buffer_write_uint32(context->send_buffer, oid);

	if ( !rapi_context_call(context) )
		goto exit;

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
		goto exit;
	rapi_database_trace("context->last_error=0x%08x", context->last_error);

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &return_value) )
		goto exit;

exit:
	return return_value;
}/*}}}*/

BOOL _CeFindAllDatabases(/*{{{*/
                RapiContext *context,
		DWORD dwDbaseType,
		WORD wFlags,
		LPWORD cFindData,
		LPLPCEDB_FIND_DATA ppFindData)
{
	uint16_t count;

	rapi_database_trace("begin");

	rapi_context_begin_command(context, 0x2c);

        if ((!cFindData) || (!ppFindData)) {
                context->last_error = ERROR_INVALID_PARAMETER;
                return false;
        }

	rapi_buffer_write_uint32(context->send_buffer, dwDbaseType);
	rapi_buffer_write_uint16(context->send_buffer, wFlags);

	if ( !rapi_context_call(context) )
		return false;

	rapi_buffer_read_uint16(context->recv_buffer, &count);

	rapi_database_trace("found 0x%04x databases", count);

	if (cFindData)
		*cFindData = count;

	if (ppFindData && count > 0)
	{
		LPCEDB_FIND_DATA find_data = calloc(count, sizeof(CEDB_FIND_DATA));

		*ppFindData = find_data;

		if (find_data)
		{
			uint32_t i;
			uint32_t name_size = 0;

			for(i = 0; i < count; i++)
			{
				if (wFlags & FAD_OID)
				{
					if ( !rapi_buffer_read_uint32(context->recv_buffer, &find_data[i].OidDb) )
						goto fail;
					rapi_database_trace("oid=%08x", find_data[i].OidDb);
				}

				if (wFlags & FAD_NAME)
					rapi_buffer_read_uint32(context->recv_buffer, &name_size);

				if (wFlags & FAD_FLAGS)
					rapi_buffer_read_uint32(context->recv_buffer, &find_data[i].DbInfo.dwFlags);

				if (wFlags & FAD_NAME)
				{
					if (name_size)
					{
						rapi_buffer_read_data(context->recv_buffer, find_data[i].DbInfo.szDbaseName, name_size * sizeof(WCHAR) );
						rapi_database_trace_wstr(find_data[i].DbInfo.szDbaseName);
					}
					else
					{
						rapi_database_error("name_size is 0");
						goto fail;
					}
				}

				if (wFlags & FAD_TYPE)
					rapi_buffer_read_uint32(context->recv_buffer, &find_data[i].DbInfo.dwDbaseType);

				if (wFlags & FAD_NUM_RECORDS)
					rapi_buffer_read_uint16(context->recv_buffer, &find_data[i].DbInfo.wNumRecords);

				if (wFlags & FAD_NUM_SORT_ORDER)
					rapi_buffer_read_uint16(context->recv_buffer, &find_data[i].DbInfo.wNumSortOrder);

				if (wFlags & FAD_SIZE)
					rapi_buffer_read_uint32(context->recv_buffer, &find_data[i].DbInfo.dwSize);

				if (wFlags & FAD_LAST_MODIFIED)
				{
					rapi_buffer_read_uint32(context->recv_buffer, &find_data[i].DbInfo.ftLastModified.dwLowDateTime);
					rapi_buffer_read_uint32(context->recv_buffer, &find_data[i].DbInfo.ftLastModified.dwHighDateTime);
				}

				if (wFlags & FAD_SORT_SPECS)
				{
					int j;

					for (j = 0; j < CEDB_MAXSORTORDER; j++)
					{
						rapi_buffer_read_uint32(context->recv_buffer, &find_data[i].DbInfo.rgSortSpecs[j].propid);
						rapi_buffer_read_uint32(context->recv_buffer, &find_data[i].DbInfo.rgSortSpecs[j].dwFlags);
					}
				}
			} /* for each database */
		}
		else
		{
			rapi_database_error("failed to allocate memory");
			goto fail;
		}

		*ppFindData = find_data;
	}

	return true;

fail:
	if (ppFindData && *ppFindData)
		free(ppFindData);
	return false;
}/*}}}*/

HANDLE _CeFindFirstDatabase(/*{{{*/
                RapiContext *context,
		DWORD dwDbaseType)
{
	HANDLE return_value = INVALID_HANDLE_VALUE;

	rapi_database_trace("begin");

	rapi_context_begin_command(context, 0x0a);
	rapi_buffer_write_uint32(context->send_buffer, dwDbaseType);

	if ( !rapi_context_call(context) )
		goto exit;

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
		goto exit;
	rapi_database_trace("context->last_error=0x%08x", context->last_error);

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &return_value) )
		goto exit;

exit:
	return return_value;
}/*}}}*/

CEOID _CeFindNextDatabase(/*{{{*/
                RapiContext *context,
		HANDLE hEnum)
{
	CEOID return_value = 0;

	rapi_database_trace("begin");

	rapi_context_begin_command(context, 0x0b);
	rapi_buffer_write_uint32(context->send_buffer, hEnum);

	if ( !rapi_context_call(context) )
		goto exit;

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
		goto exit;
	rapi_database_trace("context->last_error=0x%08x", context->last_error);

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &return_value) )
		goto exit;

exit:
	return return_value;
}/*}}}*/

HANDLE _CeOpenDatabase(/*{{{*/
                RapiContext *context,
		PCEOID poid,
		LPWSTR lpszName,
		CEPROPID propid,
		DWORD dwFlags,
		HWND hwndNotify)
{
	HANDLE handle = INVALID_HANDLE_VALUE;

	rapi_database_trace("begin");

	rapi_context_begin_command(context, 0x0e);

        if (!poid) {
                context->last_error = ERROR_INVALID_PARAMETER;
                goto exit;
        }

	rapi_buffer_write_uint32(context->send_buffer, *poid);

	rapi_buffer_write_uint32(context->send_buffer, propid);
	rapi_buffer_write_uint32(context->send_buffer, dwFlags);
        if (*poid == 0) {
                rapi_buffer_write_string(context->send_buffer, lpszName);
        }

	if ( !rapi_context_call(context) )
		goto exit;

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
		goto exit;
	rapi_database_trace("context->last_error=0x%08x", context->last_error);

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &handle) )
		goto exit;

        if (*poid == 0) {
                if ( !rapi_buffer_read_uint32(context->recv_buffer, poid) )
                        goto exit;
        }

exit:
	return handle;
}/*}}}*/


CEOID _CeReadRecordProps(/*{{{*/
                RapiContext *context,
		HANDLE hDbase,
		DWORD dwFlags,
		LPWORD lpcPropID,
		CEPROPID *rgPropID,
		LPBYTE *lplpBuffer,
		LPDWORD lpcbBuffer)
{

	/* need to do something with rgPropID */

	CEOID return_value = 0;
	uint16_t prop_id_count = 0;
	uint32_t recv_size = 0;
	size_t out_size = 0;
	uint32_t i;
	unsigned char* recv_buffer = NULL;
	unsigned char* out_buffer = NULL;
	size_t recv_propval_size = 0;
	size_t extra_data_size = 0;
	size_t out_propval_size = 0;

	rapi_database_trace("begin");

	if (! lpcbBuffer ) {
	  return_value = 0;
	  context->last_error = ERROR_INVALID_PARAMETER;
	  goto exit;
	}

	rapi_context_begin_command(context, 0x10);
	rapi_buffer_write_uint32(context->send_buffer, hDbase);
	rapi_buffer_write_uint32(context->send_buffer, dwFlags);
	rapi_buffer_write_uint32(context->send_buffer, 0);
	rapi_buffer_write_uint32(context->send_buffer, 0);
	rapi_buffer_write_uint32(context->send_buffer, 0);
	rapi_buffer_write_uint16(context->send_buffer, 0);

	if ( !rapi_context_call(context) )
		goto fail;

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
		goto fail;
	rapi_database_trace("context->last_error=0x%08x", context->last_error);

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &return_value) )
		goto fail;
	rapi_database_trace("return_value=0x%08x", return_value);

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &recv_size) )
		goto fail;
	rapi_database_trace("received size=%i", recv_size);

	if ( !rapi_buffer_read_uint16(context->recv_buffer, &prop_id_count) )
		goto fail;
	rapi_database_trace("prop_id_count=%i", prop_id_count);

	if (lpcPropID)
		*lpcPropID = prop_id_count;

	/* 
	 *  the data buffer we recieve consists of CEPROPVAL items, followed by
	 *  misc data eg. strings
	 *
	 *  we cannot do a direct cast to a CEPROPVAL array because we cannot guarantee
	 *  that pointer items will be the same size on both platforms
	 */

	/* size of CEPROPVAL over the wire is 16 bytes, ie pointers are 32 bit */
	recv_propval_size = 16 * prop_id_count;
	rapi_database_trace("original propval size = %u", recv_propval_size);

	extra_data_size = recv_size - recv_propval_size;
	rapi_database_trace("extra data size = %u", extra_data_size);

	out_propval_size = sizeof(CEPROPVAL) * prop_id_count;
	rapi_database_trace("new provpval size = %u", out_propval_size);

	out_size = extra_data_size + out_propval_size;
	rapi_database_trace("out buffer size = %u", out_size);

	if (lplpBuffer)
	{
		if (*lplpBuffer != NULL) {
		  if (*lpcbBuffer < out_size) {
		    if  (dwFlags & CEDB_ALLOWREALLOC)
		      *lplpBuffer = realloc(*lplpBuffer, out_size);
		    else
		      {
			context->last_error = ERROR_INSUFFICIENT_BUFFER;
			return_value = 0;
			*lpcbBuffer = out_size;
			goto exit;
		      }
		  }
		} else
		  *lplpBuffer = malloc(out_size);

		if (!(*lplpBuffer)) {
		  context->last_error = ERROR_NOT_ENOUGH_MEMORY;
		  return_value = 0;
		  *lpcbBuffer = out_size;
		  goto exit;
		}

		out_buffer = *lplpBuffer;

		recv_buffer = calloc(1, recv_size);
		if (!recv_buffer) {
		  rapi_database_error("failed to allocate 0x%08x bytes", recv_size);
		  context->last_error = ERROR_NOT_ENOUGH_MEMORY;
		  return_value = 0;
		  *lpcbBuffer = out_size;
		  goto exit;
		}

		CEPROPVAL* propval;
		unsigned char * buf_pos = recv_buffer;

		if ( !rapi_buffer_read_data(context->recv_buffer, recv_buffer, recv_size) )
		  {
		    rapi_database_error("failed to read buffer");
		    goto fail;
		  }

		memcpy(out_buffer + out_propval_size, recv_buffer + recv_propval_size, extra_data_size);

		propval = (CEPROPVAL*)out_buffer;

		rapi_database_trace("buffer = %p", recv_buffer);

		for (i = 0; i < prop_id_count; i++)
		  {

		    propval[i].propid = letoh32(*((uint32_t*)buf_pos));
		    buf_pos = buf_pos + 4;
		    propval[i].wLenData = letoh16(*((uint16_t*)buf_pos));
		    buf_pos = buf_pos + 2;
		    propval[i].wFlags = letoh16(*((uint16_t*)buf_pos));
		    buf_pos = buf_pos + 2;

		    rapi_database_trace("propval[%i].propid = %08x", i, propval[i].propid);


		    switch (propval[i].propid & 0xffff)
		      {

		      case CEVT_BLOB:

			rapi_database_trace("CEVT_BLOB");

			propval[i].val.blob.dwCount = letoh32(*((uint32_t*)buf_pos));
			buf_pos = buf_pos + 4;

			propval[i].val.blob.lpb = letoh32(*((uint32_t*)buf_pos)) - recv_propval_size + (sizeof(CEPROPVAL) * prop_id_count) + out_buffer;
			buf_pos = buf_pos + 4;

			rapi_database_trace("propval[%i].val.blob.dwCount = %08x",
					    i, propval[i].val.blob.dwCount);
			rapi_database_trace("propval[%i].val.blob.lpb = %08x",
					    i, propval[i].val.blob.lpb);

			rapi_database_trace("blob=%s",(char*)propval[i].val.blob.lpb);
			break;

		      case CEVT_LPWSTR:

			rapi_database_trace("CEVT_LPWSTR");

			propval[i].val.lpwstr = (LPWSTR)(letoh32(*((uint32_t*)buf_pos)) - recv_propval_size + (sizeof(CEPROPVAL) * prop_id_count) + out_buffer);
			buf_pos = buf_pos + 8;

			rapi_database_trace("string offset = %p", propval[i].val.lpwstr);
			rapi_database_trace_wstr(propval[i].val.lpwstr);
			rapi_database_trace("propval[i].val.lpwstr = %p", propval[i].val.lpwstr);
			break;

		      case CEVT_I2:
			rapi_database_trace("CEVT_I2");
			propval[i].val.iVal = letoh16(*((int16_t*)buf_pos));
			buf_pos = buf_pos + 8;
			break;
		      case CEVT_I4:
			rapi_database_trace("CEVT_I4");
			propval[i].val.lVal = letoh32(*((int32_t*)buf_pos));
			buf_pos = buf_pos + 8;
			break;
		      case CEVT_R8:

			/* TODO: convert endianness for this, need to set up 64 swap in synce.h */

			rapi_database_trace("CEVT_R8");
			memcpy(&(propval[i].val), buf_pos, 8);
			buf_pos = buf_pos + 8;
			break;
		      case CEVT_BOOL:
			rapi_database_trace("CEVT_BOOL");
			propval[i].val.boolVal = letoh16(*((int16_t*)buf_pos));
			buf_pos = buf_pos + 8;
			break;
		      case CEVT_UI2:
			rapi_database_trace("CEVT_UI2");
			propval[i].val.uiVal = letoh16(*((uint16_t*)buf_pos));
			buf_pos = buf_pos + 8;
			break;
		      case CEVT_UI4:
			rapi_database_trace("CEVT_UI4");
			propval[i].val.ulVal = letoh32(*((uint32_t*)buf_pos));
			buf_pos = buf_pos + 8;
			break;
		      case CEVT_FILETIME:
			rapi_database_trace("CEVT_FILETIME");
			propval[i].val.filetime.dwLowDateTime = letoh32(*((uint32_t*)buf_pos));
			buf_pos += 4;
			propval[i].val.filetime.dwHighDateTime = letoh32(*((uint32_t*)buf_pos));
			buf_pos += 4;
			break;
		      }
		  }

		free(recv_buffer);
	}

	*lpcbBuffer = out_size;

exit:
	return return_value;

fail:
 	rapi_database_error("failed");
	if (recv_buffer)
	  free(recv_buffer);
	return 0;
}/*}}}*/


#define ALIGN(value) ( value = ((value)+3) & ~3 )

static bool PreparePropValForWriting(unsigned* data_offset, void *send_buf, void **send_buf_pos, CEPROPVAL propval)/*{{{*/
{
	bool success = true;

	rapi_database_trace("Preparing value of type %i", propval.propid & 0xffff);

	*((CEPROPID*)*send_buf_pos) = htole32(propval.propid);
	*send_buf_pos += 4;
	*((WORD*)*send_buf_pos) = htole16(propval.wLenData);
	*send_buf_pos += 2;
	*((WORD*)*send_buf_pos) = htole16(propval.wFlags);
	*send_buf_pos += 2;

	switch (propval.propid & 0xffff)
	{
		case CEVT_BLOB:
			*((DWORD*)*send_buf_pos) = htole32(propval.val.blob.dwCount);
			*send_buf_pos += 4;

			/* don't use LPBYTE for propval->val.blob.lpb because it's a 32 bit offset */
			*((DWORD*)*send_buf_pos) = (DWORD)htole32(*data_offset);
			*send_buf_pos += 4;

			memcpy((LPBYTE)(send_buf + *data_offset), propval.val.blob.lpb, propval.val.blob.dwCount);

			*data_offset += propval.val.blob.dwCount;
			break;

		case CEVT_BOOL:
			*((uint32_t*)*send_buf_pos) = htole32(propval.val.boolVal);
			*send_buf_pos += 8;
			break;

		case CEVT_FILETIME:
			*((uint32_t*)*send_buf_pos) = htole32(propval.val.filetime.dwLowDateTime);
			*send_buf_pos += 4;
			*((uint32_t*)*send_buf_pos) = htole32(propval.val.filetime.dwHighDateTime);
			*send_buf_pos += 4;
			break;

		case CEVT_I2:
			*((int16_t*)*send_buf_pos) = htole16(propval.val.iVal);
			*send_buf_pos += 8;
			break;

		case CEVT_I4:
			*((int32_t*)*send_buf_pos) = htole32(propval.val.lVal);
			*send_buf_pos += 8;
			break;

		case CEVT_LPWSTR:
			if (propval.val.lpwstr)
			  {
				size_t size = sizeof(WCHAR) * (wstr_strlen(propval.val.lpwstr) + 1);
				rapi_database_trace_wstr(propval.val.lpwstr);

				*((DWORD*)*send_buf_pos) = (DWORD)htole32(*data_offset);

				rapi_database_trace("String offset: %u", *((DWORD*)*send_buf_pos));

				memcpy((LPBYTE)send_buf + *data_offset, propval.val.lpwstr, size);

				*data_offset += size;
				*send_buf_pos += 8;
			  }
			else
			  {
				rapi_database_error("String property value is NULL");
				success = false;
			  }
			break;

		case CEVT_UI2:
			*((uint16_t*)*send_buf_pos) = htole16(propval.val.uiVal);
			*send_buf_pos += 8;
			break;

		case CEVT_UI4:
			*((uint32_t*)*send_buf_pos) = htole32(propval.val.ulVal);
			*send_buf_pos += 8;
			break;

		case CEVT_R8:

			/* TODO: convert endianness for this, need to set up 64 swap in synce.h */

			memcpy(*send_buf_pos, &(propval.val), 8);
			*send_buf_pos += 8;
			break;

		default:
			rapi_database_error("Don't know how to prepare value type 0x%04x", propval.propid & 0xffff);
			success = false;
			break;
	}

	return success;
}/*}}}*/

CEOID _CeWriteRecordProps(
                RapiContext *context,
                HANDLE hDbase,
                CEOID oidRecord,
                WORD cPropID,
                CEPROPVAL* rgPropVal)/*{{{*/
{
	CEOID return_value = 0;

	unsigned data_offset = 0;
	void * send_buf = NULL;
	void * send_buf_pos = NULL;
	unsigned local_array_size = 0;
	unsigned local_total_size = 0;
	unsigned send_array_size = 0;
	unsigned send_total_size = 0;
	unsigned data_size = 0;
	unsigned i;

	/*
	 *	Format of the CeWriteRecordProps packet - primitives are encoded in the CEPROPVAL structures, lpwstr and blob properties are
	 *	attached to the end of the buffer and referenced by offset pointers
	 *
	 *		long hDBase | long oidRecord | long cPropID | long datalen (of following data) | n * CEPROPVAL | char[] data
	 *
	 */

	rapi_database_trace("begin");
	rapi_context_begin_command(context, 0x11);
	rapi_buffer_write_uint32(context->send_buffer, hDbase);
	rapi_buffer_write_uint32(context->send_buffer, oidRecord);
	rapi_buffer_write_uint16(context->send_buffer, cPropID);

	/*
	 * we have to go through the rgPropVals array two times:
	 *		1. to determine the size of the whole buffer, including data
	 *		2. to write out the CEPROPVAL array followed by the data segment
	 */

	/* 
	 * 1. calculate the length of the whole buffer, including the data segment at the end
	 */

	for ( i = 0; i < cPropID; i++ )
	{
		switch ( ( rgPropVal[i].propid ) & 0xFFFF )
		{
			case CEVT_BLOB:
				data_size += rgPropVal[i].val.blob.dwCount;
				break;
			case CEVT_LPWSTR:
				data_size += sizeof(WCHAR) * ( wstr_strlen( rgPropVal[i].val.lpwstr ) + 1 );
				break;

			default:
				break;
		}

		ALIGN(data_size);
	}

	local_total_size = local_array_size = cPropID * sizeof(CEPROPVAL); /* length of all cepropvals */
	send_total_size = send_array_size = cPropID * 16;

	local_total_size += data_size;
	send_total_size += data_size;

	rapi_database_trace("Array size = %i", local_array_size);
	rapi_database_trace("Total size = %i", local_total_size);
	rapi_database_trace("Send array size = %i", send_array_size);
	rapi_database_trace("Send total size = %i", send_total_size);

	rapi_buffer_write_uint32(context->send_buffer, send_total_size);

	/*
	 * 2. write out the CEPROPVAL array
	 *
	 * make a copy of the buffer, converting endianness, converting pointers to 32 bit offsets, and
	 * appending extra data to the array data
	 */

	send_buf = calloc(1, send_total_size);

	for (i = 0, data_offset = send_array_size, send_buf_pos = send_buf; i < cPropID; i++)
	{
		if (!PreparePropValForWriting(&data_offset, send_buf, &send_buf_pos, rgPropVal[i]))
		{
			rapi_database_error("PreparePropValForWriting failed");
			goto exit;
		}

		ALIGN(data_offset);
	}

	if (data_offset != send_total_size)
	{
		rapi_database_error("Data offset is %08x but should be %08x", data_offset, send_total_size);
		goto exit;
	}

	if (!rapi_buffer_write_data(context->send_buffer, send_buf, send_total_size))
	{
		rapi_database_error("rapi_buffer_write_data failed");
		goto exit;
	}

	if ( !rapi_context_call(context) )
	{
		rapi_database_error("rapi_context_call failed");
		goto exit;
	}

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
		goto exit;
	rapi_database_trace("context->last_error=0x%08x", context->last_error);

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &return_value) )
	{
		rapi_database_trace("failed to read return value");
		return_value = 0;
		goto exit;
	}
	rapi_database_trace("return_value=0x%08x", return_value);

exit:
	free(send_buf);
	send_buf = NULL;

	return return_value;
}/*}}}*/


CEOID _CeSeekDatabase(/*{{{*/
                RapiContext *context,
		HANDLE hDatabase,
		DWORD dwSeekType,
		DWORD dwValue,
		LPDWORD lpdwIndex)
{
	CEOID return_value = 0;

	rapi_database_trace("begin");

	if (!lpdwIndex)
	{
		rapi_database_error("lpdwIndex is NULL");
		goto fail;
	}

	rapi_context_begin_command(context, 0x13);
	rapi_buffer_write_uint32(context->send_buffer, hDatabase);
	rapi_buffer_write_uint32(context->send_buffer, dwSeekType);

	switch (dwSeekType)
	{
		case CEDB_SEEK_VALUESMALLER:
		case CEDB_SEEK_VALUEFIRSTEQUAL:
		case CEDB_SEEK_VALUEGREATER:
		case CEDB_SEEK_VALUENEXTEQUAL:
			rapi_database_error("Seek type by value is not yet supported", dwSeekType);
			goto fail;

		default:
			rapi_buffer_write_uint32(context->send_buffer, dwValue);
			break;
	}

	if ( !rapi_context_call(context) )
		goto fail;

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
		goto fail;
	rapi_database_trace("context->last_error=0x%08x", context->last_error);

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &return_value) )
		goto fail;

	if ( !rapi_buffer_read_uint32(context->recv_buffer, lpdwIndex) )
		goto fail;

	return return_value;

fail:
	return 0;
}/*}}}*/

BOOL _CeDeleteRecord(/*{{{*/
                RapiContext *context,
                HANDLE hDatabase,
                CEOID oidRecord)
{
  BOOL return_value = false;

  rapi_database_trace("begin");

  rapi_context_begin_command(context, 0x12);
	rapi_buffer_write_uint32(context->send_buffer, hDatabase);
  rapi_buffer_write_uint32(context->send_buffer, oidRecord);

  if ( !rapi_context_call(context) )
    goto exit;

  if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
    goto exit;
  rapi_database_trace("context->last_error=0x%08x", context->last_error);

  if ( !rapi_buffer_read_uint32(context->recv_buffer, &return_value) )
    goto exit;

exit:
  return return_value;
}/*}}}*/

BOOL _CeSetDatabaseInfo(
                RapiContext *context,
                CEOID oidDbase,
                CEDBASEINFO* pNewInfo)
{
  BOOL return_value = false;

	rapi_database_trace("begin");

	rapi_context_begin_command(context, 0x14);
	rapi_buffer_write_uint32(context->send_buffer, oidDbase);
  assert(sizeof(CEDBASEINFO) == 0x78);
	rapi_buffer_write_data(context->send_buffer, pNewInfo, sizeof(CEDBASEINFO));

	if ( !rapi_context_call(context) )
		goto exit;

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &context->last_error) )
		goto exit;
	rapi_database_trace("context->last_error=0x%08x", context->last_error);

	if ( !rapi_buffer_read_uint32(context->recv_buffer, &return_value) )
		goto exit;

exit:
	return return_value;
}

