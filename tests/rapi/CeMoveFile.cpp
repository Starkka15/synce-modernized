//
// Test CeCreateFile, CeCloseHandle, CeWriteFile, CeReadFile, CeDeleteFile
//
#include "test.h"

int main()
{
	VERIFY_HRESULT(CeRapiInit());

	DWORD length;
	WCHAR filename1[MAX_PATH];
	WCHAR filename2[MAX_PATH];
	VERIFY_NOT_FALSE(length = CeGetSpecialFolderPath(CSIDL_PERSONAL, MAX_PATH, filename1));
	wstrcpy(filename2, filename1);
	wstr_append(filename1, to_unicode("\\librapi test file 1.txt"), sizeof(filename1));
	wstr_append(filename2, to_unicode("\\librapi test file 2.txt"), sizeof(filename2));

	HANDLE handle;

	// Open file for writing, create if it does not exist, close immediatly
	VERIFY_NOT_EQUAL(INVALID_HANDLE_VALUE, handle = CeCreateFile(filename1,
				GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0));
	
	// Get file size
	VERIFY_EQUAL(0, CeGetFileSize(handle, NULL));
	
	VERIFY_NOT_FALSE(CeCloseHandle(handle));

	// Delete second file if it exists (ignore return value)
	CeDeleteFile(filename2);

	// Move from first to second filename
	TEST_NOT_FALSE(CeMoveFile(filename1, filename2));
	
	// Delete second file, should succeed
	TEST_NOT_FALSE(CeDeleteFile(filename2));

	
	VERIFY_HRESULT(CeRapiUninit());
	return TEST_SUCCEEDED;
}

