/* $Id: synce-list-programs.c 4136 2012-11-22 15:10:51Z mark_ellis $ */
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
			"\t%s [-d LEVEL] [-p DEVNAME] [-h]\n"
			"\n"
			"\t-d LEVEL     Set debug log level\n"
			"\t                 0 - No logging (default)\n"
			"\t                 1 - Errors only\n"
			"\t                 2 - Errors and warnings\n"
			"\t                 3 - Everything\n"
                        "\t-p DEVNAME   Mobile device name\n"
			"\t-h           Show this help message\n",
			name);
}

static bool handle_parameters(int argc, char** argv, char** dev_name)
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

	return true;
}

int main(int argc, char** argv)
{
	int return_value = 1;
	IRAPIDesktop *desktop = NULL;
	IRAPIEnumDevices *enumdev = NULL;
	IRAPIDevice *device = NULL;
	IRAPISession *session = NULL;
	RAPI_DEVICEINFO devinfo;
	HRESULT hr;
	LONG result;
	HKEY parent_key;
	WCHAR* parent_key_name = NULL;
	WCHAR* value_name = NULL;
	DWORD i;
	bool smartphone = false;
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

        /* Path on SmartPhone 2002 */
        parent_key_name = wstr_from_current("Security\\AppInstall");
        if (!parent_key_name) {
		fprintf(stderr, "%s: Failed to convert registry key from current encoding to UCS2\n", argv[0]);
		goto exit;
        }

        result = IRAPISession_CeRegOpenKeyEx(session, HKEY_LOCAL_MACHINE, parent_key_name, 0, 0, &parent_key);
  
	if (ERROR_SUCCESS == result)
  {
    smartphone = true;
  }
  else
  {
    smartphone = false;
    wstr_free_string(parent_key_name);

    /* Path on Pocket PC 2002 */
    parent_key_name = wstr_from_current("Software\\Apps");
    if (!parent_key_name) {
            fprintf(stderr, "%s: Failed to convert registry key from current encoding to UCS2\n", argv[0]);
            goto exit;
    }

    result = IRAPISession_CeRegOpenKeyEx(session, HKEY_LOCAL_MACHINE, parent_key_name, 0, 0, &parent_key);

    if (ERROR_SUCCESS != result)
    {
      fprintf(stderr, "%s: Unable to open parent registry key: %s\n", 
          argv[0],
          synce_strerror(result));
      goto exit;
    }
  }
  
  value_name = wstr_from_current("Instl");
  if (!value_name) {
          fprintf(stderr, "%s: Failed to convert registry value name from current encoding to UCS2\n", argv[0]);
          goto exit;
  }
	
	for (i = 0; ; i++)
	{
		WCHAR wide_name[MAX_PATH];
		DWORD name_size = sizeof(wide_name);
		HKEY program_key;
		DWORD installed = 0;
		DWORD value_size = sizeof(installed);

		result = IRAPISession_CeRegEnumKeyEx(session, parent_key, i, wide_name, &name_size, NULL, NULL,
				NULL, NULL);
		if (ERROR_SUCCESS != result)
			break;

    if (smartphone)
    {
      char* name = wstr_to_current(wide_name);
      if (!name) {
              fprintf(stderr, "%s: Failed to convert application name to current encoding\n", argv[0]);
              continue;
      } else {
              puts(name);
              wstr_free_string(name);
      }
    }
    else
    {
      result = IRAPISession_CeRegOpenKeyEx(session, parent_key, wide_name, 0, 0, &program_key);
      if (ERROR_SUCCESS != result)
        continue;

      result = IRAPISession_CeRegQueryValueEx(session, program_key, value_name, NULL, NULL,
          (LPBYTE)&installed, &value_size);

      if (ERROR_SUCCESS == result && installed)
      {
        char* name = wstr_to_current(wide_name);
        if (!name) {
                fprintf(stderr, "%s: Failed to convert application name to current encoding\n", argv[0]);
                continue;
        } else {
                puts(name);
                wstr_free_string(name);
        }
      }
      IRAPISession_CeRegCloseKey(session, program_key);
    }

	}

	IRAPISession_CeRegCloseKey(session, parent_key);

	return_value = 0;

exit:
	wstr_free_string(parent_key_name);
	wstr_free_string(value_name);

	if (session)
	{
	  IRAPISession_CeRapiUninit(session);
	  IRAPISession_Release(session);
	}

	if (device) IRAPIDevice_Release(device);
	if (enumdev) IRAPIEnumDevices_Release(enumdev);
	if (desktop) IRAPIDesktop_Release(desktop);
	return return_value;
}
