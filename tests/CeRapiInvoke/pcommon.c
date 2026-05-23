/* $Id: pcommon.c 4138 2012-11-22 15:22:46Z mark_ellis $ */
#include "pcommon.h"
#include "rapi.h"
#include <synce_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define WIDE_BACKSLASH   htole16('\\')

void convert_to_backward_slashes(char* path)
{
	while (*path)
	{
		if ('/' == *path)
			*path = '\\';

		path++;
	}
}

/**
 * See if a filename is remote
 */
bool is_remote_file(const char* filename)
{
	return filename && ':' == filename[0];
}


WCHAR* adjust_remote_path(WCHAR* old_path, bool free_path)
{
	WCHAR wide_backslash[2];
	WCHAR path[MAX_PATH];

	wide_backslash[0] = WIDE_BACKSLASH;
	wide_backslash[1] = '\0';

	/* Nothing to adjust if we have an absolute path */
	if (WIDE_BACKSLASH == old_path[0])
		return old_path;

	if (!CeGetSpecialFolderPath(CSIDL_PERSONAL, sizeof(path), path))
	{
		fprintf(stderr, "Unable to get the \"My Documents\" path.\n");
		return NULL;
	}

	wstr_append(path, wide_backslash, sizeof(path));
	wstr_append(path, old_path, sizeof(path));

	if (free_path)
		wstr_free_string(old_path);

	synce_trace_wstr(path);
	return wstrdup(path);
}


typedef void (*ANYFILE_CLOSE)(AnyFile* file);
typedef bool (*ANYFILE_ACCESSOR)(AnyFile* file, unsigned char* buffer, size_t bytes, size_t* bytesAccessed);

struct _AnyFile
{
	union
	{
		HANDLE remote;
		FILE*  local;
	} handle;
	
	ANYFILE_ACCESSOR read;
	ANYFILE_ACCESSOR write;
	ANYFILE_CLOSE close;
};

static void anyfile_remote_close(AnyFile* file)
{
	HRESULT hr;
	DWORD last_error;

	if (!(CeCloseHandle(file->handle.remote))) {
	  if (FAILED(hr = CeRapiGetError())) {
	    synce_error("Error closing remote file: %08x: %s",
			hr, synce_strerror(hr));
	  } else {
	    last_error = CeGetLastError();
	    synce_error("Error closing remote file: %d: %s",
			last_error, synce_strerror(last_error));
	  }
	}
}

static bool anyfile_remote_read(AnyFile* file, unsigned char* buffer, size_t bytes, size_t* bytesAccessed)
{
	HRESULT hr;
	DWORD last_error;

	BOOL result;
	DWORD lpNumberOfBytesRead = 0;
	result = CeReadFile(file->handle.remote, buffer, bytes, &lpNumberOfBytesRead, NULL);
	*bytesAccessed = lpNumberOfBytesRead;

	if (!result) {
	  if (FAILED(hr = CeRapiGetError())) {
	    synce_error("Failed to read from remote file: %08x: %s",
			hr, synce_strerror(hr));
	  } else {
	    last_error = CeGetLastError();
	    synce_error("Failed to read from remote file: %d: %s",
			last_error, synce_strerror(last_error));
	  }
	}

	return result;
}

static bool anyfile_remote_write(AnyFile* file, unsigned char* buffer, size_t bytes, size_t* bytesAccessed)
{
	HRESULT hr;
	DWORD last_error;

	BOOL result;
	DWORD lpNumberOfBytesWritten = 0;
	result = CeWriteFile(file->handle.remote, buffer, bytes, &lpNumberOfBytesWritten, NULL);
	*bytesAccessed = lpNumberOfBytesWritten;

	if (!result) {
	  if (FAILED(hr = CeRapiGetError())) {
	    synce_error("Failed to write to remote file: %08x: %s",
			hr, synce_strerror(hr));
	  } else {
	    last_error = CeGetLastError();
	    synce_error("Failed to write to remote file: %d: %s",
			last_error, synce_strerror(last_error));
	  }
	}

	return result;
}

