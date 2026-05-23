#include "test.h"

int main()
{
	VERIFY_HRESULT(CeRapiInit());

	WCHAR* path = to_unicode("\\My Documents\\*.*");

	CE_FIND_DATA* find_data = NULL;
	DWORD file_count = 0;
	
	TEST_NOT_FALSE(CeFindAllFiles(path, FAF_NAME, &file_count, &find_data));

	printf("Number of files: %u\n", file_count);
	for (unsigned i = 0; i < file_count; i++)
		printf("File %3u: \"%s\"\n", i, from_unicode(find_data[i].cFileName));
	
	TEST_HRESULT(CeRapiFreeBuffer(find_data));

	VERIFY_HRESULT(CeRapiUninit());
	return TEST_SUCCEEDED;
}

