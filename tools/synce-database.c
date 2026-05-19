/* $Id: synce-registry.c 3688 2009-01-29 18:39:53Z mark_ellis $ */
#include <rapi2.h>
#include <synce_log.h>
#include <pcommon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>


char* dev_name = NULL;
bool list_recurse = false ; 
const char *prog_name;


static void show_usage(const char* name)
{
	fprintf(stderr,
			"Syntax:\n"
			"\n"
			"\t%s [-d LEVEL] [-p DEVNAME] [-t TYPE ] [-h]\n"
                        "\t[-r] PARENTKEY KEY VALUE\t\tRead value\n"
                        "\t  -w PARENTKEY KEY VALUE NEWVALUE\tWrite value\n"
                        "\t  -l PARENTKEY KEY\t\tList key\n"
                        "\t  -n PARENTKEY KEY\t\t\tNew key\n"
                        "\t  -x PARENTKEY KEY VALUE\t\tDelete value (not supported)\n"
                        "\t  -X PARENTKEY KEY\t\t\tDelete key (not supported)\n"
			"\n"
			"\t-d LEVEL     Set debug log level\n"
			"\t                 0 - No logging (default)\n"
			"\t                 1 - Errors only\n"
			"\t                 2 - Errors and warnings\n"
			"\t                 3 - Everything\n"
			"\t-D           Dump complete registry to screen\n"
			"\t-L           Enable list recursion\n"
			"\t-h           Show this help message\n"
                        "\t-p DEVNAME   Mobile device name\n"
                        "\t-t TYPE      New type for writes:\n"
                        "\t\tsz         String (default)\n"
                        "\t\tdword      Double-word\n"
                        "\t\tdword_bige Big-endian double-word\n"
                        "\t\texpand_sz  String with path expansion\n"
                        "\t\tmulti_sz   Multiple strings (not supported)\n"
                        "\t\tbinary     Binary data (not supported)\n"
			"\tPARENTKEY    Registry parent (HKLM, etc.)\n"
			"\tKEY          Registry key\n"
			"\tVALUE        Registry value within key\n"
			"\tNEWVALUE     New value for writes\n"
                        "",
			name
                );
}

#if 0
static void dump(const char *desc, void* data, size_t len)/*{{{*/
{
	uint8_t* buf = (uint8_t*)data;
	size_t i, j;
	char hex[8 * 3 + 1];
	char chr[8 + 1];

	printf("%s (%zd bytes):\n", desc, len);
	for (i = 0; i < len + 7; i += 8) {
		for (j = 0; j < 8; j++) 
			if (j + i >= len) {
				hex[3*j+0] = ' ';
				hex[3*j+1] = ' ';
				hex[3*j+2] = ' ';
				chr[j] = ' ';
			} else {
				uint8_t c = buf[j + i];
				const char *hexchr = "0123456789abcdef";
				hex[3*j+0] = hexchr[(c >> 4) & 0xf];
				hex[3*j+1] = hexchr[c & 0xf];
				hex[3*j+2] = ' ';
				if (c > ' ' && c <= '~')
					chr[j] = c;
				else
					chr[j] = '.';
			}
		hex[8*3] = '\0';
		chr[8] = '\0';
		printf("  %04zx: %s %s\n", i, hex, chr);
	}
}/*}}}*/
#endif

const char * property_type_to_str(CEPROPID propid)
{
        switch (propid & 0xffff)
                {
                case CEVT_BLOB:
                        return "CEVT_BLOB";
                        break;
                case CEVT_BOOL:
                        return "CEVT_BOOL";
                        break;
                case CEVT_FILETIME:
                        return "CEVT_FILETIME";
                        break;
                case CEVT_I2:
                        return "CEVT_I2";
                        break;
                case CEVT_I4:
                        return "CEVT_I4";
                        break;
                case CEVT_LPWSTR:
                        return "CEVT_LPWSTR";
                        break;
                case CEVT_R8:
                        return "CEVT_R8";
                        break;
                case CEVT_UI2:
                        return "CEVT_UI2";
                        break;
                case CEVT_UI4:
                        return "CEVT_UI4";
                        break;
                default:
                        break;
                }
        return "Unknown type";
}

