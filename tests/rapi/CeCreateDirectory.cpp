//
// Test CeCreateDirectory and CeRemoveDirectory
//

#include "test.h"

int main()
{
	VERIFY_HRESULT(CeRapiInit());

	DWORD length;
	WCHAR dirname[MAX_PATH];
	VERIFY_NOT_FALSE(length = CeGetSpecialFolderPath(CSIDL_PERSONAL, MAX_PATH, dirname))
	wstr_append(dirname, to_unicode("\\librapi test directory"), sizeof(dirname));
	
	// First, remove directory (ignoring return value)
	CeRemoveDirectory(dirname);
	
	// Create directroy. This should succeed.
	TEST_NOT_EQUAL(0, CeCreateDirectory(dirname, NULL));
	
	// Create directroy again. This should fail.
	TEST_EQUAL(0, CeCreateDirectory(dirname, NULL));
	
	// Remove directory. This should succeed.
	TEST_NOT_EQUAL(0, CeRemoveDirectory(dirname));
	
	// Remove directory again. This should fail.
	TEST_EQUAL(0, CeRemoveDirectory(dirname));
	
	VERIFY_HRESULT(CeRapiUninit());
	return TEST_SUCCEEDED;
}

