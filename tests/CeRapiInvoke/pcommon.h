#ifndef __pcommon_h__
#define __pcommon_h__

#include <synce.h>

#ifdef __cplusplus
extern "C"
{
#endif

void convert_to_backward_slashes(char* path);
bool is_remote_file(const char* filename);
WCHAR* adjust_remote_path(WCHAR* old_path, bool free_path);

struct _AnyFile;
typedef struct _AnyFile AnyFile;

typedef enum
{
	READ = 1,
	WRITE = 2
} ANYFILE_ACCESS;

AnyFile* anyfile_open(const char* filename, ANYFILE_ACCESS access);
void anyfile_close(AnyFile* file);
bool anyfile_read (AnyFile* file, unsigned char* buffer, size_t bytes, size_t* bytesAccessed);
bool anyfile_write(AnyFile* file, unsigned char* buffer, size_t bytes, size_t* bytesAccessed);

#ifdef __cplusplus
}
#endif

#endif

