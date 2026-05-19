/* $Id: invoke.c 4124 2012-11-20 16:23:17Z mark_ellis $ */
#include "backend_ops_1.h"
#include "rapi_context.h"
#include "irapistream.h"
#include "irapistream_internal.h"
#include <assert.h>
#include <stdlib.h>
#include <sys/socket.h>

static HRESULT CeRapiInvokeCommon(
    RapiContext* context,
    LPCWSTR pDllPath,
    LPCWSTR pFunctionName,
    DWORD cbInput,
    const BYTE *pInput,
    DWORD dwReserved,
    BOOL inRapiStream
    )
{
  if (cbInput)
    if (!pInput)
      return E_INVALIDARG;

  rapi_context_begin_command(context, 0x45);
  rapi_buffer_write_uint32(context->send_buffer, dwReserved);
  rapi_buffer_write_string(context->send_buffer, pDllPath);
  rapi_buffer_write_string(context->send_buffer, pFunctionName);
  rapi_buffer_write_uint32(context->send_buffer, cbInput);
  if (cbInput)
    rapi_buffer_write_data  (context->send_buffer, pInput, cbInput);
  rapi_buffer_write_uint32(context->send_buffer, inRapiStream);

  return S_OK;
}

static HRESULT CeRapiInvokeStream( /*{{{*/
    RapiContext *source_context,
    LPCWSTR pDllPath,
    LPCWSTR pFunctionName,
    DWORD cbInput,
    const BYTE *pInput,
    DWORD *pcbOutput,
    BYTE **ppOutput,
    IRAPIStream **ppIRAPIStream,
    DWORD dwReserved)
{
  HRESULT return_value = E_FAIL;
  RapiContext* context = NULL;

  assert(ppIRAPIStream);

  *ppIRAPIStream = rapi_stream_new();
  context = (**ppIRAPIStream).context;

  return_value = rapi_context_connect(context);
  if (FAILED(return_value))
  {
    synce_error("rapi_context_connect failed");
    goto exit;
  }

  return_value = CeRapiInvokeCommon(
      context,
      pDllPath,
      pFunctionName,
      cbInput,
      pInput,
      dwReserved,
      TRUE);
  if (FAILED(return_value))
  {
    synce_error("CeRapiInvokeCommon failed");
    goto exit;
  }

  if ( !rapi_buffer_send(context->send_buffer, context->socket) )
  {
    synce_error("synce_socket_send failed");
    return E_FAIL;
  }

  return_value = IRAPIStream_Read(*ppIRAPIStream, &context->last_error, sizeof(DWORD), NULL);
  if (FAILED(return_value))
  {
    synce_error("IRAPIStream_Read failed");
    goto exit;
  }

  if (context->last_error)
  {
    return_value = E_FAIL;
    goto exit;
  }

  return_value = S_OK;

exit:
  if (FAILED(return_value))
  {
    rapi_stream_destroy(*ppIRAPIStream);
    *ppIRAPIStream = NULL;
  }
  return return_value;
}/*}}}*/

static HRESULT CeRapiInvokeBuffers(
    RapiContext *context,
    LPCWSTR pDllPath,
    LPCWSTR pFunctionName,
    DWORD cbInput,
    const BYTE *pInput,
    DWORD *pcbOutput,
    BYTE **ppOutput,
    DWORD dwReserved)
{
  HRESULT return_value = E_UNEXPECTED;
  HRESULT hr;
  uint32_t unknown;
  size_t bytes_left;
  unsigned output_size;
  uint32_t last_error;

  hr = CeRapiInvokeCommon(
      context,
      pDllPath,
      pFunctionName,
      cbInput,
      pInput,
      dwReserved,
      FALSE);
  if (FAILED(hr))
  {
    synce_error("CeRapiInvokeCommon failed");
    goto exit;
  }

  if ( !rapi_buffer_send(context->send_buffer, context->socket) )
  {
    synce_error("synce_socket_send failed");
    hr = E_FAIL;
    goto exit;
  }

  if ( !rapi_buffer_recv(context->recv_buffer, context->socket) )
  {
    synce_error("rapi_buffer_recv failed");
    hr = E_FAIL;
    goto exit;
  }

  bytes_left = rapi_buffer_get_size(context->recv_buffer);
  /*synce_trace("buffer 1 size: 0x%08x", bytes_left);*/

  do
  {
    /* unknown 1 */
    if (!rapi_buffer_read_uint32(context->recv_buffer, &unknown))
    {
      synce_error("Failed to read");
      break;
    }
    synce_trace("unknown: 0x%08x", unknown);
    bytes_left -= 4;

    if (!bytes_left)
      break;

    /* last error */
    if (!rapi_buffer_read_uint32(context->recv_buffer, &last_error))
    {
      synce_error("Failed to read");
      break;
    }
    context->last_error = last_error;
    synce_trace("last_error: 0x%08x", last_error);
    bytes_left -= 4;

    if (!bytes_left)
      break;

    /* result */
    if (!rapi_buffer_read_uint32(context->recv_buffer, (uint32_t*)&return_value))
    {
      synce_error("Failed to read return value");
      break;
    }
    synce_trace("return value: 0x%08x", return_value);
    bytes_left -= 4;

    if (!bytes_left)
      break;

    /* output size */
    if (!rapi_buffer_read_uint32(context->recv_buffer, &output_size))
    {
      synce_error("Failed to read output size");
      break;
    }
    /*synce_trace("Output size: 0x%08x", output_size);*/
    bytes_left -= 4;

    if (pcbOutput)
      *pcbOutput = output_size;

    if (ppOutput)
    {
      *ppOutput = malloc(output_size);
      if (!*ppOutput)
      {
        hr = E_OUTOFMEMORY;
        break;
      }

      if (!rapi_buffer_read_data(context->recv_buffer, *ppOutput, output_size))
      {
        synce_error("Failed to read output data");
        hr = E_FAIL;
        break;
      }
    }

  } while (0);

  if ( !rapi_buffer_recv(context->recv_buffer, context->socket) )
  {
    synce_error("rapi_buffer_recv failed");
    hr = E_FAIL;
    goto exit;
  }

  /*synce_trace("buffer 2 size: 0x%08x",
      rapi_buffer_get_size(context->recv_buffer));*/

exit:
  if (SUCCEEDED(hr))
    return return_value;
  else
    return hr;
}

HRESULT _CeRapiInvoke( /*{{{*/
    RapiContext *context,
    LPCWSTR pDllPath,
    LPCWSTR pFunctionName,
    DWORD cbInput,
    const BYTE *pInput,
    DWORD *pcbOutput,
    BYTE **ppOutput,
    IRAPIStream **ppIRAPIStream,
    DWORD dwReserved)
{
  if (ppIRAPIStream)
    return CeRapiInvokeStream(context, pDllPath, pFunctionName, cbInput, pInput,
        pcbOutput, ppOutput, ppIRAPIStream, dwReserved);
  else
    return CeRapiInvokeBuffers(context, pDllPath, pFunctionName, cbInput, pInput,
        pcbOutput, ppOutput, dwReserved);
}/*}}}*/