static bool handle_parameters(int argc, char** argv)
{
	int c;
	int log_level = SYNCE_LOG_LEVEL_LOWEST;
	while ((c = getopt(argc, argv, "d:hp:t:rwxlnXDL")) != -1)
	{
		switch (c)
		{
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

        argc -= optind;

	return true;
}

bool
list_databases_by_all(IRAPISession *session)
{
        HRESULT hr;
        DWORD last_error;
        CEDB_FIND_DATA* find_data = NULL;
        WORD db_count = 0;
        WORD flags = 0;
        BOOL retval;
        char *db_name = NULL;
        unsigned i;
        bool result = FALSE;

        flags = FAD_OID | FAD_FLAGS | FAD_NAME | FAD_TYPE | FAD_NUM_RECORDS | FAD_NUM_SORT_ORDER | FAD_SIZE | FAD_LAST_MODIFIED | FAD_SORT_SPECS;

        retval = IRAPISession_CeFindAllDatabases(session, 0, flags, &db_count, &find_data);
        if (!retval) {
                if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
                        fprintf(stderr, "%s: Failed to enumerate databases: %s\n",
                                prog_name, synce_strerror(hr));
                        goto exit;
                }

                last_error = IRAPISession_CeGetLastError(session);
                fprintf(stderr, "%s: Failed to enumerate databases: %s\n",
                        prog_name, synce_strerror(last_error));
                goto exit;
        }

        printf("Number of databases: %u\n", db_count);
        for (i = 0; i < db_count; i++) {
                db_name = wstr_to_current(find_data[i].DbInfo.szDbaseName);

                printf("Database %3u: \"%s\" (oid=0x%x, type=0x%x, rows=%u)\n",
                       i,
                       db_name,
                       find_data[i].OidDb,
                       (unsigned)find_data[i].DbInfo.dwDbaseType,
                       find_data[i].DbInfo.wNumRecords);

                free(db_name);
        }

        IRAPISession_CeRapiFreeBuffer(session, find_data);

        result = TRUE;
 exit:
        return result;
}


bool
list_databases_by_enum(IRAPISession *session)
{
        HRESULT hr;
        DWORD last_error;
        char *db_name = NULL;
        unsigned i = 0;
        bool result = FALSE;
        HANDLE handle;
        DWORD dwDbaseType = 0;
        CEOID oid = 0;
	CEOIDINFO info;

        handle = IRAPISession_CeFindFirstDatabase(session, dwDbaseType);
        if (handle == INVALID_HANDLE_VALUE) {
                if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
                        fprintf(stderr, "%s: error enumerating first database: %s\n",
                                prog_name, synce_strerror(hr));
                        goto exit;
                }

                last_error = IRAPISession_CeGetLastError(session);
                fprintf(stderr, "%s: error enumerating first database: %s\n",
                        prog_name, synce_strerror(last_error));
                goto exit;
	}

	memset(&info, 0, sizeof(info));
        while ((oid = IRAPISession_CeFindNextDatabase(session, handle)) != 0) {
		if (!IRAPISession_CeOidGetInfo(session, oid, &info)) {
                        if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
                                fprintf(stderr, "%s: error getting database info: %s\n",
                                        prog_name, synce_strerror(hr));
                                goto exit;
                        }

                        last_error = IRAPISession_CeGetLastError(session);
                        fprintf(stderr, "%s: error getting database info: %s\n",
                                prog_name, synce_strerror(last_error));
                        goto exit;
                }

                if (info.wObjType != OBJTYPE_DATABASE) {
                        fprintf(stderr, "%s: object not a database, skipping...\n", prog_name);
                        continue;
                }

                db_name = wstr_to_current(info.u.infDatabase.szDbaseName);

                printf("Database %3u: \"%s\" (oid=0x%x, type=0x%x, rows=%u)\n",
                       i,
                       db_name,
                       oid,
                       info.u.infDatabase.dwDbaseType,
                       info.u.infDatabase.wNumRecords);

                free(db_name);
                i++;
        }

        if (oid == 0) {
                if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
                        fprintf(stderr, "%s: error enumerating databases: %s\n",
                                prog_name, synce_strerror(hr));
                        goto exit;
                }

                last_error = IRAPISession_CeGetLastError(session);

                if (last_error != ERROR_NO_MORE_ITEMS) {
                        fprintf(stderr, "%s: error enumerating databases: %s\n",
                                prog_name, synce_strerror(last_error));
                        goto exit;
                }
        }

        if (!IRAPISession_CeCloseHandle(session, handle)) {
                if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
                        fprintf(stderr, "%s: error closing database enumeration: %s\n",
                                prog_name, synce_strerror(hr));
                        goto exit;
                }

                last_error = IRAPISession_CeGetLastError(session);
                fprintf(stderr, "%s: error closing database enumeration: %s\n",
                        prog_name, synce_strerror(last_error));
                goto exit;
        }

        result = TRUE;
 exit:
        return result;
}

