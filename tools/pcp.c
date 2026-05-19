/* $Id: pcp.c 4136 2012-11-22 15:10:51Z mark_ellis $ */
#include "pcommon.h"
#include <rapi2.h>
#include <synce_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

char* dev_name = NULL;
bool recursive = false;
char* prog_name = NULL;

static void show_usage(const char* name)
{
	fprintf(stderr,
			"Syntax:\n"
			"\n"
			"\t%s [-r] [-d LEVEL] [-p DEVNAME] [-h] SOURCE DESTINATION\n"
			"\n"
			"\t-r           Copy directories recursively\n"
			"\t-d LEVEL     Set debug log level\n"
			"\t                 0 - No logging (default)\n"
			"\t                 1 - Errors only\n"
			"\t                 2 - Errors and warnings\n"
			"\t                 3 - Everything\n"
			"\t-h           Show this help message\n"
                        "\t-p DEVNAME   Mobile device name\n"
			"\tSOURCE       The source filename, prepended with \":\" for remote files\n"
			"\tDESTINATION  The destination filename, prepended with \":\" for remote files\n",
			name);
}

static bool handle_parameters(int argc, char** argv, char** source, char** dest)
{
	int c;
	int path_count;
	int log_level = SYNCE_LOG_LEVEL_ERROR;
	prog_name = strdup(argv[0]);

	while ((c = getopt(argc, argv, "rd:hp:")) != -1)
	{
		switch (c)
		{
			case 'r':
				recursive = true;
				break;

			case 'd':
				log_level = atoi(optarg);
				break;

                        case 'p':
                                dev_name = optarg;
                                break;
			
			case 'h':
			default:
				show_usage(argv[0]);
				return false;
		}
	}
				
	synce_log_set_level(log_level);

	path_count = argc - optind;
	if (path_count < 1 || path_count > 2)
	{
		fprintf(stderr, "%s: You need to specify source and destination file names on command line\n\n", argv[0]);
		show_usage(argv[0]);
		return false;
	}
		
	*source = strdup(argv[optind++]);
	if (path_count > 1)
		*dest = strdup(argv[optind++]);

	return true;
}

static bool remote_copy(IRAPISession *session, const char* ascii_source, const char* ascii_dest)
{
  HRESULT hr;
  DWORD last_error;
  BOOL result = false;
  LPWSTR lpExistingFileNameW = NULL;
  LPWSTR lpNewFileNameW = NULL;

  lpExistingFileNameW = wstr_from_current(ascii_source+1);
  lpNewFileNameW      = wstr_from_current(ascii_dest+1);

  if (!lpExistingFileNameW || !lpNewFileNameW)
    goto exit;

  result = IRAPISession_CeCopyFile(session, lpExistingFileNameW, lpNewFileNameW, false);

  if (!result) {
    if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
      fprintf(stderr, "%s: failed to copy %s to %s: %s.\n",
	      prog_name, ascii_source, ascii_dest, synce_strerror(hr));
      goto exit;
    }

    last_error = IRAPISession_CeGetLastError(session);
    fprintf(stderr, "%s: failed to copy %s to %s: %s.\n",
	    prog_name, ascii_source, ascii_dest, synce_strerror(last_error));
    goto exit;
  }

 exit:
  wstr_free_string(lpExistingFileNameW);
  wstr_free_string(lpNewFileNameW);
  return result;
}

#define ANYFILE_BUFFER_SIZE (64*1024)

