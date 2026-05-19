/* $Id */
#define _BSD_SOURCE 1
#include "synce_log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

/* evil static data */
static int current_log_level = SYNCE_LOG_LEVEL_DEFAULT;
static bool use_syslog = false;

static int level_to_priority[] =
{
  0,
  LOG_ERR,
  LOG_WARNING,
  LOG_INFO,
  LOG_DEBUG
};


void synce_log_set_level(int level)
{
	current_log_level = level;
}

void synce_log_use_syslog()
{
  use_syslog = true;
}

void _synce_log(int level, const char* file, int line, const char* format, ...)
{
  va_list ap;

  if (level > current_log_level)
    return;

  if (use_syslog)
  {
    /* Do not log function name and line to syslog */
    va_start(ap, format);
    vsyslog(level_to_priority[level], format, ap);
    va_end(ap);
  }
  else
  {
    fprintf(stderr, "[%s:%i] ", file, line);

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fprintf(stderr, "\n");
  }
}

void _synce_log_wstr(int level, const char* file, int line, const char* name,
		const WCHAR* wstr)
{
  if (level <= current_log_level)
  {
    char* str = wstr_to_current(wstr);
    if (!str)
            str = strdup("");

    fprintf(stderr, "[%s:%i] %s=\"%s\"\n", file, line, name, str);

    wstr_free_string(str);
  }
}

