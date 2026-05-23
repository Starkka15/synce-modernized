// $Id: test.h 1489 2003-12-22 14:12:05Z twogood $
extern "C" {
#include <stdio.h>
}

#include "rapi.h"

using namespace synce;

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#endif

//
// Return values from main()
//
#define TEST_SUCCEEDED 0
#define TEST_FAILED 1

//
// HRESULT tests
// 
#define VERIFY_HRESULT(call) \
if (FAILED((call))) { printf("FAIL.\n"); return TEST_FAILED; }

#define TEST_HRESULT(call) \
printf("Testing %s...", #call); \
VERIFY_HRESULT(call) else printf("ok.\n");

//
// Test return value
//
#define VERIFY_EQUAL(value, call) \
if ((value) != (call)) { printf("FAIL.\n"); return TEST_FAILED; }

#define TEST_EQUAL(value, call) \
printf("Testing %s...", #call); \
VERIFY_EQUAL((value), (call)) else printf("ok.\n");


//
// Test return value
//
#define VERIFY_NOT_EQUAL(value, call) \
if ((value) == (call)) { printf("FAIL.\n"); return TEST_FAILED; }

#define TEST_NOT_EQUAL(value, call) \
printf("Testing %s...", #call); \
VERIFY_NOT_EQUAL((value), (call)) else printf("ok.\n");


//
// Test to verify that the return value is zero
//
#define VERIFY_NOT_FALSE(call) VERIFY_NOT_EQUAL(0, (call))
#define TEST_NOT_FALSE(call) TEST_NOT_EQUAL(0, call)


//
// Convert text
//
char* from_unicode(const WCHAR* inbuf)
{
	return wstr_to_ascii(inbuf);
}

WCHAR* to_unicode(const char* inbuf)
{
	return wstr_from_ascii(inbuf);
}

#if 0
// This does not work with Linux kernel 2.2 and earlier
bool is_valid_ptr(void * ptr)
{
	size_t pagesize = getpagesize();
#if __FreeBSD__
	char vec;
#else
	unsigned char vec;
#endif
	
	// align pointer
	ptr = (void*)((unsigned)ptr & ~(pagesize-1));

	int result = mincore(ptr, 1, &vec);
	if (0 != result)
	{
		printf("mincore failed: %s\n", strerror(errno));
		return false;
	}

	return true;
}
#endif