static bool anyfile_copy(IRAPISession *session, const char* source_ascii, const char* dest_ascii, size_t* bytes_copied)
{
	bool success = false;
	size_t bytes_read;
	size_t bytes_written;
	unsigned char* buffer = NULL;
	AnyFile* source = NULL;
	AnyFile* dest   = NULL;

	if (!(buffer = malloc(ANYFILE_BUFFER_SIZE)))
	{
		fprintf(stderr, "%s: Failed to allocate buffer of size %i\n", prog_name, ANYFILE_BUFFER_SIZE);
		goto exit;
	}

	if (!(source = anyfile_open(source_ascii, READ, session)))
	{
		fprintf(stderr, "%s: Failed to open source file '%s'\n", prog_name, source_ascii);
		goto exit;
	}

	if (!(dest = anyfile_open(dest_ascii, WRITE, session)))
	{
		fprintf(stderr, "%s: Failed to open destination file '%s'\n", prog_name, dest_ascii);
		goto exit;
	}

	for(;;)
	{
		if (!anyfile_read(source, buffer, ANYFILE_BUFFER_SIZE, &bytes_read))
		{
			fprintf(stderr, "%s: Failed to read from source file '%s'\n", prog_name, source_ascii);
			goto exit;
		}

		if (0 == bytes_read)
		{
			/* End of file */
			break;
		}

		if (!anyfile_write(dest, buffer, bytes_read, &bytes_written))
		{
			fprintf(stderr, "%s: Failed to write to destination file '%s'\n", prog_name, dest_ascii);
			goto exit;
		}

		if ((int)bytes_written != (int)bytes_read)
		{
			fprintf(stderr, "%s: Only wrote %zi bytes of %zi to destination file '%s'\n", prog_name, 
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
}

static bool copy_file(IRAPISession *session, const char* source, const char* dest, size_t* bytes_copied)
{
	if (is_remote_file(source) && is_remote_file(dest))
	{
		/*
		 * Both are remote; use CeCopyFile()
		 */
		if (!remote_copy(session, source, dest))
		  return false;;
	}
	else
	{
		/*
		 * At least one is local, Use the AnyFile functions
		 */
		if (!anyfile_copy(session, source, dest, bytes_copied))
		  return false;

	}
	return true;
}

static bool do_copy(IRAPISession *session, const char* source, const char* dest, size_t* bytes_copied);

static bool copy_dir(IRAPISession *session, const char* source, const char* dest, size_t* bytes_copied)
{
  char *src_list, *filename;
  char *new_src, *new_dest;
  CE_FIND_DATA *data = NULL;
  uint itemcount;
  uint i;
  WCHAR *widestr = NULL;
  BOOL rapi_result;
  HRESULT hr;
  DWORD last_error;
  DIR *dir_handle;
  struct dirent *dir_entry;
  bool result = false;

  if (is_remote_file(source)) {
    src_list = calloc(1, strlen(source) + 3);
    src_list = strcat(src_list, source);
    src_list = strcat(src_list, "/*");
    convert_to_backward_slashes(src_list);

    widestr = wstr_from_current(src_list+1);
    if (!widestr) {
            fprintf(stderr, "%s: Failed to convert the name '%s' from the current encoding to UCS2\n", prog_name, src_list+1);
            return false;
    }

    free(src_list);

    rapi_result = IRAPISession_CeFindAllFiles(session, widestr, FAF_ATTRIBUTES | FAF_NAME , &itemcount, &data);
    wstr_free_string(widestr);

    if (!rapi_result)
      {
	if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
	  fprintf(stderr, "%s: error opening directory %s: %s\n",
		  prog_name, source, synce_strerror(hr));
	  return false;
	}

	last_error = IRAPISession_CeGetLastError(session);
	fprintf(stderr, "%s: error opening directory %s: %s\n",
		  prog_name, source, synce_strerror(last_error));
	return false;
      }

    for (i = 0; i < itemcount; i++) {
      filename = wstr_to_current(data[i].cFileName);
      if (!filename) {
              fprintf(stderr, "%s: Failed to convert a filename to the current encoding, skipping\n", prog_name);
              continue;
      }

      new_src = calloc(1, strlen(source) + 1 + strlen(filename) + 1);
      new_src = strcat(new_src, source);
      new_src = strcat(new_src, "/");
      new_src = strcat(new_src, filename);

      new_dest = calloc(1, strlen(dest) + 1 + strlen(filename) + 1);
      new_dest = strcat(new_dest, dest);
      new_dest = strcat(new_dest, "/");
      new_dest = strcat(new_dest, filename);

      if (data[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
	result = do_copy(session, new_src, new_dest, bytes_copied);
      } else {
	result = copy_file(session, new_src, new_dest, bytes_copied);
      }

      free(new_src);
      free(new_dest);
    }

    IRAPISession_CeRapiFreeBuffer(session, data);
  } else {
    if (!(dir_handle = opendir(source))) {
      fprintf(stderr, "%s: error opening directory %s: %s\n",
	      prog_name, source, strerror(errno));
      return false;
    }
    while ((dir_entry = readdir(dir_handle)) != NULL) {
      if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0))
	continue;

      new_src = calloc(1, strlen(source) + 1 + strlen(dir_entry->d_name) + 1);
      new_src = strcat(new_src, source);
      new_src = strcat(new_src, "/");
      new_src = strcat(new_src, dir_entry->d_name);

      new_dest = calloc(1, strlen(dest) + 1 + strlen(dir_entry->d_name) + 1);
      new_dest = strcat(new_dest, dest);
      new_dest = strcat(new_dest, "/");
      new_dest = strcat(new_dest, dir_entry->d_name);

      if (dir_entry->d_type == DT_DIR) {
	result = do_copy(session, new_src, new_dest, bytes_copied);
      } else {
	result = copy_file(session, new_src, new_dest, bytes_copied);
      }

      free(new_src);
      free(new_dest);
    }

    closedir(dir_handle);
  }

  return result;
}


static bool does_exist(IRAPISession *session, const char* name)
{
  if (is_remote_file(name))
    {
      CE_FIND_DATA entry;
      WCHAR *tempwstr = NULL;
      HANDLE handle;
      HRESULT hr;
      DWORD last_error;

      tempwstr = wstr_from_current(name+1);
      if (!tempwstr) {
              fprintf(stderr, "%s: Failed to convert the name '%s' from the current encoding to UCS2\n", prog_name, name);
              return false;
      }

      handle = IRAPISession_CeFindFirstFile(session, tempwstr, &entry);
      wstr_free_string(tempwstr);

      if(handle == INVALID_HANDLE_VALUE)
	{
	  if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
	    fprintf(stderr, "%s: error finding %s: %s\n",
		    prog_name, name, synce_strerror(hr));
	    return false;
	  }

	  last_error = IRAPISession_CeGetLastError(session);

	  if (last_error == ERROR_NO_MORE_FILES)
	    return false;

	  fprintf(stderr, "%s: error finding %s: %s\n",
		  prog_name, name, synce_strerror(last_error));
	  return false;
	}
      IRAPISession_CeFindClose(session, handle);

      return true;
    }
  else
    {
      struct stat entry;

      if (stat(name, &entry) != 0) {
	return false;
      }
      return true;
    }

  return false;
}