static void anyfile_local_close(AnyFile* file)
{
	if (file->handle.local)
	  if (fclose(file->handle.local) != 0)
	    synce_error("Error closing local file: %d: %s", errno, strerror(errno));

	return;
}

static bool anyfile_local_read(AnyFile* file, unsigned char* buffer, size_t bytes, size_t* bytesAccessed)
{
	*bytesAccessed = fread(buffer, 1, bytes, file->handle.local);
	if (ferror(file->handle.local)) {
	  synce_error("Failed to read from local file: %d: %s", errno, strerror(errno));
	  return false;
	}

	return true;
}

static bool anyfile_local_write(AnyFile* file, unsigned char* buffer, size_t bytes, size_t* bytesAccessed)
{
	*bytesAccessed = fwrite(buffer, 1, bytes, file->handle.local);
	if (ferror(file->handle.local)) {
	  synce_error("Failed to write to local file: %d: %s", errno, strerror(errno));
	  return false;
	}

	return true;
}


/**
 * Open remote file for reading or writing
 */
static AnyFile* anyfile_remote_open(const char* filename, ANYFILE_ACCESS access)
{
	HRESULT hr;
	DWORD last_error;

	WCHAR* wide_filename = wstr_from_current(filename);
        if (!wide_filename) {
                synce_error("Failed to convert filename '%s' from current encoding to UCS2", filename);
                return NULL;
        }

	AnyFile* file = (AnyFile*)calloc(1, sizeof(AnyFile));

	switch (access)
	{
		case READ:  
			file->handle.remote = CeCreateFile(wide_filename, GENERIC_READ, 0, NULL, 
					OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			break;

		case WRITE: 
			file->handle.remote = CeCreateFile(wide_filename, GENERIC_WRITE, 0, NULL,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			break;
	}
	
	if (INVALID_HANDLE_VALUE == file->handle.remote)
	{

	  if (FAILED(hr = CeRapiGetError())) {
	    synce_error("Failed to open file '%s': %08x: %s",
			filename, hr, synce_strerror(hr));
	  } else {
	    last_error = CeGetLastError();
	    synce_error("Failed to open file '%s': %d: %s",
			filename, last_error, synce_strerror(last_error));
	  }

	  free(file);
	  file = NULL;
	}
	else
	{
		file->close = anyfile_remote_close;
		file->write = anyfile_remote_write;
		file->read  = anyfile_remote_read;
	}

	wstr_free_string(wide_filename);
	return file;
}

/**
 * Open local file for reading or writing
 */
static AnyFile* anyfile_local_open(const char* filename, ANYFILE_ACCESS access)
{
	AnyFile* file = (AnyFile*)calloc(1, sizeof(AnyFile));

	switch (access)
	{
		case READ:  
			file->handle.local = fopen(filename, "r");
			break;

		case WRITE: 
			file->handle.local = fopen(filename, "w");
			break;
	}
	
	if (NULL == file->handle.local)
	{
		synce_error("Failed to open file '%s': %d: %s",
			    filename, errno, strerror(errno));
		free(file);
		file = NULL;
	}
	else
	{
		file->close = anyfile_local_close;
		file->write = anyfile_local_write;
		file->read  = anyfile_local_read;
	}

	return file;
}

/**
 * Open file
 */
AnyFile* anyfile_open(const char* filename, ANYFILE_ACCESS access)
{
    char *tmpfilename;
    AnyFile* file = NULL;

    tmpfilename = (char *) strdup(filename);
	if (is_remote_file(tmpfilename))
	{
		convert_to_backward_slashes(tmpfilename);
		file = anyfile_remote_open(tmpfilename + 1, access);
	}
	else
	{
		file = anyfile_local_open(tmpfilename, access);
	}
    free (tmpfilename);

    return file;
}

void anyfile_close(AnyFile* file)
{
	(*file->close)(file);
}

bool anyfile_read(AnyFile* file, unsigned char* buffer, size_t bytes, size_t* bytesAccessed)
{
	return (*file->read)(file, buffer, bytes, bytesAccessed);
}

bool anyfile_write(AnyFile* file, unsigned char* buffer, size_t bytes, size_t* bytesAccessed)
{
	return (*file->write)(file, buffer, bytes, bytesAccessed);
}

