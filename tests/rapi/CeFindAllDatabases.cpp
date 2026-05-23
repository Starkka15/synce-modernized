#include "test.h"

int main()
{
	VERIFY_HRESULT(CeRapiInit());

	CEDB_FIND_DATA* find_data = NULL;
	WORD db_count = 0;
	
	TEST_NOT_FALSE(CeFindAllDatabases(0, 0xffff, &db_count, &find_data));

	printf("Number of databases: %u\n", db_count);
	for (unsigned i = 0; i < db_count; i++)
		printf("Database %3u: \"%s\" (oid=0x%x, type=0x%x, rows=%u)\n", i, 
				from_unicode(find_data[i].DbInfo.szDbaseName), 
				find_data[i].OidDb,
				(unsigned)find_data[i].DbInfo.dwDbaseType,
				find_data[i].DbInfo.wNumRecords);
	
	TEST_HRESULT(CeRapiFreeBuffer(find_data));

	VERIFY_HRESULT(CeRapiUninit());
	return TEST_SUCCEEDED;
}