static bool is_dir(IRAPISession *session, const char* name)
{
  if (is_remote_file(name))
    {
      CE_FIND_DATA entry;
      WCHAR *tempwstr = NULL;
      HANDLE handle;
      HRESULT hr;
      DWORD last_error;

      tempwstr = wstr_from_current(name+1);
      if (!tempwstr) {
              fprintf(stderr, "%s: Failed to convert the name '%s' from the current encoding to UCS2\n", prog_name, name);
              return false;
      }

      handle = IRAPISession_CeFindFirstFile(session, tempwstr, &entry);
      wstr_free_string(tempwstr);

      if(handle == INVALID_HANDLE_VALUE)
	{
	  if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
	    fprintf(stderr, "%s: error finding %s: %s\n",
		    prog_name, name, synce_strerror(hr));
	    return false;
	  }

	  last_error = IRAPISession_CeGetLastError(session);
	  fprintf(stderr, "%s: error finding %s: %s\n",
		  prog_name, name, synce_strerror(last_error));
	  return false;
	}
      IRAPISession_CeFindClose(session, handle);

      if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	return true;
    }
  else
    {
      struct stat entry;

      if (stat(name, &entry) != 0) {
	fprintf(stderr, "%s: error finding %s: %s\n",
		prog_name, name, strerror(errno));
	return false;
      }
      if (S_ISDIR(entry.st_mode))
	return true;
    }

  return false;
}

static char *any_basename(const char *path)
{
  const char *p = NULL;
  char *name = NULL;

  if (is_remote_file(path))
    {
      for (p = path + strlen(path); p != path; p--)
	{
	  if (*p == '/' || *p == '\\')
	    {
	      name = strdup(p+1);
	      break;
	    }
	}

      if (!name || '\0' == name[0])
	name = strdup(path+1);
    }
  else
    {
      p = strrchr(path, '/');

      if (p)
	name = strdup(p+1);
      else
	name = strdup(path);
    }
  return name;
}

