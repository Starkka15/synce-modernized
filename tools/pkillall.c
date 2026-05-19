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
			"\t%s [-d LEVEL] [-p DEVNAME] [-h]\n"
			"\n"
			"\t-d LEVEL     Set debug log level\n"
			"\t                 0 - No logging (default)\n"
			"\t                 1 - Errors only\n"
			"\t                 2 - Errors and warnings\n"
			"\t                 3 - Everything\n"
			"\t-h           Show this help message\n"
                        "\t-p DEVNAME   Mobile device name\n",
			name);
}

static bool handle_parameters(int argc, char** argv, char** dev_name)
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
  char* dev_name = NULL;

  if (!handle_parameters(argc, argv, &dev_name))
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

  if (!IRAPISession_CeKillAllApps(session))
  {
    fprintf(stderr,"%s: CeKillAllApps failed: %s: %s\n",argv[0], synce_strerror(IRAPISession_CeGetLastError(session)), synce_strerror(IRAPISession_CeRapiGetError(session)));
    result = 2;
    goto exit;
  }

 exit:
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
