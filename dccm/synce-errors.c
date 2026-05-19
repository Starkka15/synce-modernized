#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "synce-errors.h"

GQuark
synce_errors_quark (void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_static_string ("synce_errors");
  return quark;
}

