#include "test.h"
#include <rapi.h>

int main()
{
	TEST_HRESULT(CeRapiInit());
	TEST_HRESULT(CeRapiUninit());
	return TEST_SUCCEEDED;
}

