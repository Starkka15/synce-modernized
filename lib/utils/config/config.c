/*
Config version 1.0 by Nicholas FitzRoy-Dale, incorporating public-domain code
by Jerry Coffin and HenkJan Wolthuis. Parse a Windows 3.11 (".ini")-style 
configuration file.

This code is in the public domain. Do whatever you want with it. 

Very quick intro:

Config file format:
- - - 8< - - -
[Section]
Key1: value1
someOtherKey: anotherValue

[Section 2]
# This is a very important key.
VIKey: Ahar! #Note: very important.

(..etc)
- - - >8 - - -

All whitespace (tab and space characters) after the ':' but before the value
is ignored, as is all whitespace after the key but before the ':'. Keys are
case-insensitive and in fact are converted to lower-case for storage. Values
are case-sensitive and may contain any character except a newline.

Other stuff:
	* Start single-line comments using a #. Comments can be at the start
	  of a line, or following a value.
	* If you don't specify an initial section, keys are stored in a section
	  called "DEFAULT".
	* You can use an equal sign to separate keys and values instead of a colon
	  if you prefer. (key=value or key: value)
	* Keys are case-insensitive, as are section names.

To use:

- - - 8< - - -
struct configFile *myConfig;
int someIntValue;
char *someStringValue;

myConfig = readConfigFile ('programrc')
if (!myConfig) {
	( die horribly )
}
someStringValue=getConfigString (myConfig, "trilogy", "thequestion");
if (!someStringValue) {
	( die horribly )
}
someIntValue=getConfigInt (myConfig, "trilogy", "theanswer");
unloadConfigFile (myConfig)
- - - >8 - - -

Don't attempt to read from someStringValue after you have unloaded the config
file. Note that if a given integer value is not present in the config file, 
getConfigInt returns the integer 0. IE, it's impossible to tell whether that
integer 0 was the result of a missing key, or whether the key was present and the
value was 0. If you want to avoid this, get a string value, check the string, and
then use atoi(3).

To get an idea of the internal representation of the config file, call
_cfgdbgPrintConfigFile(struct configFile *cfg). 


11/Feb/2001: Extended to provide full Windows 3.11 .ini-file emulation, with
[Section Headers]. Also support comments at the start of a line and after a key.
White space after a value is now stripped.

Everything is now stored in a hash table, which is great because I can now be
really lazy and also moderately efficient. 

	I am a computer and I dance like metronome
	Counting down to zero, clockwork punk!
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>

#include "config.h"

#define myisblank(c)   ((c) == ' ' || (c) == '\t')

/* Attempt to read and parse a config file. Return a handle to the config file
 * structure, or NULL if errors were encountered.
*/
struct configFile *readConfigFile (char *filename)
{
	/* TODO: Attempt to read from homedir, /etc, etc if file not found in cwd. */
	
	struct stat filestats;
	struct configFile *cfg;
	int fileHandle;
	ssize_t bytesRead;
		
	if (stat (filename, &filestats) != 0) {
/*		perror ("ReadConfigFile: stat");*/
		return NULL;
	}
	if (filestats.st_size > MAXIMUM_SANE_FILESIZE) 
		return NULL;
	
	cfg = malloc (sizeof (struct configFile));
	cfg->bbdg = malloc (filestats.st_size);
	cfg->sections = hashConstructTable (31); /* Some prime, just for laughs */
	
	if ( (fileHandle=open (filename, O_RDONLY)) == -1) {
/*		perror ("ReadConfigFile: open");*/
		return NULL;
	}
	if ( (bytesRead=read (fileHandle, cfg->bbdg, filestats.st_size))
			!= filestats.st_size) {
/*		perror ("ReadConfigFile: read");*/
		return NULL;
	}
	close (fileHandle);
	cfg->bbdgSize=filestats.st_size;

	return _cfgParseConfigFile (cfg);
}

static inline int isKeyValSep (char c) {
	return (c==':' || c=='=');
}

static inline int isCommentStart (char c) {
	return (c=='#');
}

/* Small FSM. States indicate what the machine is looking for *next*, 
 * so eg _cfgKEYSTART means "looking for the token that indicates the start
 * of a key"
*/
struct configFile *_cfgParseConfigFile (struct configFile *cfg)
{
	char *currentSectionString="DEFAULT";
	char *currentStringStart=NULL;
	char *currentKey=NULL;
	unsigned int filePos=0, state=_cfgKEYSTART;
	hash_table *tempHash;
	
	/* Create the default section. */
	tempHash=hashConstructTable (31);
	hashInsert (currentSectionString, tempHash, cfg->sections);
	
