#include <stdlib.h>
#include <ctype.h>
#include "test.h"
#include <synce_log.h>

int handle_property(PCEPROPVAL value)
{
	//unsigned k;
	unsigned id = value->propid >> 16;
	switch (id)
	{
#if 0
		// Mailbox list
		case 0x0000: printf("FolderNumber"); break;
		case 0x0001: printf("FolderName"); break;	// also used by Messages
		case 0x0005: printf("MessageOid"); break;
		case 0x0006: printf("ParentFolderNumber"); break;	// set to 65535 for top folder
		// case 0x0008: printf("xxx"); break;	// also used by Messages

		// Messages
		// case 0x0001: printf("xx"); break; // also used by Mailbox list
		case 0x0003: printf("Size1"); break;
		case 0x0004: printf("Date"); break;  // XXX: id 0x0004 is also used by Appointments
		case 0x0007: printf("Subject"); break;
		case 0x0008: printf("Header"); break;	// also used by Mailbox list
		case 0x0009: printf("FolderOid"); break;
		case 0x000a: printf("Body"); break;
		case 0x000d: printf("Attachment"); break;
		case 0x0011: printf("Size2"); break;
		
		// Categories
		case 0x4001: printf("Name"); break;
		case 0x4002: printf("Id"); break;	// the bit indexed by Id is set in CategoryBits
		case 0x4003: printf("Category2"); break;
		case 0x4004: printf("Category3"); break;
		case 0x4005: printf("Category4"); break;
		case 0x4006: printf("Category5"); break;
		
		// Contacts
		case 0x3a06: printf("First name"); break;
		case 0x3a08: printf("Work tel"); break;
		case 0x3a09: printf("Home tel"); break;
		case 0x3a11: printf("Last name"); break;
		case 0x3a16: printf("Company"); break;
		case 0x3a17: printf("Job title"); break;
		case 0x3a18: printf("Department"); break;
		case 0x3a1c: printf("Mobile tel"); break;
		case 0x3a1e: printf("Car tel"); break;
		case 0x3a24: printf("Work fax"); break;
		case 0x3a25: printf("Home fax"); break;
		case 0x3a2f: printf("Home2 tel"); break;
		case 0x4007: printf("Work2 tel"); break;
		case 0x4008: printf("Web page"); break;
		case 0x4009: printf("Pager"); break;
		case 0x4013: printf("Full name"); break;
		case 0x4040: printf("Address1"); break;
		case 0x4041: printf("Address2"); break;
		case 0x4042: printf("Address3"); break;
		case 0x4043: printf("Address4"); break;
		case 0x4044: printf("Country"); break;
		case 0x4083: printf("E-mail"); break;
		case 0x4093: printf("E-mail2"); break;
		case 0x40a3: printf("E-mail3"); break;

		// Tasks
		case 0x4104: printf("StartDate"); break;
			// Always 
								 
		case 0x4105: printf("DueDate"); break;
		case 0x410f: printf("Completed"); break;
			// this is both a FILETIME and a number!
			// for the number: 0=NotCompleted,1=Completed

		// Appointments
		//case 0x0004: printf("Sensitivity"); break;	// 0=Normal,1=Private  XXX: id 0x0004 is also used by Mail
		case 0x000f: printf("BusyStatus"); break;	// 0=Free,1=Tentative,2=Busy,3=OutOfOffice
		case 0x0016: printf("Categories"); break;
		case 0x0017: printf("_Notes"); break;
		case 0x0037: printf("Subject"); break;

		case 0x0067: printf("_52bytes"); break;
			// the first 36 are always the same, on my system:
			// 04 00 00 00 82 00 e0 00 74 c5 b7 10 1a 82 e0 08 00 00 00 00 
			// 83 54 2d 00 90 00 75 50 72 27 14 79 56 8b 58 5d
			// the last 16 varies in a very random way, proably a GUID
								 
		case 0x4208: printf("Location"); break;
		case 0x420d: printf("Start"); break;
		case 0x4213: printf("Duration"); break; // see DurationUnit
		case 0x4215: printf("_DurationUnit"); break; // 1=Days,2=Minutes
		case 0x4501: printf("ReminderMinutesBeforeStart"); break; // minutes
		case 0x4503: printf("ReminderEnabled"); break;
		case 0x4509: printf("ReminderSoundFile"); break;
#endif
		default:
			printf("%04x", id);
			break;
	}
	printf("=");

	char *tmp_str = NULL;
	unsigned type = value->propid & 0xffff;
	switch (type)
	{
		case CEVT_I2:  printf("0x%04x/%i",  value->val.iVal,  value->val.iVal);  break;
		case CEVT_I4:  printf("0x%08x/%i", value->val.lVal,  value->val.lVal);  break;
		case CEVT_UI2: printf("0x%04x/%u",  value->val.uiVal, value->val.uiVal); break;
		case CEVT_UI4: printf("0x%08x/%u",  value->val.ulVal, value->val.ulVal); break;
		case CEVT_BOOL: printf("0x%08x/%u",  value->val.boolVal, value->val.boolVal); break;
		
		case CEVT_LPWSTR:
			tmp_str = wstr_to_ascii(value->val.lpwstr);
			printf("\"%s\"", tmp_str);
			free(tmp_str);
			break;

		case CEVT_FILETIME:
			{
				if (0 == value->val.filetime.dwLowDateTime && 0 == value->val.filetime.dwHighDateTime)
				{
					printf("NULL");
				}
				else
				{
#ifndef WIN32
					time_t unixtime = filetime_to_unix_time(&value->val.filetime);
					struct tm *tm = localtime(&unixtime);
					char buffer[MAX_PATH];
					strftime(buffer, MAX_PATH, "%c", tm);
					printf("%08x %08x=%s",value->val.filetime.dwHighDateTime,value->val.filetime.dwLowDateTime, buffer);
#endif
				}
			}
			break;

		case CEVT_BLOB:
			printf("BLOB (size=%u)", value->val.blob.dwCount);
/*			printf(" \"");
			for (k = 0; k < value->val.blob.dwCount; k++)
			{
				int c = value->val.blob.lpb[k];
				printf("%c", isprint(c) ? c : '.');
			}
			printf("\"={ ");
			for (k = 0; k < value->val.blob.dwCount; k++)
			{
				printf("%02x ", value->val.blob.lpb[k]);
			}
			printf("}");*/
			break;
			
		default:
			printf("(type %u)", type);
			break;
	}

	return TEST_SUCCEEDED;
}

