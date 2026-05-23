#include "test.h"

int main()
{
	VERIFY_HRESULT(CeRapiInit());

	HANDLE handle;
	CE_FIND_DATA find_data;
	
	WCHAR * all_files = to_unicode("*.*");
	TEST_NOT_EQUAL(INVALID_HANDLE_VALUE, handle = CeFindFirstFile(all_files, &find_data));

	printf("First file: \"%s\"\n", from_unicode(find_data.cFileName));

	TEST_NOT_EQUAL(0, CeFindClose(handle));
	
	VERIFY_HRESULT(CeRapiUninit());
	return TEST_SUCCEEDED;
}

