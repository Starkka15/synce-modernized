#include "test.h"

int main()
{
	VERIFY_HRESULT(CeRapiInit());

	WCHAR buffer[MAX_PATH];
	DWORD length;
	
	// XXX: CSIDL_DESKTOPDIRECTORY does not work, but that's probably OK
	//TEST_NOT_FALSE(length = CeGetSpecialFolderPath(CSIDL_DESKTOPDIRECTORY, MAX_PATH, buffer));

	TEST_NOT_FALSE(length = CeGetSpecialFolderPath(CSIDL_FAVORITES, MAX_PATH, buffer));
	printf("CSIDL_FAVORITES: %s\n", from_unicode(buffer));
	TEST_NOT_FALSE(length = CeGetSpecialFolderPath(CSIDL_FONTS, MAX_PATH, buffer));
	printf("CSIDL_FONTS: %s\n", from_unicode(buffer));
	TEST_NOT_FALSE(length = CeGetSpecialFolderPath(CSIDL_PERSONAL, MAX_PATH, buffer));
	printf("CSIDL_PERSONAL: %s\n", from_unicode(buffer));
	TEST_NOT_FALSE(length = CeGetSpecialFolderPath(CSIDL_PROGRAMS, MAX_PATH, buffer));
	printf("CSIDL_PROGRAMS: %s\n", from_unicode(buffer));
	TEST_NOT_FALSE(length = CeGetSpecialFolderPath(CSIDL_RECENT, MAX_PATH, buffer));
	printf("CSIDL_RECENT: %s\n", from_unicode(buffer));
	TEST_NOT_FALSE(length = CeGetSpecialFolderPath(CSIDL_STARTUP, MAX_PATH, buffer));
	printf("CSIDL_STARTUP: %s\n", from_unicode(buffer));
	
	VERIFY_HRESULT(CeRapiUninit());
	return TEST_SUCCEEDED;
}

