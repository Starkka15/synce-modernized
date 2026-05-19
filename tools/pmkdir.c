/* $Id: pmkdir.c 4136 2012-11-22 15:10:51Z mark_ellis $ */
#include "pcommon.h"
#include <rapi2.h>
#include <synce_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static void show_usage(const char* name)
{
	fprintf(stderr,
			"Syntax:\n"
			"\n"
			"\t%s [-d LEVEL] [-p DEVNAME] [-h] FILE\n"
			"\n"
			"\t-d LEVEL     Set debug log level\n"
			"\t                 0 - No logging (default)\n"
			"\t                 1 - Errors only\n"
			"\t                 2 - Errors and warnings\n"
			"\t                 3 - Everything\n"
			"\t-h           Show this help message\n"
                        "\t-p DEVNAME   Mobile device name\n"
			"\tFILE         The remote directory you want to create\n",
			name);
}

static bool handle_parameters(int argc, char** argv, char** path, char** dev_name)
{
	int c;
	int log_level = SYNCE_LOG_LEVEL_LOWEST;

	while ((c = getopt(argc, argv, "d:p:h")) != -1)
	{
		switch (c)
		{
			case 'd':
				log_level = atoi(optarg);
				break;
			
                        case 'p':
                                *dev_name = optarg;
                                break;
			
			case 'h':
			default:
				show_usage(argv[0]);
				return false;
		}
	}

	synce_log_set_level(log_level);

	if (optind == argc)
	{
		fprintf(stderr, "%s: No directory name specified on command line\n\n", argv[0]);
		show_usage(argv[0]);
		return false;
	}
		
	/* TODO: handle more than one path */
	*path = strdup(argv[optind]);

	return true;
}

int main(int argc, char** argv)
{
	int result = 1;
	IRAPIDesktop *desktop = NULL;
	IRAPIEnumDevices *enumdev = NULL;
	IRAPIDevice *device = NULL;
	IRAPISession *session = NULL;
	RAPI_DEVICEINFO devinfo;
	char* path = NULL;
	HRESULT hr;
	WCHAR* wide_path = NULL;
	char* dev_name = NULL;
	
	if (!handle_parameters(argc, argv, &path, &dev_name))
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

	convert_to_backward_slashes(path);
	wide_path = wstr_from_current(path);
        if (!wide_path) {
		fprintf(stderr, "%s: Failed to convert path '%s' from current encoding to UCS2\n", 
				argv[0],
				path);
		goto exit;
        }

	wide_path = adjust_remote_path(session, wide_path, true);

	if (!IRAPISession_CeCreateDirectory(session, wide_path, NULL))
	{
		fprintf(stderr, "%s: Failed to create directory '%s': %s\n", 
				argv[0],
				path,
				synce_strerror(IRAPISession_CeGetLastError(session)));
		goto exit;
	}

	result = 0;

exit:
	if (wide_path)
		wstr_free_string(wide_path);

	if (path)
		free(path);

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