	while (filePos < cfg->bbdgSize) {
		switch (state) {
			case _cfgKEYSTART:
				if (cfg->bbdg[filePos]=='[') {
					filePos++;
					currentStringStart = (char*) &(cfg->bbdg[filePos]);
					state=_cfgSECTIONEND;
					break;
				}
				if (isCommentStart(cfg->bbdg[filePos])) {
					filePos++;
					state=_cfgCOMMENTEND;
					break;
				}
				if ( !isspace ((char) cfg->bbdg[filePos]) ) {
					currentStringStart = (char*) &(cfg->bbdg[filePos]);
					state=_cfgKEYEND;
				} else {
					filePos ++;
				}
				break;
			case _cfgCOMMENTEND:
				if (cfg->bbdg[filePos]=='\n') {
					state=_cfgKEYSTART;
				}
				filePos++;
				break;
			case _cfgSECTIONEND:
				if (cfg->bbdg[filePos]==']') {
					cfg->bbdg[filePos]='\0';
					currentSectionString=currentStringStart;
					state=_cfgKEYSTART;
				}
				filePos++;
				break;
			case _cfgKEYEND:
				if (isspace (cfg->bbdg[filePos]) || isKeyValSep(cfg->bbdg[filePos])) {
					if (isKeyValSep(cfg->bbdg[filePos])) {
						cfg->bbdg[filePos]='\0';
					} else {
						cfg->bbdg[filePos]='\0';
						filePos++;
					}
					currentKey=currentStringStart;
					state=_cfgCOLON;
				} else {
					//Do this in search routine instead (with strcasecmp)
					//cfg->bbdg[filePos] = tolower(cfg->bbdg[filePos]);
					filePos++;
				}
				break;
			case _cfgCOLON:
				if (isKeyValSep(cfg->bbdg[filePos]) || cfg->bbdg[filePos]=='\0') {
					state=_cfgVALSTART;
				}
				filePos++;
				break;
			case _cfgVALSTART:
				if (!myisblank(cfg->bbdg[filePos])) {
					currentStringStart = (char*) &(cfg->bbdg[filePos]);
					state=_cfgVALEND;
				} else {
					filePos ++;
				}
				break;
			case _cfgVALEND:
				if (cfg->bbdg[filePos]=='\n' || isCommentStart(cfg->bbdg[filePos])) {
					/* First see if the current section exists. */
					tempHash=hashLookup (currentSectionString, cfg->sections);
					if (tempHash==NULL) {
						tempHash=hashConstructTable (31);
						hashInsert (currentSectionString, tempHash, cfg->sections);
					}
					/* Now stick it in the table. */
					if (isCommentStart(cfg->bbdg[filePos])) {
						cfg->bbdg[filePos]='\0';
						hashInsert (currentKey, currentStringStart, tempHash);
						state=_cfgCOMMENTEND;
					} else {
						cfg->bbdg[filePos]='\0';
						hashInsert (currentKey, currentStringStart, tempHash);
						state=_cfgKEYSTART;
					}
				}
				filePos++;
				break;
		}
		
	}
	return cfg;
}

char *getConfigString (struct configFile *cfg, char *section, char *key)
{
	struct hash_table *hashSection;
	hashSection=hashLookup (section, cfg->sections);
	if (!hashSection) return NULL;
	return hashLookup (key, hashSection);
}

int getConfigInt (struct configFile *cfg, char *section, char *key)
{
	char *configString;
	if ( (configString=getConfigString(cfg, section, key))==NULL) {
		return 0;
	}
	return atoi (configString);
}

double getConfigDouble (struct configFile *cfg, char *section, char *key)
{
	char *configString;
	if ( (configString=getConfigString(cfg, section, key))==NULL) {
		return 0;
	}
	return atof (configString);
}

void _cfgdbgPrintConfigDatum (char *key, void *data)
{
	printf ("\t\t%s:%s\n", key, (const char*)data);
}

void _cfgdbgPrintConfigSection (char *key, void *section)
{
	printf ("\tConfig section %s:\n", key);
	hashEnumerate ((hash_table *)section, _cfgdbgPrintConfigDatum);
}

void _cfgdbgPrintConfigFile(struct configFile *cfg)
{
	printf ("Config file:\n");
	hashEnumerate (cfg->sections, _cfgdbgPrintConfigSection);
}

/* This function called by hashFreeTable, itself called from unloadConfigFile.
 * Memory-based data structure of config file is a hash table (config sections)
 * of hash tables (config options). This function frees the config options
 * hashtables as each config section is freed.
*/
void unloadConfigSection (void *data)
{
	struct hash_table *sectionTable = data;
	hashFreeTable (sectionTable, NULL);
}

void unloadConfigFile (struct configFile *cfg)
{
	if (!cfg)
		return;
	hashFreeTable (cfg->sections, unloadConfigSection);
	free(cfg->bbdg);
	free(cfg);
}