int handle_record(PCEPROPVAL values, DWORD property_count)
{
	for (unsigned j = 0; j < property_count; j++)
	{
		if (j > 0)
			printf("; ");

		if (TEST_SUCCEEDED != handle_property(values+j))
			return TEST_FAILED;

	} // for every property

	return TEST_SUCCEEDED;
}

int handle_database(HANDLE db, DWORD num_records)
{
#define MYBUFSIZE  (16*1024)
	PCEPROPVAL values = NULL;
	DWORD buffer_size = 0;

	for (unsigned i = 0; i < num_records; i++)
	{
		values = (PCEPROPVAL)malloc(MYBUFSIZE);
		buffer_size = MYBUFSIZE;

		WORD property_count;
		CEOID oid;

		LPBYTE values_lpbyte = (LPBYTE)values;
		TEST_NOT_FALSE(oid = CeReadRecordProps(db, CEDB_ALLOWREALLOC, &property_count, NULL, &values_lpbyte, &buffer_size));
	
		printf("Row %u (oid=0x%x): ", i, oid);
	
		if (TEST_SUCCEEDED != handle_record(values, property_count))
			return TEST_FAILED;
		
		printf("\n");

		unsigned char * p = property_count * sizeof(CEPROPVAL) + (BYTE*)values;
		for (unsigned i = 0; i < (buffer_size - property_count * sizeof(CEPROPVAL)); i++)
		{
			int c = p[i];
			printf("%02x %c\n", c, isprint(c) ? c : '.');
		}
	
		CeRapiFreeBuffer(values);
		
	} // for every row

	return TEST_SUCCEEDED;
}

int main()
{
	VERIFY_HRESULT(CeRapiInit());

//	rapi_log_set_level(RAPI_LOG_LEVEL_ERROR);
	
	CEDB_FIND_DATA* find_data = NULL;
	WORD db_count = 0;
	
	// Find database
	VERIFY_NOT_FALSE(CeFindAllDatabases(0, 0xffff, &db_count, &find_data));
/*	if (db_count != 1)
	{
		printf("Expected one database, found %u.\n", db_count);
		return TEST_FAILED;
	}*/

	for (unsigned i = 0; i < db_count; i++)
	{
		HANDLE db = INVALID_HANDLE_VALUE;
		TEST_NOT_EQUAL(INVALID_HANDLE_VALUE, db = CeOpenDatabase(&find_data[i].OidDb, NULL, 0, CEDB_AUTOINCREMENT, 0));

		handle_database(db, find_data[i].DbInfo.wNumRecords);
	
		TEST_NOT_FALSE(CeCloseHandle(db));
	}
	
	//printf ("find_data is valid pointer: %i\n", is_valid_ptr(find_data));
	VERIFY_HRESULT(CeRapiFreeBuffer(find_data));

	VERIFY_HRESULT(CeRapiUninit());
	return TEST_SUCCEEDED;
}

