/* $Id: synce-registry.c 4136 2012-11-22 15:10:51Z mark_ellis $ */
#include <rapi2.h>
#include <synce_log.h>
#include <pcommon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define ACTION_READVAL 0
#define ACTION_WRITEVAL 1
#define ACTION_DELETEVAL 2
#define ACTION_LISTKEY 3
#define ACTION_NEWKEY 4
#define ACTION_DELETEKEY 5
#define ACTION_DUMP_REGISTRY 6


char* dev_name = NULL;
int action = ACTION_READVAL;
bool list_recurse = false ; 
const char *prog_name;

#define STR_EQUAL(a,b)  (0 == strcasecmp(a,b))



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

const char * value_type_to_str(DWORD value_type)
{
        switch (value_type)
                {
                case REG_NONE:
                        return "REG_NONE";
                        break;
                case REG_SZ:
                        return "REG_SZ";
                        break;
                case REG_EXPAND_SZ:
                        return "REG_EXPAND_SZ";
                        break;
                case REG_BINARY:
                        return "REG_BINARY";
                        break;
                case REG_DWORD:
                        return "REG_DWORD";
                        break;
                case REG_DWORD_BIG_ENDIAN:
                        return "REG_DWORD_BIG_ENDIAN";
                        break;
                case REG_LINK:
                        return "REG_LINK";
                        break;
                case REG_MULTI_SZ:
                        return "REG_MULTI_SZ";
                        break;
                default:
                        break;
                }
        return "Unknown type";
}

static bool handle_parameters(int argc, char** argv, char** parent_str, char** key_name, char** value_name, LPDWORD value_type, char **new_value)
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

                        case 'L':
                                list_recurse = true ;
                                break ;
                        case 'p':
                                dev_name = optarg;
                                break;
			
                        case 'r':
                                action = ACTION_READVAL;
                                break;

                        case 'w':
                                action = ACTION_WRITEVAL;
                                break;

                        case 'x':
                                action = ACTION_DELETEVAL;
                                break;

                        case 'l':
                                action = ACTION_LISTKEY;
                                break;

                        case 'n':
                                action = ACTION_NEWKEY;
                                break;

                        case 'X':
                                action = ACTION_DELETEKEY;
                                break;

                        case 'D':
                                action = ACTION_DUMP_REGISTRY;
                                break;

                        case 't':
                                if (strcasecmp(optarg,"binary") == 0)
                                {
                                  *value_type = REG_BINARY;
                                }
                                else if (strcasecmp(optarg,"dword")==0 || strcasecmp(optarg,"dword_little_endian") == 0)
                                {
                                  *value_type = REG_DWORD;
                                }
                                else if (strcasecmp(optarg,"dword_bige") == 0)
                                {
                                  *value_type = REG_DWORD_BIG_ENDIAN;
                                }
                                else if (strcasecmp(optarg,"expand_sz") == 0)
                                {
                                  *value_type = REG_EXPAND_SZ;
                                }
                                else if (strcasecmp(optarg,"multi_sz") == 0)
                                {
                                  *value_type = REG_MULTI_SZ;
                                }
                                else if (strcasecmp(optarg,"sz") == 0)
                                {
                                  *value_type = REG_SZ;
                                }
                                else
                                {
                                  fprintf(stderr,"%s: Unrecognized value type '%s' for argument -t\n",argv[0],optarg);
                                  return false;
                                }
                                break;

			case 'h':
			default:
				show_usage(argv[0]);
				return false;
		}
	}
				
	synce_log_set_level(log_level);

        argc -= optind;
        switch(action)
        {
          case ACTION_READVAL:
          case ACTION_DELETEVAL:
            if (argc != 3)
            {
              fprintf(stderr,"Wrong number of parameters\n");
              show_usage(argv[0]);
              return false;
            }
            break;
          case ACTION_WRITEVAL:
            if (argc != 4)
            {
              fprintf(stderr,"Wrong number of parameters\n");
              show_usage(argv[0]);
              return false;
            }
            break;
          case ACTION_DELETEKEY:
          case ACTION_LISTKEY:
            if (argc != 2)
            {
              fprintf(stderr,"Wrong number of parameters\n");
              show_usage(argv[0]);
              return false;
            }
            break;
        }


        switch(argc)
        {
          case 4:
            if ((*new_value = strdup(argv[optind+3])) == NULL)
              return false;
            /* fall through */
          case 3:
            if ((*value_name = strdup(argv[optind+2])) == NULL)
              return false;
            /* fall through */
          case 2:
            if ((*key_name = strdup(argv[optind+1])) == NULL)
              return false;
            /* fall through */
          case 1:
            if ((*parent_str = strdup(argv[optind])) == NULL)
              return false;
        }

	return true;
}

