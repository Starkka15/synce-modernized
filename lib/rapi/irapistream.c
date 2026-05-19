#include <stdlib.h>

#include "irapistream.h"
#include "irapistream_internal.h"

IRAPIStream* rapi_stream_new()/*{{{*/
{
  IRAPIStream* stream = calloc(1, sizeof(IRAPIStream));

  if (stream)
  {
    stream->context = rapi_context_new();
  }

  return stream;
}/*}}}*/

void rapi_stream_destroy(IRAPIStream* stream)/*{{{*/
{
  if (stream)
  {
    rapi_context_unref(stream->context);
    free(stream);
  }
}/*}}}*/

ULONG IRAPIStream_Release(IRAPIStream* stream)/*{{{*/
{
  rapi_stream_destroy(stream);
  return 0;
}/*}}}*/

HRESULT IRAPIStream_Read( /*{{{*/
    IRAPIStream* stream,
    void *pv,
    ULONG cb,
    ULONG *pcbRead)
{
  HRESULT hr = E_FAIL;

  if (pv && synce_socket_read(stream->context->socket, pv, cb))
  {
    if (pcbRead)
      *pcbRead = cb;

    hr = S_OK;
  }

  return hr;
}/*}}}*/

HRESULT IRAPIStream_Write( /*{{{*/
    IRAPIStream* stream,
    void const *pv,
    ULONG cb,
    ULONG *pcbWritten)
{
  HRESULT hr = E_FAIL;

  if (pv && synce_socket_write(stream->context->socket, pv, cb))
  {
    if (pcbWritten)
      *pcbWritten = cb;

    hr = S_OK;
  }

  return hr;
}/*}}}*/


/*
int (*IRAPIStream_GetRawSocket)(IRAPIStream* stream);
*/
