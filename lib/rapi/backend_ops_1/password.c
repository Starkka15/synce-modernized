/* $Id: password.c 3807 2009-07-19 12:11:08Z mark_ellis $ */
#undef __STRICT_ANSI__
#define _GNU_SOURCE
#include "rapi_internal.h"
#include "backend_ops_1.h"
#include "rapi_buffer.h"
#include "rapi_context.h"
#include <stdlib.h>


BOOL _CeCheckPassword( /*{{{*/
                RapiContext *context,
		LPWSTR lpszPassword)
{
	BOOL return_value = 0;

	rapi_context_begin_command(context, 0x34);
	rapi_buffer_write_optional_string(context->send_buffer, lpszPassword);

	if ( !rapi_context_call(context) )
		return false;

	rapi_buffer_read_uint32(context->recv_buffer, &context->last_error);
	rapi_buffer_read_uint32(context->recv_buffer, &return_value);

	return return_value;
}/*}}}*/

