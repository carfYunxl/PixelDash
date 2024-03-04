
#include <stdio.h>
#include <vector>
#include <fstream>

#include "CommDef.h"
#include "../../dependency/Adb/include/HFST_ADB_SDK_ErrorCodeDef.h"

void printArray(unsigned char* arr, unsigned int size)
{
	for (unsigned int i = 0;i < size;++i)
	{
		if (i % 16 == 0 && i != 0)
		{
			printf("\n");
		}
		printf("%02X ", arr[i]);
	}
}

//#pragma comment(lib, "../Debug/TTK_Communication_API_A8015.lib")
int A8015_Test()
{
	CommunicationModeSelect( 1 );
	int res = ADB_Init( false, "d:\\platform-tools\\adb.exe", "/data/httu/sitronix_server &" );
	if ( HFST_ADB_ERROR_OK != res ) {
		printf( "ADB_Init faild, res = %d\n", res );
		return res;
	}

	unsigned char tmp_status = 0xFF;
	res = ReadI2CReg( &tmp_status, 0x01, 1 );
	printf( "Read I2C REG 0x01, res: %02X, status: %02X\n", res, tmp_status );

	unsigned char chip_id = 0xFF;
	res = ReadI2CReg( &chip_id, 0xF4, 1 );
	printf( "Read I2C REG 0xF4, res: %02X, chip_id: %02X\n", res, chip_id );

	for ( int idx = 0; idx < 10; ++idx ) {
		unsigned char tmp_dist[200]{ 0 };
		res = ReadI2CReg( tmp_dist, 0x40, 60 );

		printf( "[%d]dist: ", idx );
		for ( int pos = 0; pos < 10; ++pos )
			printf( "%02X ", tmp_dist[pos] );
		printf( "\n" );
	}

	for (int i = 0;i < 5; ++i)
	{
		unsigned char r16K[0xFC00]{ 0 };
		res = ReadFlashSW(r16K, 0xB800, 0x800);
		if (res > 0)
		{
			unsigned char buf[8]{ 0 };
			res = ReadI2CReg(buf, 0xF4, 8);
		}

		res = WriteFlashSW(r16K, 0xB800, 0x800);
		if (res > 0)
		{
			unsigned char buf[8]{ 0 };
			res = ReadI2CReg(buf, 0xF4, 8);
		}
	}

	for (int i = 0; i < 10; ++i)
	{
		unsigned char bufs[8]{ 0 };
		res = ReadI2CReg(bufs, 0xF4, 8);
		if (res < 0)
		{
			printf("[fail] ----------\n");
		}
		else
		{
			printf("[success] chip id = %d\n", bufs[0]);
		}
	}

	printf( "complete!\n\n" );
	ADB_UnInit();

	return 1;
}