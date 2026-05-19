#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

#include "log.h"

void
log_to_syslog(const gchar *log_domain,
		    GLogLevelFlags log_level,
		    const gchar *message,
		    gpointer user_data)
{
  gint max_log_level;
  if (user_data)
    max_log_level = *(gint*)user_data;
  else
    max_log_level = 3;

  gboolean is_fatal = (log_level & G_LOG_FLAG_FATAL) != 0;
  gsize msg_len = 0;
  gchar msg_prefix[25];
  gchar *msg;
  gint priority;

  switch (log_level & G_LOG_LEVEL_MASK)
    {
    case G_LOG_LEVEL_ERROR:
      if (max_log_level < 1) return;
      strcpy (msg_prefix, "ERROR");
      priority = LOG_ERR;
      break;
    case G_LOG_LEVEL_CRITICAL:
      if (max_log_level < 2) return;
      strcpy (msg_prefix, "CRITICAL");
      priority = LOG_WARNING;
      break;
    case G_LOG_LEVEL_WARNING:
      if (max_log_level < 3) return;
      strcpy (msg_prefix, "WARNING");
      priority = LOG_WARNING;
      break;
    case G_LOG_LEVEL_MESSAGE:
      if (max_log_level < 4) return;
      strcpy (msg_prefix, "Message");
      priority = LOG_INFO;
      break;
    case G_LOG_LEVEL_INFO:
      if (max_log_level < 5) return;
      strcpy (msg_prefix, "INFO");
      priority = LOG_INFO;
      break;
    case G_LOG_LEVEL_DEBUG:
      if (max_log_level < 6) return;
      strcpy (msg_prefix, "DEBUG");
      priority = LOG_DEBUG;
      break;
    default:
      if (max_log_level < 6) return;
      strcpy (msg_prefix, "LOG");
      priority = LOG_DEBUG;
      break;
    }
  if (log_level & G_LOG_FLAG_RECURSION)
    strcat (msg_prefix, " (recursed)");
  strcat(msg_prefix, ": ");

  if (log_domain) {
    msg_len = strlen(msg_prefix) + strlen(log_domain) + 1;
    msg = malloc(msg_len + 1);
    strcpy(msg, log_domain);
    strcat(msg, "-");
  } else {
    msg_len = strlen(msg_prefix);
    msg = malloc(msg_len + 1);
    strcpy(msg, "");
  }
  strcat(msg, msg_prefix);

  if (!message) {
    msg_len = msg_len + 14;
    msg = realloc(msg, msg_len + 1);
    strcat(msg, "(NULL) message");
  } else {
    msg_len = msg_len + strlen(message);
    msg = realloc(msg, msg_len + 1);
    strcat(msg, message);
  }

  if (is_fatal) {
    msg_len = msg_len + 13;
    msg = realloc(msg, msg_len + 1);
    strcat(msg, ", aborting...");
  }

  syslog(priority, "%s", msg);
  free(msg);
}