bool
read_record(IRAPISession *session, HANDLE handle)
{
        bool result = FALSE;
        HRESULT hr;
        DWORD last_error;
        uint i;
        char *tmp_str = NULL;
        CEOID rec_oid = 0;
        /* 0 = don't allow realloc of buffer */
        DWORD dwFlags = CEDB_ALLOWREALLOC;

        WORD num_props = 0;
        CEPROPID * rgPropID = NULL;
        LPBYTE buffer = malloc(4096*4096);
        DWORD lpcbBuffer = 4096*4096;
        CEPROPVAL *field = NULL;

        rec_oid = IRAPISession_CeReadRecordProps(session, handle,
                                    dwFlags, 
                                    &num_props,
                                    rgPropID,
                                    &buffer, 
                                    &lpcbBuffer 
                                    );

        if (rec_oid == 0) {
                if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
                        fprintf(stderr, "%s: error reading record: %s\n",
                                prog_name, synce_strerror(hr));
                        goto exit;
                }

                last_error = IRAPISession_CeGetLastError(session);
                fprintf(stderr, "%s: error reading record: %s\n",
                        prog_name, synce_strerror(last_error));
                goto exit;
        }

        printf("Number of properties = %d\n", num_props);

        field = (CEPROPVAL*)buffer;
        for (i = 0; i < num_props; i++) {
                printf("field %d: %s: ", i, property_type_to_str(field[i].propid));

                switch (field[i].propid & 0xffff)
                        {
                        case CEVT_BLOB:
                                printf("not yet supported\n");
                                break;
                        case CEVT_BOOL:
                                if (field[i].val.boolVal == FALSE)
                                        printf("FALSE\n");
                                else
                                        printf("TRUE\n");
                                break;
                        case CEVT_FILETIME:
                                printf("not yet supported\n");
                                break;
                        case CEVT_I2:
                                printf("%d\n", field[i].val.iVal);
                                break;
                        case CEVT_I4:
                                printf("%d\n", field[i].val.iVal);
                                break;
                        case CEVT_LPWSTR:
                                tmp_str = wstr_to_current(field[i].val.lpwstr);
                                printf("%s\n", tmp_str);
                                free(tmp_str);
                                break;
                        case CEVT_R8:
                                printf("not yet supported\n");
                                break;
                        case CEVT_UI2:
                                printf("%u\n", field[i].val.uiVal);
                                break;
                        case CEVT_UI4:
                                printf("%u\n", field[i].val.uiVal);
                                break;
                        default:
                                printf("unknown\n");
                                break;
                        }
        }

        result = TRUE;
 exit:
        IRAPISession_CeRapiFreeBuffer(session, buffer);
        return result;
}


bool
read_database(IRAPISession *session, const char *dbname, CEOID oid)
{
        bool result = FALSE;
        HRESULT hr;
        DWORD last_error;
        uint record = 1;

        HANDLE handle;
        /* 0 to use name */
        /* open by name doesn't work */
        LPWSTR lpszName = wstr_from_current(dbname);
        /* 0 if sort order not important */
        CEPROPID propid = 0;
        /* position increments after reading */
        DWORD dwFlags = CEDB_AUTOINCREMENT;

        handle = IRAPISession_CeOpenDatabase (session, &oid,
                                 lpszName,
                                 propid,
                                 dwFlags,
                                 0
                                 );

        wstr_free_string(lpszName);

        if (handle == INVALID_HANDLE_VALUE) {
                if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
                        fprintf(stderr, "%s: error opening database %s: %s\n",
                                prog_name, dbname, synce_strerror(hr));
                        goto exit;
                }

                last_error = IRAPISession_CeGetLastError(session);
                fprintf(stderr, "%s: error opening database %s: %s\n",
                        prog_name, dbname, synce_strerror(last_error));
                goto exit;
	}

        printf("opened database ceoid = 0x%x\n", oid);

        for (;;) {
                printf("record %d\n", record);
                if (!read_record(session, handle))
                        break;
                record++;
        }

        if (!IRAPISession_CeCloseHandle(session, handle)) {
                if (FAILED(hr = IRAPISession_CeRapiGetError(session))) {
                        fprintf(stderr, "%s: error closing database %s: %s\n",
                                prog_name, dbname, synce_strerror(hr));
                        goto exit;
                }

                last_error = IRAPISession_CeGetLastError(session);
                fprintf(stderr, "%s: error closing database %s: %s\n",
                        prog_name, dbname, synce_strerror(last_error));
                goto exit;
        }



        result = TRUE;
 exit:
        return result;
}


int
main(int argc, char** argv)
{
        int result = 1;
	IRAPIDesktop *desktop = NULL;
	IRAPIEnumDevices *enumdev = NULL;
	IRAPIDevice *device = NULL;
	IRAPISession *session = NULL;
	RAPI_DEVICEINFO devinfo;
        HRESULT hr;

        prog_name = argv[0];

        if (!handle_parameters(argc,argv))
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

        list_databases_by_all(session);
        list_databases_by_enum(session);

        
        read_database(session, "pmailAttachs", 0);
        
        /*
        read_database("", 0x2001558);
        */

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

