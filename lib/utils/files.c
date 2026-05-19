/* $Id: files.c 3983 2011-03-21 17:23:56Z mark_ellis $ */

/* play with defines to allow snprintf() */
#undef __STRICT_ANSI__
#define _GNU_SOURCE 1
#include "synce.h"
#include "synce_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define DIRECTORY_NAME                ".synce"
#define DEFAULT_CONNECTION_FILENAME   "active_connection"
#define SCRIPT_DIRECTORY              "scripts"

static char connection_filename[MAX_PATH] = {DEFAULT_CONNECTION_FILENAME};

static bool make_sure_directory_exists(char* directory)
{
	struct stat dir_stat;

	/*
	 * Make sure that this directory exists
	 */

	if (stat(directory, &dir_stat) < 0)
	{
		if (mkdir(directory, 0700) < 0)
		{
			synce_error("Failed to create directory %s", directory);
			return false;
		}
	}

	return true;
}

/**
 * Get path to config files
 */
bool synce_get_directory(char** path)
{
  char buffer[MAX_PATH];
  char *p;
  struct passwd* user = NULL;

  if (!path)
    return false;

  /* if there is a preference for config dir set
     as an environment variable, use it */
  if ((p = getenv ("SYNCE_CONF_DIR")) != NULL) {
    if (make_sure_directory_exists (p)) {
      *path = strdup (p);
      return true;
    }
  }

  /* XXX: not very thread-safe? */
  user = getpwuid(getuid());

  *path = NULL;

  if (!user)
    return false;

  snprintf(buffer, sizeof(buffer), "%s/" DIRECTORY_NAME, user->pw_dir);

  if (!make_sure_directory_exists(buffer))
    return false;

  *path = strdup(buffer);

  return true;
}

/**
  Set the file name used for active connection info
*/
bool synce_set_connection_filename(const char* filename)
{
  bool success = false;
  
  /* disallow file names containing '..' */
  if (filename && !strstr(filename, ".."))
  {
    /* Use snprintf to limit length and assure string is terminated with 0 */
    int n = snprintf(connection_filename, sizeof(connection_filename), "%s", filename);

    /* Return false if file name was too long or some other error occured */
    success = (n >= 0 && n < (int)sizeof(connection_filename));
  }

  if (!success)
    synce_warning("Invalid filename: '%s'", filename);

  return success;
}

/*
   Restore the default filename used for active connection info
 */
bool synce_set_default_connection_filename()
{
  return synce_set_connection_filename(DEFAULT_CONNECTION_FILENAME);
}

/**
 * Get file name for active connection info
 */
bool synce_get_connection_filename(char** filename)
{
	bool success = false;
	char* path = NULL;
	char buffer[MAX_PATH];

	if (!filename)
		goto exit;

	*filename = NULL;
	
	if (!synce_get_directory(&path))
		goto exit;

	snprintf(buffer, sizeof(buffer), "%s/%s" , path, connection_filename);
	*filename = strdup(buffer);

	success = true;

exit:
	if (path)
		free(path);
	return success;
}

bool synce_get_subdirectory(const char* name, char** directory)
{
	bool success = false;
	char* path = NULL;
	char buffer[MAX_PATH];

	if (!name || !directory)
		goto exit;

	if (strchr(name, '/'))	/* prevent bad names */
		goto exit;

	*directory = NULL;
	
	if (!synce_get_directory(&path))
		goto exit;

	snprintf(buffer, sizeof(buffer), "%s/%s", path, name);

	if (!make_sure_directory_exists(buffer))
		goto exit;
	
	*directory = strdup(buffer);

	success = true;

exit:
	if (path)
		free(path);
	return success;
}

bool synce_get_script_directory(char** directory)
{
	return synce_get_subdirectory(SCRIPT_DIRECTORY, directory);
}


