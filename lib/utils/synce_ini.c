/* $Id: synce_ini.c 3983 2011-03-21 17:23:56Z mark_ellis $ */
#include "synce_ini.h"
#include "config/config.h"
#include <stdlib.h>

struct _SynceIni 
{
  struct configFile* cfg;
};

SynceIni* synce_ini_new(const char* filename)/*{{{*/
{
  SynceIni* ini = calloc(1, sizeof(SynceIni));
  if (!ini)
    return NULL;

  ini->cfg = readConfigFile((char*)filename);

  if (ini->cfg)
    return ini;
  
  free(ini);
  return NULL;
}/*}}}*/

void synce_ini_destroy(SynceIni* ini)/*{{{*/
{
  if (ini)
  {
    unloadConfigFile(ini->cfg);
    free(ini);
  }
}/*}}}*/

int synce_ini_get_int(SynceIni* ini, const char* section, const char* key)
{
  if (ini)
    return getConfigInt(ini->cfg, (char*)section, (char*)key);
  else
    return 0;
}

const char* synce_ini_get_string(SynceIni* ini, const char* section, const char* key)
{
  if (ini)
    return getConfigString(ini->cfg, (char*)section, (char*)key);
  else
    return NULL;
}