int delete_key(IRAPISession *session, HKEY parent, const char *key_name)
{
  int result = 1;
  LONG retval;
  LPWSTR key_name_wide = NULL;

  key_name_wide = wstr_from_current(key_name);
  if (!key_name_wide) {
          fprintf(stderr, "%s: Failed to convert registry key '%s' from current encoding to UCS2\n", prog_name, key_name);
          return result;
  }

  retval = IRAPISession_CeRegDeleteKey(session, parent, key_name_wide);
  wstr_free_string(key_name_wide);

  if (ERROR_SUCCESS != retval)
  {
          fprintf(stderr,"Failed to delete key '%s': %s\n",
                  key_name,
                  synce_strerror(retval));
          return result;
  }

  result = 0;
  return result;
}

int new_key(IRAPISession *session, HKEY parent, const char *key_name)
{
  int result = 1;
  LONG retval;
  HKEY new_key;
  DWORD new_disposition;
  LPWSTR key_name_wide = NULL;

  key_name_wide = wstr_from_current(key_name);
  if (!key_name_wide) {
          fprintf(stderr, "%s: Failed to convert registry key '%s' from current encoding to UCS2\n", prog_name, key_name);
          return result;
  }

  retval = IRAPISession_CeRegCreateKeyEx(session, parent, key_name_wide, 0, 0, 0, 0, NULL, &new_key, &new_disposition);
  wstr_free_string(key_name_wide);
  if (ERROR_SUCCESS != retval)
  {
    fprintf(stderr,"Failed to create key '%s': %s\n",
            key_name,
            synce_strerror(retval));
    return result;
  }

  if (ERROR_SUCCESS != (retval = IRAPISession_CeRegCloseKey(session, new_key)))
  {
    fprintf(stderr,"Failed to close key '%s': %s\n",
            key_name,
            synce_strerror(retval));
    return result;
  }

  if (new_disposition != REG_CREATED_NEW_KEY)
  {
    fprintf(stderr,"Key '%s' already exists\n",
            key_name);
    return result;
  }

  result = 0;
  return result;
}

void
print_multi_sz(const LPBYTE value, DWORD value_size)
{
        LPWSTR next_str_w = NULL;
        char *next_str = NULL;
        size_t pos = 0;

        next_str_w = (LPWSTR)value;

        while(TRUE) {
                next_str = wstr_to_current(next_str_w);
                if (!next_str) {
                        fprintf(stderr, "%s: Failed to convert registry multi string value to current encoding\n", prog_name);
                        break;
                }
                printf("\"%s\" ", next_str);

                pos = pos + strlen(next_str) + 1;
                next_str_w = next_str_w + strlen(next_str) + 1;
                free(next_str);

                if (*next_str_w == 0)
                        break;
        }
        printf("\n");
}

void print_value(const char *value_name, DWORD value_type, const LPBYTE value, DWORD value_size)
{
  char *value_str = NULL;

  printf("\"%s\"{%s} = ", value_name, value_type_to_str(value_type));

  switch (value_type)
          {
          case REG_NONE:
                  dump("REG_NONE", value, value_size);
                  break;

          case REG_SZ:
          case REG_EXPAND_SZ:
                  value_str = wstr_to_current((LPCWSTR)value);
                  if (!value_str) {
                          fprintf(stderr, "%s: Failed to convert registry string value to current encoding\n", prog_name);
                          value_str = strdup("");
                  }
                  printf("\"%s\"\n", value_str);
                  free(value_str);
                  break;

          case REG_BINARY:
                  dump("REG_BINARY", value, value_size);
                  break;

          case REG_DWORD:
                  printf("%08x\n", letoh32(*(LPDWORD)value));
                  break;

          case REG_DWORD_BIG_ENDIAN:
                  printf("%08x\n", betoh32(*(LPDWORD)value));
                  break;

          case REG_LINK:
                  dump("REG_LINK", value, value_size);
                  break;

          case REG_MULTI_SZ:
                  print_multi_sz(value, value_size);
                  break;

          default:
                  dump("Value", value, value_size);
                  break;
          }
}


