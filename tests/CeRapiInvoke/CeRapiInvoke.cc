#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <stdint.h>
#include <string.h>

#include "rapi.h"
#include "pcommon.h"

using namespace std;
using namespace synce;

#define ANYFILE_BUFFER_SIZE (16*1024)

// copied from pcp.c
static bool anyfile_copy(const char* source_ascii, const char* dest_ascii, const char* name, size_t* bytes_copied)/*{{{*/
{
	bool success = false;
	size_t bytes_read;
	size_t bytes_written;
	uint8_t* buffer = NULL;
	AnyFile* source = NULL;
	AnyFile* dest   = NULL;

	if (!(buffer = (uint8_t*)malloc(ANYFILE_BUFFER_SIZE)))
	{
		fprintf(stderr, "%s: Failed to allocate buffer of size %i\n", name, ANYFILE_BUFFER_SIZE);
		goto exit;
	}

	if (!(source = anyfile_open(source_ascii, READ)))
	{
		fprintf(stderr, "%s: Failed to open source file '%s'\n", name, source_ascii);
		goto exit;
	}

	if (!(dest = anyfile_open(dest_ascii, WRITE)))
	{
		fprintf(stderr, "%s: Failed to open destination file '%s'\n", name, dest_ascii);
		goto exit;
	}

	for(;;)
	{
		if (!anyfile_read(source, buffer, ANYFILE_BUFFER_SIZE, &bytes_read))
		{
			fprintf(stderr, "%s: Failed to read from source file '%s'\n", name, source_ascii);
			goto exit;
		}

		if (0 == bytes_read)
		{
			/* End of file */
			break;
		}

		if (!anyfile_write(dest, buffer, bytes_read, &bytes_written))
		{
			fprintf(stderr, "%s: Failed to write to destination file '%s'\n", name, dest_ascii);
			goto exit;
		}

		if (bytes_written != bytes_read)
		{
			fprintf(stderr, "%s: Only wrote %zi bytes of %zi to destination file '%s'\n", name, 
					bytes_written, bytes_read, dest_ascii);
			goto exit;
		}

		*bytes_copied += bytes_written;
	}

	success = true;

exit:
	if (buffer)
		free(buffer);
	
	if (source)
	{
		anyfile_close(source);
		free(source);
	}

	if (dest)
	{
		anyfile_close(dest);
		free(dest);
	}

	return success;
}/*}}}*/

#define BASENAME    "invokeme.dll"
#define LOCAL_DLL   "./dll/" BASENAME
#define REMOTE_DLL  "/windows/" BASENAME

static bool test_ping_result()
{
  uint32_t expected = 0x12345678;
  uint32_t le_expected = htole32(expected);
  HRESULT hr = rapi_invoke(
      REMOTE_DLL,
      "PingResult",
      sizeof(le_expected), (BYTE*)&le_expected,
      NULL, NULL,
      NULL,
      0);
  return hr == (HRESULT)expected;
}

#define PING_BUFFER_SIZE  1000

static bool test_ping_buffer()
{
  BYTE buffer[PING_BUFFER_SIZE];
  DWORD output_size;
  BYTE* output_buffer;
  
  HRESULT hr = rapi_invoke(
      REMOTE_DLL,
      "PingBuffer",
      PING_BUFFER_SIZE, buffer,
      &output_size, &output_buffer,
      NULL,
      0);
  if (hr != S_OK)
    return false;

  bool success = 
    (PING_BUFFER_SIZE == output_size) &&
    memcmp(buffer, output_buffer, PING_BUFFER_SIZE) == 0;

  CeRapiFreeBuffer(output_buffer);

  return success;
}

static bool test_ping_stream()
{
  bool success = false;

  IRAPIStream* stream = NULL;

  DWORD le_size = htole32(PING_BUFFER_SIZE);
  HRESULT hr = rapi_invoke(
      REMOTE_DLL,
      "PingStream",
      sizeof(le_size), (BYTE*)&le_size,
      0, NULL,
      &stream,
      0);
  if (hr != S_OK)
    return false;

  cerr << "a ";
  
  ULONG count;

  BYTE input_buffer[PING_BUFFER_SIZE];
  hr = IRAPIStream_Write(stream, input_buffer, PING_BUFFER_SIZE, &count);
  if (FAILED(hr) || count != PING_BUFFER_SIZE)
  {
    cerr << "IRAPIStream_Write failed" << endl;
    goto exit;
  }
  cerr << "b ";

  BYTE output_buffer[PING_BUFFER_SIZE];
  hr = IRAPIStream_Read(stream, output_buffer, PING_BUFFER_SIZE, &count);
  if (FAILED(hr) || count != PING_BUFFER_SIZE)
  {
    cerr << "IRAPIStream_Read failed" << endl;
    goto exit;
  }
  cerr << "c ";

  success = 
    memcmp(input_buffer, output_buffer, PING_BUFFER_SIZE) == 0;

exit:
  IRAPIStream_Release(stream);
  return success;
}

static bool test_last_error()
{
  DWORD last_error = ERROR_INVALID_PARAMETER;
  HRESULT hr;
  hr = rapi_invoke(
      REMOTE_DLL,
      "TestLastError",
      sizeof(last_error), (BYTE*)&last_error,
      NULL, NULL,
      NULL,
      0);
  if (CeRapiGetLastError() != last_error)
  {
    cerr << "LastError: expected=0x" << hex << last_error <<
      " recevied=0x" << hex << CeRapiGetLastError() << endl;
    return false;
  }
  return true;
}


int main()
{
  int error_count = 0;
  WCHAR* remote_dll = wstr_from_ascii(REMOTE_DLL);

  HRESULT hr = CeRapiInit();
  if (FAILED(hr))
  {
    cerr << "CeRapiInit failed: " << synce_strerror(hr) << endl;
    ++error_count;
    goto exit;
  }

  size_t bytes_copied;
  if (!anyfile_copy(
        LOCAL_DLL,
        ":" REMOTE_DLL,
        BASENAME,
        &bytes_copied))
  {
    cerr << "Failed to copy DLL to device:" << synce_strerror(hr) << endl;
    ++error_count;
    goto exit;
  }

  cout << "Testing call to non-existent function... ";
  hr = rapi_invoke(
      REMOTE_DLL,
      "ThisMethodShouldNotExist",
      0, NULL,
      NULL, NULL,
      NULL,
      0);
  if (SUCCEEDED(hr))
  {
    cerr << "CeRapiInvoke succeeded to call non-existant method?!" << endl;
    ++error_count;
  } else
    cout << "ok" << endl;

  cout << "Testing setting error code... ";
  if (!test_last_error())
  {
    cerr << "TestLastError failed" << endl;
    ++error_count;
  } else
    cout << "ok" << endl;

  cout << "Testing setting return code... ";
  if (!test_ping_result())
  {
    cerr << "PingResult failed" << endl;
    ++error_count;
  } else
    cout << "ok" << endl;

  cout << "Testing \"ping\" by buffer... ";
  if (!test_ping_buffer())
  {
    cerr << "PingBuffer failed" << endl;
    ++error_count;
  }
    cout << "ok" << endl;

  cout << "Testing \"ping\" by stream... ";
  if (!test_ping_stream())
  {
    cerr << "PingStream failed" << endl;
    ++error_count;
  }
    cout << "ok" << endl;

exit:
#if 1
  if (!CeDeleteFile(remote_dll))
  {
    cerr << "CeDeleteFile failed." << endl;
    ++error_count;
  }
#endif

  CeRapiUninit();
  wstr_free_string(remote_dll);
  return error_count;
}

