#include "pcommon.h"
#include <rapi2.h>
#include <synce_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>


static void show_usage(const char* name)
{
	fprintf(stderr,
			"Syntax:\n"
			"\n"
			"\t%s [-d LEVEL] [ -p DEVNAME ] [-h] SHORTCUT TARGET\n"
			"\n"
			"\t-d LEVEL     Set debug log level\n"
			"\t                 0 - No logging (default)\n"
			"\t                 1 - Errors only\n"
			"\t                 2 - Errors and warnings\n"
			"\t                 3 - Everything\n"
			"\t-h           Show this help message\n"
                        "\t-p DEVNAME   Mobile device name\n"
                        "\tSHORTCUT     Shortcut to create\n"
                        "\tTARGET       Shortcut target\n"
			,name);
}

static bool handle_parameters(int argc, char** argv, char **shortcut, char **target, char** dev_name)
{
	int c;
	int log_level = SYNCE_LOG_LEVEL_LOWEST;

	while ((c = getopt(argc, argv, "d:hp:")) != -1)
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

	if ((argc - optind) != 2)
	{
		fprintf(stderr, "%s: You need to specify shortcut and target file names on command line\n\n", argv[0]);
		show_usage(argv[0]);
		return false;
	}
		
	*shortcut = strdup(argv[optind++]);
	*target   = strdup(argv[optind++]);

	return true;
}

int main(int argc, char** argv)
{
  int result = 0;
  IRAPIDesktop *desktop = NULL;
  IRAPIEnumDevices *enumdev = NULL;
  IRAPIDevice *device = NULL;
  IRAPISession *session = NULL;
  RAPI_DEVICEINFO devinfo;
  HRESULT hr;
  char *shortcut = NULL, *target = NULL;
  WCHAR* wide_shortcut = NULL;
  WCHAR* wide_target = NULL;
  WCHAR *tmp, *tmp_quote;
  size_t tmpsize;
  char* dev_name = NULL;

  if (!handle_parameters(argc, argv, &shortcut, &target, &dev_name))
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

  convert_to_backward_slashes(shortcut);
  wide_shortcut = wstr_from_current(shortcut);
  if (!wide_shortcut) {
          fprintf(stderr, "%s: Failed to convert shortcut name '%s' from current encoding to UCS2\n", 
                  argv[0],
                  shortcut);
          goto exit;
  }

  wide_shortcut = adjust_remote_path(session, wide_shortcut, true);

  convert_to_backward_slashes(target);
  wide_target = wstr_from_current(target);
  if (!wide_target) {
          fprintf(stderr, "%s: Failed to convert shortcut target '%s' from current encoding to UCS2\n", 
                  argv[0],
                  target);
          goto exit;
  }

  wide_target = adjust_remote_path(session, wide_target, true);
  /* Wrap target in quotes.  This is required for paths with spaces (for some reason) */
  tmp_quote = wstr_from_current("\"");
  tmpsize = (wstrlen(wide_target) + 3) * sizeof(WCHAR);
  tmp = malloc(tmpsize);
  if (!tmp)
    goto exit;
  wstrcpy(tmp,tmp_quote);
  if (!wstr_append(tmp,wide_target,tmpsize))
    goto exit;
  if (!wstr_append(tmp,tmp_quote,tmpsize))
    goto exit;
  wstr_free_string(wide_target);
  wstr_free_string(tmp_quote);
  wide_target = tmp;
  
  BOOL res = IRAPISession_CeSHCreateShortcut(session, wide_shortcut, wide_target);
  if (!res)
  {
    fprintf(stderr, "%s: Unable to create shortcut to '%s' at '%s': %s\n",
            argv[0],target,shortcut,
	    synce_strerror(IRAPISession_CeGetLastError(session)));
    result = 1;
    goto exit;
  }

 exit:
  wstr_free_string(wide_shortcut);
  wstr_free_string(wide_target);
  
  if (shortcut)
    free(shortcut);
  
  if (target)
    free(target);

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
