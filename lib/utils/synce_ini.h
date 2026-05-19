/* $Id: synce_ini.h 3983 2011-03-21 17:23:56Z mark_ellis $ */
#ifndef __synce_ini_h__
#define __synce_ini_h__

typedef struct _SynceIni SynceIni;

SynceIni* synce_ini_new(const char* filename);
void synce_ini_destroy(SynceIni* ini);

int synce_ini_get_int(SynceIni* ini, const char* section, const char* key);
const char* synce_ini_get_string(SynceIni* ini, const char* section, const char* key);

#endif