int list_key(IRAPISession *session, HKEY key, const char* key_path, bool do_recurse) 
{ 
  int result = 1;
  int i;
  char *key_name = NULL;
  char *value_name = NULL;
  LONG retval;

  DWORD lpcSubKeys;
  DWORD lpcbMaxSubKeyLen;
  DWORD lpcbMaxClassLen;
  DWORD lpcValues;
  DWORD lpcbMaxValueNameLen;
  DWORD lpcbMaxValueLen;

  DWORD value_name_wide_size;
  LPWSTR value_name_wide = NULL;
  DWORD value_size;
  LPBYTE value = NULL;
  DWORD value_type;
  LPWSTR key_name_wide = NULL;
  DWORD key_name_size;
  HKEY childKey = 0;
  char *child_key_path = NULL;

  /* First print the path till now: */
  printf("\n[%s]\n", key_path );

  /* determine the size of the holding arrays for all 
     subvalues/keys */

  result = IRAPISession_CeRegQueryInfoKey(session, key, NULL, NULL, NULL, 
		  &lpcSubKeys, &lpcbMaxSubKeyLen, &lpcbMaxClassLen, 
		  &lpcValues, &lpcbMaxValueNameLen, &lpcbMaxValueLen, 
		  NULL, NULL) ;

  /* One important thing is to add +1 to all string lengths, because the
     lengths obtained do not include the NULL terminator.
     This does not include strings stored as a data component, the lengths
     of which do include the NULL */

  lpcbMaxClassLen++;
  lpcbMaxValueNameLen++;
  lpcbMaxSubKeyLen++;

  value_name_wide = malloc(sizeof(WCHAR) * lpcbMaxValueNameLen);
  value = malloc(sizeof(BYTE) * lpcbMaxValueLen);

  /* print all of the values */
  for(i = 0; ; i++)
  {
          value_name_wide_size = lpcbMaxValueNameLen;
          value_size = lpcbMaxValueLen;
          memset(value, 0, value_size);

          retval = IRAPISession_CeRegEnumValue(session, key, i, value_name_wide,
                                  &value_name_wide_size, NULL, &value_type,
                                  value, &value_size);

          if (ERROR_NO_MORE_ITEMS == retval)
                  break;

          if (ERROR_SUCCESS != retval) {
                  fprintf(stderr,"Failed to list value #%d: %s\n",
                          i, synce_strerror(retval));
                  goto exit;
          }

          value_name = wstr_to_current(value_name_wide);
          if (!value_name) {
                  fprintf(stderr, "Failed to convert registry value #%d name to current encoding\n", i);
                  continue;
          }
          print_value(value_name, value_type, value, value_size);
          free(value_name);
  }

  key_name_wide = malloc(sizeof(WCHAR) * lpcbMaxSubKeyLen);
  
  for(i = 0; ; i++)
  {
          key_name_size = lpcbMaxSubKeyLen;

          retval = IRAPISession_CeRegEnumKeyEx(session, key, i, key_name_wide, &key_name_size,
                                  NULL, NULL, NULL, NULL);
          if (ERROR_NO_MORE_ITEMS == retval)
                  break;

          if (ERROR_SUCCESS != retval) {
                  fprintf(stderr,"Failed to list subkey #%d: %s\n",
                          i, synce_strerror(retval));
                  goto exit;
          }

          key_name = wstr_to_current(key_name_wide);
          if (!key_name) {
                  fprintf(stderr, "Failed to convert registry key name to current encoding, skipping\n");
                  continue;
          }

          if (!do_recurse){
                  /* then just print all the subkeys */

                  fprintf(stdout, "\n[%s\\%s]\n", key_path, key_name); 
          } else {
                  childKey = 0 ; 

                  child_key_path = malloc(strlen(key_path) + strlen(key_name) + 2);
                  sprintf(child_key_path,"%s\\%s", key_path, key_name);

                  if ((retval = IRAPISession_CeRegOpenKeyEx(session, key, key_name_wide, 0, 0, &childKey)) == ERROR_SUCCESS) {
                          /* We have childkey now
                             Do list_key on this */
			  list_key(session, childKey, child_key_path, do_recurse);
		  } else {
                          fprintf(stderr, "Failed to open registry sub key '%s', skipping\n", key_name);
                  }
          }
          free(key_name);
  }

  result = 0;

 exit:
  if (value_name_wide)
          free(value_name_wide);
  if (value)
          free(value);
  if (key_name_wide)
          free(key_name_wide);

  return result;
}


int dump_registry(IRAPISession *session)
{
  int result = 0;

  /* First the HKLM */
  result = list_key(session, HKEY_LOCAL_MACHINE, "HKEY_LOCAL_MACHINE", true);
  if (result != 0) {
  	return result;
  }

  /* Then the HKCU */
  result = list_key(session, HKEY_CURRENT_USER, "HKEY_CURRENT_USER", true );
  if (result != 0) {
  	return result;
  }

  /* And finally the HKCR */
  result = list_key(session, HKEY_CLASSES_ROOT, "HKEY_CLASSES_ROOT", true );

  return result;
}


