#include "test.h"

static const char* database_name = "TestDatabase";
static const DWORD database_type = 31337;

static const int VALUE_COUNT = 8;

static int DeleteDatabases()
{
	CEOID oid = 0;

	/* Delete any old test databases */

	HANDLE enumerate = CeFindFirstDatabase(database_type);

	while (0 != (oid = CeFindNextDatabase(enumerate)))
	{
		TEST_NOT_FALSE(CeDeleteDatabase(oid));
	}

	TEST_NOT_FALSE(CeCloseHandle(enumerate));

	return TEST_SUCCEEDED;
}

int main()
{
	VERIFY_HRESULT(CeRapiInit());

	if (DeleteDatabases() != TEST_SUCCEEDED)
		return TEST_FAILED;

	/* Create database */
	
	CEOID oid = 0;
	TEST_NOT_FALSE(oid = CeCreateDatabase(to_unicode(database_name), database_type, 0, NULL));

	/* Open database */

	HANDLE database;
 	TEST_NOT_EQUAL(INVALID_HANDLE_VALUE,
			database = CeOpenDatabase(&oid, NULL, 0, CEDB_AUTOINCREMENT, 0));

	/* Write stuff */

	CEPROPVAL write_values[VALUE_COUNT];
	memset(&write_values, 0, sizeof(write_values));

	const char* blob_data = "Binary Large Object"; // Note: uneven length
	write_values[0].propid = 0x10000 | CEVT_BLOB;
	write_values[0].val.blob.dwCount = strlen(blob_data);
	write_values[0].val.blob.lpb = (LPBYTE)blob_data;

	write_values[1].propid = 0x20000 | CEVT_BOOL;
	write_values[1].val.boolVal = 1;
	
	write_values[2].propid = 0x30000 | CEVT_FILETIME;
	filetime_from_unix_time(time(NULL), &write_values[2].val.filetime);

	write_values[3].propid = 0x40000 | CEVT_I2;
	write_values[3].val.iVal = 0x1234;

	write_values[4].propid = 0x50000 | CEVT_I4;
	write_values[4].val.lVal = 0x23456789;

	write_values[5].propid = 0x60000 | CEVT_LPWSTR;
	write_values[5].val.lpwstr = to_unicode("Some string value"); // Note: uneven length

	write_values[6].propid = 0x70000 | CEVT_UI2;
	write_values[6].val.uiVal = 0xbcde;

	write_values[7].propid = 0x80000 | CEVT_UI4;
	write_values[7].val.ulVal = 0x98765432;

#if 0
	/* Write each value */
	
	for (int i = 0; i < VALUE_COUNT; i++)
	{
		/* Write value */
		CEOID write_record;
		TEST_NOT_FALSE(write_record = CeWriteRecordProps(database, 0, 1, &write_values[i]));
 
		/* Seek */
		DWORD index;
		TEST_NOT_FALSE(CeSeekDatabase(database, CEDB_SEEK_BEGINNING, 0, &index));
		
		/* Read value */
		CEOID read_record;
		CEPROPVAL* read_value = NULL;
		DWORD read_size = 0;
		WORD read_count = 0;
		TEST_NOT_FALSE(read_record = CeReadRecordProps(database, CEDB_ALLOWREALLOC, 
					&read_count, NULL, (LPBYTE*)&read_value, &read_size));

		if (read_count != 1 )
		{
			printf("ERROR: read_count has unexpected value: %i\n", read_count);
		}

		if (write_values[i].propid != read_value->propid)
		{
			printf("ERROR: different propid for index %i\n", i);
		}

#define NOT_EQUAL(member) (write_values[i].val.member != read_value->val.member)
		
		switch (i)
		{
			case 0:
				if (NOT_EQUAL(blob.dwCount))
					printf("ERROR: unexpecred blob size %i\n", read_value->val.blob.dwCount);
				else if (memcmp(write_values[i].val.blob.lpb, read_value->val.blob.lpb, 
							write_values[i].val.blob.dwCount) != 0)
					printf("ERROR: unexpected blob data\n");
				break;

			case 1:
				if(NOT_EQUAL(boolVal))
					printf("ERROR: bool\n");
				break;

			case 2:
				if (memcmp(&write_values[i].val.filetime, &read_value->val.filetime, 
							sizeof(FILETIME)) != 0)
					printf("ERROR: filetime\n");
				break;

			case 3:
			case 4:
				if(NOT_EQUAL(iVal))
					printf("ERROR: int\n");
				break;

			case 5:
				if (!wstr_equal(write_values[i].val.lpwstr, read_value->val.lpwstr))
					printf("ERROR: string\n");
				break;

			case 6:
			case 7:
				if(NOT_EQUAL(uiVal))
					printf("ERROR: unsigned int\n");
				break;

		}
	}

#endif

	/* Write all write_values */

	CEOID record;
	TEST_NOT_FALSE(record = CeWriteRecordProps(database, 0, VALUE_COUNT, write_values));

	/* Seek */
	DWORD index;
	TEST_NOT_FALSE(CeSeekDatabase(database, CEDB_SEEK_BEGINNING, 0, &index));

	/* Read value */
	CEOID read_record;
	CEPROPVAL* read_value = NULL;
	DWORD read_size = 0;
	WORD read_count = 0;

	LPBYTE read_value_lpbyte = (LPBYTE)read_value;
	TEST_NOT_FALSE(read_record = CeReadRecordProps(database, CEDB_ALLOWREALLOC, 
				&read_count, NULL, &read_value_lpbyte, &read_size));



	/* Clean up */
	
	TEST_NOT_FALSE(CeCloseHandle(database));
	
//	TEST_NOT_FALSE(CeDeleteDatabase(oid));

	return TEST_SUCCEEDED;
}

