#include "test.h"

int main()
{
	VERIFY_HRESULT(CeRapiInit());

	CEOSVERSIONINFO version;
	
	memset(&version, 0, sizeof(CEOSVERSIONINFO));
	version.dwOSVersionInfoSize = sizeof(CEOSVERSIONINFO);
	
	printf("size = 0x%zx\n", sizeof(CEOSVERSIONINFO));
	
	TEST_NOT_EQUAL(0, CeGetVersionEx(&version));
	
	printf("dwMajorVersion=%i, dwMinorVersion=%i, dwBuildNumber=%i, dwPlatformId=%i, szCSDVersion=\"%s\"\n",
			version.dwMajorVersion,
			version.dwMinorVersion,
			version.dwBuildNumber,
			version.dwPlatformId,	// 3 == VER_PLATFORM_WIN32_CE
			from_unicode(version.szCSDVersion));

	if (3 != version.dwPlatformId)
	{
		printf("Platform ID is not 3!\n");
		return TEST_FAILED;
	}
	
	VERIFY_HRESULT(CeRapiUninit());
	return TEST_SUCCEEDED;
}