int delete_val(IRAPISession *session, HKEY key, const char *value_name)
{
  int result = 1;
  LONG retval;
  LPWSTR value_name_wide = NULL;

  value_name_wide = wstr_from_current(value_name);
  if (!value_name_wide) {
          fprintf(stderr, "%s: Failed to convert registry value name '%s' from current encoding to UCS2\n", prog_name, value_name);
          return result;
  }

  retval = IRAPISession_CeRegDeleteValue(session, key, value_name_wide);
  if (ERROR_SUCCESS != retval)
  {
    fprintf(stderr,"Failed to delete value '%s': %s\n",
            value_name,
            synce_strerror(retval));
    return result;
  }

  result = 0;
  return result;
}


int read_val(IRAPISession *session, const char *parent_str, const char *key_str, HKEY key, const char *value_name)
{
  int result = 1;
  LONG retval;
  DWORD value_size = 0;
  LPBYTE value = NULL;
  DWORD value_type;
  LPWSTR value_name_wide = NULL;

  value_name_wide = wstr_from_current(value_name);
  if (!value_name_wide) {
          fprintf(stderr, "%s: Failed to convert registry value name '%s' from current encoding to UCS2\n", prog_name, value_name);
          goto exit;
  }

  retval = IRAPISession_CeRegQueryValueEx(session, key, value_name_wide, NULL, NULL, NULL, &value_size);
  if (ERROR_SUCCESS != retval) {
    fprintf(stderr, "%s: Failed to get size of value '%s\\%s\\%s': %s\n", 
            prog_name, parent_str, key_str, value_name,
            synce_strerror(retval));
    goto exit;
  }

  value = calloc(1, value_size);
  if (!value) {
          fprintf(stderr, "%s: Failed to allocate %i bytes for value\n", prog_name, value_size);
          goto exit;
  }

  retval = IRAPISession_CeRegQueryValueEx(session, key, value_name_wide, NULL, &value_type, value, &value_size);
  if (ERROR_SUCCESS != retval) {
    fprintf(stderr, "%s: Failed to get value '%s\\%s\\%s': %s\n", 
            prog_name, parent_str, key_str, value_name,
            synce_strerror(retval));
    goto exit;
  }

  printf("[%s\\%s]\n", parent_str, key_str);

  print_value(value_name, value_type, value, value_size);

  result = 0;
 exit:
  if (value_name_wide)
          wstr_free_string(value_name_wide);
  if (value)
          free(value);

  return result;
}


int write_val(IRAPISession *session, HKEY key, const char *value_name, DWORD value_type, const char *new_value)
{
  LONG retval;
  void *valBuf = NULL;
  int valSize;
  DWORD dwordVal;
  int success = 1;
  LPWSTR value_name_wide = NULL;

  value_name_wide = wstr_from_current(value_name);
  if (!value_name_wide) {
          fprintf(stderr, "%s: Failed to convert registry value name '%s' from current encoding to UCS2\n", prog_name, value_name);
          goto exit;
  }

  switch(value_type)
  {
    case REG_NONE:
      fprintf(stderr,"REG_NONE not yet supported.\n");
      goto exit;

    case REG_SZ:
    case REG_EXPAND_SZ:
      valBuf = wstr_from_current(new_value);
      if (!valBuf) {
              fprintf(stderr, "Failed to convert registry string value from current encoding to UCS2\n");
              goto exit;
      }

      valSize = (wstrlen(valBuf)+1) * sizeof(WCHAR);
      break;

    case REG_BINARY:
      fprintf(stderr,"REG_BINARY not yet supported.\n");
      goto exit;

    case REG_DWORD:
      dwordVal = strtol(new_value,NULL,0);
      dwordVal = htole32(dwordVal);
      valBuf = &dwordVal;
      valSize = sizeof(DWORD);
      break;

    case REG_DWORD_BIG_ENDIAN:
      dwordVal = strtol(new_value,NULL,0);
      dwordVal = htobe32(dwordVal);
      valBuf = &dwordVal;
      valSize = sizeof(DWORD);
      break;

    case REG_LINK:
      fprintf(stderr,"REG_LINK not yet supported.\n");
      goto exit;

    case REG_MULTI_SZ:
      fprintf(stderr,"REG_MULTI_SZ not yet supported.\n");
      goto exit;

    default:
      fprintf(stderr,"Unrecognized value type somehow!\n");
      goto exit;
  }

  retval = IRAPISession_CeRegSetValueEx(session, key, value_name_wide, 0, value_type, valBuf, valSize);
  if (ERROR_SUCCESS != retval) {
          fprintf(stderr, "%s: Unable to set value of '%s' to '%s' value: %s\n", 
                  prog_name, value_name, value_type_to_str(value_type),
                  synce_strerror(retval));
  } else {
          success = 0;
  }

 exit:
  if (value_name_wide)
          wstr_free_string(value_name_wide);

  switch(value_type)
  {
    case REG_SZ:
    case REG_EXPAND_SZ:
      wstr_free_string(valBuf);
      break;
  }

  return success;
}