static bool do_copy(IRAPISession *session, const char* source, const char* dest, size_t* bytes_copied)
{
  bool result;
  char *dir_name;
  char *actual_dest;
  HRESULT hr;
  DWORD last_error;

  if (is_dir(session, source)) {
    if (!recursive) {
      fprintf(stderr, "%s: omitting directory '%s'\n", prog_name, source);
      return false;
    }

    if (does_exist(session, dest)) {
      if (!is_dir(session, dest)) {
	fprintf(stderr, "%s: cannot overwrite non-directory '%s' with directory '%s'\n", prog_name, dest, source);
	return false;
      }

      dir_name = any_basename(source);

      actual_dest = calloc(1, 1 + strlen(dest) + 1 + strlen(dir_name) + 1);

      actual_dest = strcat(actual_dest, dest);
      if (is_remote_file(dest))
	actual_dest = strcat(actual_dest, "\\");
      else
	actual_dest = strcat(actual_dest, "/");
      actual_dest = strcat(actual_dest, dir_name);

      free(dir_name);

    } else {
      actual_dest = strdup(dest);
    }

    /* need to make sure dir called actual_dest exists */
    if (is_remote_file(actual_dest)) {
      LPWSTR tmpwstr;
      BOOL rapi_result;

      tmpwstr = wstr_from_current(actual_dest+1);
      if (!tmpwstr) {
              fprintf(stderr, "%s: Failed to convert the directory name '%s' from the current encoding to UCS2\n", prog_name, actual_dest);
              free(actual_dest);
              return false;
      }

      rapi_result = IRAPISession_CeCreateDirectory(session, tmpwstr, NULL);
      wstr_free_string(tmpwstr);

      if (!rapi_result) {
	if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
	  fprintf(stderr, "%s: error creating directory '%s': %08x: %s\n",
		  prog_name, actual_dest, hr, synce_strerror(hr));
	  free(actual_dest);
	  return false;
	}

	last_error = IRAPISession_CeGetLastError(session);

	if (last_error != ERROR_ALREADY_EXISTS) {
	  fprintf(stderr, "%s: error creating directory '%s': %d: %s\n",
		  prog_name, actual_dest, last_error, synce_strerror(last_error));
	  free(actual_dest);
	  return false;
	}
      }

    } else {
      if (mkdir(actual_dest, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
	if (errno != EEXIST) {
	  fprintf(stderr, "%s: unable to create directory '%s': %s\n", prog_name, actual_dest, strerror(errno));
	  free(actual_dest);
	  return false;
	}
    }

    result = copy_dir(session, source, actual_dest, bytes_copied);

    free(actual_dest);
  } else {

    if (does_exist(session, dest)) {
      if (!is_dir(session, dest)) {
	fprintf(stderr, "%s: file '%s' already exists\n", prog_name, dest);
	return false;
      }

      dir_name = any_basename(source);

      actual_dest = calloc(1, 1 + strlen(dest) + 1 + strlen(dir_name) + 1);

      actual_dest = strcat(actual_dest, dest);
      if (is_remote_file(dest))
	actual_dest = strcat(actual_dest, "\\");
      else
	actual_dest = strcat(actual_dest, "/");
      actual_dest = strcat(actual_dest, dir_name);

      free(dir_name);

    } else {
      actual_dest = strdup(dest);
    }

    result = copy_file(session, source, actual_dest, bytes_copied);
    free(actual_dest);
  }

  return result;
}

int main(int argc, char** argv)
{
	int result = 1;
	IRAPIDesktop *desktop = NULL;
	IRAPIEnumDevices *enumdev = NULL;
	IRAPIDevice *device = NULL;
	IRAPISession *session = NULL;
	RAPI_DEVICEINFO devinfo;
	char* source = NULL;
	char* dest = NULL;
	HRESULT hr;
	time_t start;
	time_t duration;
	size_t bytes_copied = 0;
	DWORD last_error;
	
	if (!handle_parameters(argc, argv, &source, &dest))
		goto exit;

	if (FAILED(hr = IRAPIDesktop_Get(&desktop)))
	{
	  fprintf(stderr, "%s: failed to initialise RAPI: %d: %s\n", 
		  argv[0], hr, synce_strerror_from_hresult(hr));
	  goto exit;
	}

	if (FAILED(hr = IRAPIDesktop_EnumDevices(desktop, &enumdev)))
	{
	  fprintf(stderr, "%s: failed to get connected devices: %d: %s\n", 
		  argv[0], hr, synce_strerror_from_hresult(hr));
	  goto exit;
	}

	while (SUCCEEDED(hr = IRAPIEnumDevices_Next(enumdev, &device)))
	{
	  if (dev_name == NULL)
	    break;

	  if (FAILED(IRAPIDevice_GetDeviceInfo(device, &devinfo)))
	  {
	    fprintf(stderr, "%s: failure to get device info\n", argv[0]);
	    goto exit;
	  }
	  if (strcmp(dev_name, devinfo.bstrName) == 0)
	    break;
	}

	if (FAILED(hr))
	{
	  fprintf(stderr, "%s: Could not find device '%s': %08x: %s\n", 
		  argv[0],
		  dev_name?dev_name:"(Default)", hr, synce_strerror_from_hresult(hr));
	  device = NULL;
	  goto exit;
	}

	IRAPIDevice_AddRef(device);
	IRAPIEnumDevices_Release(enumdev);
	enumdev = NULL;

	if (FAILED(hr = IRAPIDevice_CreateSession(device, &session)))
	{
	  fprintf(stderr, "%s: Could not create a session to device: %08x: %s\n", 
		  argv[0], hr, synce_strerror_from_hresult(hr));
	  goto exit;
	}

	if (FAILED(hr = IRAPISession_CeRapiInit(session)))
	{
	  fprintf(stderr, "%s: Unable to initialize connection to device: %08x: %s\n", 
		  argv[0], hr, synce_strerror_from_hresult(hr));
	  goto exit;
	}

	if (!dest)
	{
		char* p;

		if (is_remote_file(source))
		{

			for (p = source + strlen(source); p != source; p--)
			{
				if (*p == '/' || *p == '\\')
				{
					dest = strdup(p+1);
					break;
				}
			}

			if (!dest || '\0' == dest[0])
			{
				fprintf(stderr, "%s: Unable to extract destination filename from source path '%s'\n",
						argv[0], source);
				goto exit;
			}
		}
		else
		{
			WCHAR mydocuments[MAX_PATH];
			char* mydocuments_char = NULL;
			p = strrchr(source, '/');

			if (p)
				p++;
			else
				p = source;

			if ('\0' == *p)
			{
				fprintf(stderr, "%s: Unable to extract destination filename from source path '%s'\n",
						argv[0], source);
				goto exit;
			}

			if (!IRAPISession_CeGetSpecialFolderPath(session, CSIDL_PERSONAL, sizeof(mydocuments), mydocuments))
			  {

			    if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
			      fprintf(stderr, "%s: Unable to get the \"My Documents\" path: %s.\n",
				      argv[0], synce_strerror(hr));
			      goto exit;
			    }

			    last_error = IRAPISession_CeGetLastError(session);
			    fprintf(stderr, "%s: Unable to get the \"My Documents\" path: %s.\n",
				    argv[0], synce_strerror(last_error));
			    goto exit;
			  }

			mydocuments_char = wstr_to_current(mydocuments);
                        if (!mydocuments_char) {
                                fprintf(stderr, "%s: Failed to convert the \"My Documents\" path to the current encoding", argv[0]);
                                goto exit;
                        }

			dest = calloc(1, 1 + wstr_strlen(mydocuments) + 1 + strlen(p) + 1);

			strcat(dest, ":");
			strcat(dest, mydocuments_char);
			strcat(dest, "\\");
			strcat(dest, p);
			
			wstr_free_string(mydocuments_char);
		}
	}

	/* remove trailing slashes, unless it's root */

	if (source[strlen(source) - 1] == '\\' || source[strlen(source) - 1] == '/')
	  source[strlen(source) - 1] = '\0';
	if (dest[strlen(dest) - 1] == '\\' || dest[strlen(dest) - 1] == '/')
	  dest[strlen(dest) - 1] = '\0';

	if (0 == strcmp(source, dest))
	{
		fprintf(stderr, "You don't want to copy a file to itself.\n");
		goto exit;
	}

  	if (!is_remote_file(source) && !is_remote_file(dest))
	  {
	    fprintf(stderr, "Warning: You are about to copy from one local file to another.\n"
		    "Please view the built-in help or read the man page.\n");
	  }

	start = time(NULL);

	if (!do_copy(session, source, dest, &bytes_copied))
	  goto exit;

	duration = time(NULL) - start;

	if (0 == duration)
	  printf("File copy took less than one second!\n");
	else
	  if (bytes_copied > 0)
	    printf("File copy of %zi bytes took %li minutes and %li seconds, that's %li bytes/s.\n",
		   bytes_copied, duration / 60, duration % 60, bytes_copied / duration);
	  else
	    printf("File copy took %li minutes and %li seconds.\n",
		   duration / 60, duration % 60);

	result = 0;

exit:
	if (source)
		free(source);

	if (dest)
		free(dest);

	if (session)
	{
	  IRAPISession_CeRapiUninit(session);
	  IRAPISession_Release(session);
	}

	if (device) IRAPIDevice_Release(device);
	if (enumdev) IRAPIEnumDevices_Release(enumdev);
	if (desktop) IRAPIDesktop_Release(desktop);
	return result;
}
