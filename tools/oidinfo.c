/* $Id: oidinfo.c 4136 2012-11-22 15:10:51Z mark_ellis $ */
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
			"\t%s [-d LEVEL] [-p DEVNAME] [-h] OID\n"
			"\n"
			"\t-d LEVEL     Set debug log level\n"
			"\t                 0 - No logging (default)\n"
			"\t                 1 - Errors only\n"
			"\t                 2 - Errors and warnings\n"
			"\t                 3 - Everything\n"
                        "\t-p DEVNAME   Mobile device name\n"
			"\t-h           Show this help message\n"
			"\tOID          The object identifier we want to know more about\n",
			name);
}

static bool handle_parameters(int argc, char** argv, CEOID* oid, char** dev_name)
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
		printf("Missing OID parameter\n\n");
		show_usage(argv[0]);
		return false;
	}

	*oid = strtoul(argv[optind], NULL, 0);

	return true;
}

int main(int argc, char** argv)
{
	int result = 1;
	HRESULT hr;
	CEOID oid = 0;
	CEOIDINFO info;
	IRAPIDesktop *desktop = NULL;
	IRAPIEnumDevices *enumdev = NULL;
	IRAPIDevice *device = NULL;
	IRAPISession *session = NULL;
	RAPI_DEVICEINFO devinfo;
	char* dev_name = NULL;

	if (!handle_parameters(argc, argv, &oid, &dev_name))
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

	memset(&info, 0, sizeof(info));

	if (IRAPISession_CeOidGetInfo(session, oid, &info))
	{
		switch (info.wObjType)
		{
			case OBJTYPE_INVALID:
				printf("Invalid object\n");
				break;

			case OBJTYPE_FILE:
				printf("File object:\n"
						   "============\n");
				{
					char *name = wstr_to_current(info.u.infFile.szFileName);
                                        if (!name) {
                                                printf("Error: failed to convert object name to current encoding\n");
                                        } else {
                                                printf("Attributes:  %08x\n"
                                                       "Name:        \"%s\"\n"
                                                       , 
                                                       info.u.infFile.dwAttributes,
                                                       name);
                                                wstr_free_string(name);
                                        }
				}
				break;
				
			case OBJTYPE_DIRECTORY:
				printf("Directory object:\n"
						   "=================\n");
				{
					char *name = wstr_to_current(info.u.infDirectory.szDirName);
                                        if (!name) {
                                                printf("Error: failed to convert object name to current encoding\n");
                                        } else {
                                                printf("Attributes:  %08x\n"
                                                       "Parent OID:  %08x\n"
                                                       "Name:        \"%s\"\n"
                                                       , 
                                                       info.u.infDirectory.dwAttributes,
                                                       info.u.infDirectory.oidParent,
                                                       name);
                                                wstr_free_string(name);
					}
				}
				break;

			case OBJTYPE_DATABASE:
				printf(
						"Database:\n"
						"=========\n");
				{
					char *name = wstr_to_current(info.u.infDatabase.szDbaseName);
                                        if (!name) {
                                                printf("Error: failed to convert object name to current encoding\n");
                                        } else {
                                                printf(
                                                       "Flags: %08x\n"
                                                       "Name: \"%s\"\n"
                                                       "Type: %08x\n"
                                                       "Record count: %i\n"
                                                       "Sort order count: %i\n"
                                                       "Size: %i\n"
                                                       ,
                                                       info.u.infDatabase.dwFlags,
                                                       name,
                                                       info.u.infDatabase.dwDbaseType,
                                                       info.u.infDatabase.wNumRecords,
                                                       info.u.infDatabase.wNumSortOrder,
                                                       info.u.infDatabase.dwSize
                                                       );
                                                wstr_free_string(name);
					}
				}
				break;

			case OBJTYPE_RECORD:
				printf(
						"Database record:\n"
						"================\n"
						"Parent OID: %08x\n", 
						info.u.infRecord.oidParent);
				break;

			default:
				printf("Unknown object with type %i\n", info.wObjType);
				break;
		}
	}
	else
	{
		fprintf(stderr, "%s: Failed to get object information: %s\n", 
				argv[0],
				synce_strerror(IRAPISession_CeGetLastError(session)));
		goto exit;
	}

	result = 0;

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