int main(int argc, char** argv)
{
  int result = 1;
  IRAPIDesktop *desktop = NULL;
  IRAPIEnumDevices *enumdev = NULL;
  IRAPIDevice *device = NULL;
  IRAPISession *session = NULL;
  RAPI_DEVICEINFO devinfo;
  char* parent_str  = NULL;
  char* key_name          = NULL;
  char* value_name  = NULL;
  char* new_value   = NULL;
  WCHAR* key_name_wide = NULL;
  HKEY parent;
  HKEY key = 0;
  HRESULT hr;
  DWORD value_type = REG_SZ;
  LONG retval;

  prog_name = argv[0];
  
  if (!handle_parameters(argc,argv,&parent_str,&key_name,&value_name,&value_type,&new_value))
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

  /* Add this before anything, since we don't need the
     parent_str etc.. */
  if (action==ACTION_DUMP_REGISTRY){
	  result = dump_registry(session) ;
	  if (result != 0){
		  fprintf(stderr, "%s: Registry dump failed...\n", prog_name); 
	  }
	  goto exit;
  }

  /* handle abbreviations */
  if (STR_EQUAL(parent_str, "HKCR"))
    parent_str = "HKEY_CLASSES_ROOT";
  else if (STR_EQUAL(parent_str, "HKCU"))
    parent_str = "HKEY_CURRENT_USER";
  else if (STR_EQUAL(parent_str, "HKLM"))
    parent_str = "HKEY_LOCAL_MACHINE";
  else if (STR_EQUAL(parent_str, "HKU"))
    parent_str = "HKEY_USERS";

  if (STR_EQUAL(parent_str, "HKEY_CLASSES_ROOT"))
    parent = HKEY_CLASSES_ROOT;
  else if (STR_EQUAL(parent_str, "HKEY_CURRENT_USER"))
    parent = HKEY_CURRENT_USER;
  else if (STR_EQUAL(parent_str, "HKEY_LOCAL_MACHINE"))
    parent = HKEY_LOCAL_MACHINE;
  else if (STR_EQUAL(parent_str, "HKEY_USERS"))
    parent = HKEY_USERS;
  else
  {
    fprintf(stderr, "Invalid parent key\n");
    goto exit;
  }


  convert_to_backward_slashes(key_name);


  if (action == ACTION_DELETEKEY)
  {
    result = delete_key(session, parent, key_name);
    goto exit;
  }
  else if (action == ACTION_NEWKEY)
  {
    result = new_key(session, parent, key_name);
    goto exit;
  }




  key_name_wide = wstr_from_current(key_name);
  if (!key_name_wide)
  {
    fprintf(stderr, "%s: Failed to convert registry key name '%s' from current encoding to UCS2\n", prog_name, key_name);
    goto exit;
  }
  retval = IRAPISession_CeRegOpenKeyEx(session, parent, key_name_wide, 0, 0, &key);

  wstr_free_string(key_name_wide);

  if (retval != ERROR_SUCCESS)
  {
    fprintf(stderr, "Failed to open key: '%s\\%s'\n", parent_str, key_name);
    goto exit;
  }

  if (action == ACTION_LISTKEY)
  {
    char *path = NULL;
    path = malloc(strlen(parent_str) + strlen(key_name) + 2);

    sprintf(path, "%s\\%s", parent_str, key_name); 
    result = list_key(session, key, path, list_recurse);
    free(path);
    goto exit;
  }

  if (action == ACTION_DELETEVAL)
  {
    result = delete_val(session, key, value_name);
  }
  else if (action == ACTION_READVAL)
  {
    result = read_val(session, parent_str, key_name, key, value_name);
  }
  else if (action == ACTION_WRITEVAL)
  {
    result = write_val(session, key, value_name, value_type, new_value);
  }
  
exit:
  if (key)
    IRAPISession_CeRegCloseKey(session, key);

  if (session)
  {
    IRAPISession_CeRapiUninit(session);
    IRAPISession_Release(session);
  }

  if (device) IRAPIDevice_Release(device);
  if (enumdev) IRAPIEnumDevices_Release(enumdev);
  if (desktop) IRAPIDesktop_Release(desktop);

  if (key_name)
    free(key_name);
  return result;
}

