
#include <windows.h>
#include <vector>
#include <chrono>
#include "CommDef.h"

#pragma comment(lib, "../Debug/TTK_Communication_API_A8018.lib")

//#define A8018_HW_TEST
#define A8018_SW_TEST
//#define A8018_PRAM_TEST
//#define A8018_SW_CMD_TEST

int SetI2C_Clock_A8018( int nKHz )
{
	BYTE byteCommand[BulkLen] = { 0 };
	if ( nKHz == 0x00 )   nKHz = 400;

	nKHz = nKHz * 1000;
	byteCommand[0] = 0x50;
	byteCommand[1] = 0x05;
	byteCommand[2] = 0x00;
	byteCommand[3] = 0x03;
	byteCommand[4] = 0x00;//Set
	byteCommand[5] = nKHz & 0xFF;//L
	byteCommand[6] = (nKHz >> 8) & 0xFF;//M
	byteCommand[7] = (nKHz >> 16) & 0xFF;//H
	USBComm_WriteToBulkEx( byteCommand, BulkLen );

	return 1;

}

constexpr unsigned int FlashSize64K = 1024 * 64;
constexpr unsigned int FlashSize48K = 1024 * 48;

int FlashHW_Test_WR()
{
	unsigned char tmp_data_WR[]{ 0x32, 0x59, 0x0A, 0x0B, 0x03, 0x04, 0x05, 0x06 };
	return WriteFlashHW_WR( tmp_data_WR, 0xF3F0, 0x08 );
}

void FlashHW_Test( BOOL bNeedWrite, unsigned char * bufDumpData )
{
	int res = 1;

	// 正常使用
	struct {
		unsigned int nStartAddr;
		unsigned int nWriteSize;
	} FlashTestBuf[]{
		// Special
//		{0xBC00, 0x800},	// 0xBC00 ~ 0xC400
//		{0xC000, 0x800},	// 0xC000 ~ 0xC800
//		{0xEC00, 0x800},	// 0xEC00 ~ 0xF400
//		{0xF800, 0x800},	// 0xF800 ~ 0x10000

		// Normal
//		{0x00, 0x400},		// 0x00 ~ 0x400
//		{0x00, 0x800},		// 0x00 ~ 0x800
//		{0x400, 0x800},		// 0x400 ~ 0xC00
//		{0x400, 0xC00},		// 0x400 ~ 0x1000
//		{0x00, 0xC000},		// 0x00 ~ 0xC000
//		{0xBC00, 0x400},	// 0xBC00 ~ 0xC000
//		{0xF000, 0x400},	// 0xF000 ~ 0xF400
//		{0xF000, 0x800},	// 0xF000 ~ 0xF800
		{0xF000, 0xC00},	// 0xF000 ~ 0xFC00
//		{0xF400, 0x400},	// 0xF400 ~ 0xF800
//		{0xF400, 0x800},	// 0xF400 ~ 0xFC00
//		{0xF800, 0x400},	// 0xF800 ~ 0xFC00
//		{0x00, 0xFC00}		// 0x00 ~ 0xFC00
	};

	printf( "\n" );
	for ( int idx = 0; idx < _countof( FlashTestBuf ); ++idx ) {
		auto s_time = std::chrono::system_clock::now();
		res = WriteFlashHW( &bufDumpData[FlashTestBuf[idx].nStartAddr], FlashTestBuf[idx].nStartAddr, FlashTestBuf[idx].nWriteSize );
		printf( "WriteFlashHW ISP Addr %X Size %X ... %s\n", FlashTestBuf[idx].nStartAddr, FlashTestBuf[idx].nWriteSize, 1 == res ? "PASS" : "NG" );
		auto e_time = std::chrono::system_clock::now();
		printf( "WriteFlashHW cost %lld ms\r\n", std::chrono::duration_cast<std::chrono::milliseconds>(e_time - s_time).count() );

		if ( -3 == res ) {
			printf( "Addr Error\n\n" );
			continue;
		}

		if ( 1 != res ) {
			int x = 0;
			x++;
		}

		unsigned char bufReadData[FlashSize64K]{ 0 };
		s_time = std::chrono::system_clock::now();
		//SetI2C_Clock_A8018( 1800 );
		res = ReadFlashHW( bufReadData, FlashTestBuf[idx].nStartAddr, FlashTestBuf[idx].nWriteSize );
		printf( "ReadFlashHW back ... %s\n", 1 == res ? "PASS" : "NG" );
		e_time = std::chrono::system_clock::now();
		printf( "ReadFlashHW cost %lld ms\r\n", std::chrono::duration_cast<std::chrono::milliseconds>(e_time - s_time).count() );

// 		FILE * fp1 = fopen( "d:\\hw_read.bin", "wb" );
// 		fwrite( bufReadData, 1, FlashTestBuf[idx].nWriteSize, fp1 );
// 		fclose( fp1 );

		if ( 1 != res ) {
			int x = 0;
			x++;
		}

		res = memcmp( &bufDumpData[FlashTestBuf[idx].nStartAddr], bufReadData, FlashTestBuf[idx].nWriteSize );
		if ( res ) {
			int nCompSize = FlashTestBuf[idx].nWriteSize;
			if ( FlashTestBuf[idx].nStartAddr + nCompSize > 0xFC00 )
				nCompSize = 0xFC00 - FlashTestBuf[idx].nStartAddr;
			res = memcmp( &bufDumpData[FlashTestBuf[idx].nStartAddr], bufReadData, nCompSize );
		}
		printf( "Check ... %s\n\n", 0 == res ? "PASS" : "NG" );
	}

	ResetTP();
	Sleep( 200 );
}

void SW_DeviceStatus()
{
	unsigned char cmd = 0x01;
	int res = WriteCmd( &cmd, 1 );

	unsigned char data[8]{ 0 };
	res = ReadCmd( data, 8 );

	printf( "DeviceStatus: %02X ChipID: %02X IC_Ver: %02X ISP_Ver: %02X %02X\n", data[0], data[1], data[2], data[3], data[4] );
}

bool JumpISP( void )
{
	int ret, i = 0, Repeat = 2;
	unsigned char state[9];
	ret = ReadI2CReg( state, 0x01, 0x06 );
	if ( ret < 0 )
	{
		return false;
	}

	if ( (state[0] & 0x0F) == 0x06 )
	{
		return true;
	}

	unsigned char chip_id = 0xFF;
	ret = ReadI2CReg( &chip_id, 0xF4, 1 );

	unsigned char pattern[9] = { "STX_FWUP" };

	//====check status====end*/
	Repeat = 2;
	while ( Repeat-- )
	{
		for ( i = 0; i < 8; i++ )
		{
			//ret = WriteI2CReg_Bulk(0x00,JumpSWISPString[i]);
			ret = WriteI2CReg( &pattern[i], 0x00, 1 );
			if ( ret == false )
			{
				return false;
			}
		}
		
		Sleep( 10 );

		//====check status====
		ret = ReadI2CReg( state, 0x01, 0x08 );
		if ( ret < 0 )
			return false;

		if ( (state[0] & 0x0F) == 0x06 )
			return true;		
	}
	return false;
}

void FlashSW_Test( BOOL bNeedWrite, unsigned char * bufDumpData )
{
	if ( bNeedWrite ) {
		SetI2CAddr( 0x18, 1, 0 );
		int res = WriteFlashHW( &bufDumpData[0xF000], 0xF000, 0xC00 );
		printf( "WriteFlashHW ISP Addr 0 Size 0xFC00 ... %s\n", 1 == res ? "PASS" : "NG" );
		ResetTP();
		Sleep( 500 );
		SetI2CAddr( 0x55, 1, 0 );
	}

	int res = JumpISP();
	SW_DeviceStatus();

	struct {
		unsigned int nStartAddr;
		unsigned int nWriteSize;
	} FlashTestBuf[]{
		// Special
//		{0xBC00, 0x800},	// 0xBC00 ~ 0xC400
//		{0xC000, 0x800},	// 0xC000 ~ 0xC800
//		{0xEC00, 0x800},	// 0xEC00 ~ 0xF400
//		{0xF800, 0x800},	// 0xF800 ~ 0x10000

		// Normal
//		{0x00, 0x400},		// 0x00 ~ 0x400
//		{0x00, 0x800},		// 0x00 ~ 0x800
//		{0x400, 0x400},		// 0x400 ~ 0x800
//		{0x400, 0x800},		// 0x400 ~ 0xC00
		{0x00, 0xC000},		// 0x00 ~ 0xC000
		//{0xBC00, 0x400},	// 0xBC00 ~ 0xC000
//		{0xF000, 0x400},	// 0xF000 ~ 0xF400
		//{0xF000, 0x800},	// 0xF000 ~ 0xF800
//		{0xF000, 0xC00},	// 0xF000 ~ 0xFC00
//		{0xF000, 0x400},	// 0xF400 ~ 0xF800
//		{0xF400, 0x800},	// 0xF400 ~ 0xFC00
//		{0xF800, 0x400},	// 0xF800 ~ 0xFC00
//		{0x00, 0xFC00}		// 0x00 ~ 0xFC00
	};

	printf( "\n" );
	for ( int idx = 0; idx < _countof( FlashTestBuf ); ++idx ) {
		auto s_time = std::chrono::system_clock::now();
		res = WriteFlashSW( &bufDumpData[FlashTestBuf[idx].nStartAddr], FlashTestBuf[idx].nStartAddr, FlashTestBuf[idx].nWriteSize );
		printf( "WriteFlashSW ISP Addr %X Size %X ... %s\n", FlashTestBuf[idx].nStartAddr, FlashTestBuf[idx].nWriteSize, 1 == res ? "PASS" : "NG" );
		auto e_time = std::chrono::system_clock::now();
		printf( "WriteFlashSW cost %lld ms\r\n", std::chrono::duration_cast<std::chrono::milliseconds>(e_time - s_time).count() );

		if ( 1 != res ) {
			int x = 0;
			x++;
		}

		unsigned char bufReadData[FlashSize64K]{ 0 };
		s_time = std::chrono::system_clock::now();
		res = ReadFlashSW( bufReadData, FlashTestBuf[idx].nStartAddr, FlashTestBuf[idx].nWriteSize );
		printf( "ReadFlashSW back ... %s\n", 1 == res ? "PASS" : "NG" );
		e_time = std::chrono::system_clock::now();
		printf( "ReadFlashSW cost %lld ms\r\n", std::chrono::duration_cast<std::chrono::milliseconds>(e_time - s_time).count() );

		if ( 1 != res ) {
			int x = 0;
			x++;
		}

		res = memcmp( &bufDumpData[FlashTestBuf[idx].nStartAddr], bufReadData, FlashTestBuf[idx].nWriteSize );
		printf( "Check ... %s\n\n", 0 == res ? "PASS" : "NG" );

		if ( res ) {
			for ( int i = 0; i < FlashTestBuf[idx].nWriteSize; ++i ) {
				if ( (FlashTestBuf[idx].nStartAddr + i) > 0xFC00 )
					break;
				if ( (FlashTestBuf[idx].nStartAddr + i) > FlashSize48K && (FlashTestBuf[idx].nStartAddr + i) <= 0xF000 )
					continue;
				if ( bufDumpData[FlashTestBuf[idx].nStartAddr + i] != bufReadData[i] )
					printf( "Addr %X Dump = %02X, Red = %02X\n", i, bufDumpData[FlashTestBuf[idx].nStartAddr + i], bufReadData[i] );
			}
		}
	}
}

void PRAM_Test()
{
// 	const char * file = "d:\\A8018_Open_Short_V0.4.bin";
// 
// 	FILE * fp = fopen( file, "rb" );
// 	if ( !fp ) return;
// 
// 	unsigned char bufDumpData[6 * 1024];
// 	memset( bufDumpData, 0xFF, 6 * 1024 );
// 
// 	size_t nReadSize = fread( bufDumpData, 1, 6 * 1024, fp );
// 	fclose( fp );
// 
// 	int res = HFST_A8018_SDK_WriteURAMSwitchToPRAM( bufDumpData, nReadSize );
// 	printf( "PRAM ... %s\r\n", res > 0 ? "OK" : "NG" );
// 
// 	if ( res > 0 ) {
// 		SetI2CAddr( 0x55, 1, 0 );
// 
// 		unsigned char tmpRead = 0xFF;
// 		res = ReadI2CReg( &tmpRead, 0x01, 1 );
// 
// 		printf( "PRAM Status: %02X\r\n", tmpRead );
// 	}
}

void RUN_A8018_SW_CMD_TEST_0x01()
{
	unsigned char cmd_0x01 = 0x01;
	WriteCmd( &cmd_0x01, 1 );

	unsigned char tmp_data_0x01[0x0F]{ 0 };
	ReadCmd( tmp_data_0x01, 0x0F );

	unsigned char FabID = tmp_data_0x01[2] >> 6;
	unsigned char Version = tmp_data_0x01[2] & 0x3F;

	printf( "============ CMD 0x01 ============\n" );
	printf( "Device Status  : 0x%02X\n", tmp_data_0x01[0] );
	printf( "IC ID          : 0x%02X(0x10)\n", tmp_data_0x01[1] );
	printf( "FabID & Version: 0x%02X ==> %c8018%c\n", tmp_data_0x01[2], 'A' + FabID, 'A' + Version);
	printf( "ISP Version    : 0x%02X 0x%02X\n", tmp_data_0x01[3], tmp_data_0x01[4] );
	printf( "\n" );
}

bool RUN_A8018_SW_CMD_TEST_0x83( unsigned char oldICPI2cAddr, unsigned char IspI2cAddr )
{
// 	for ( unsigned char i = 0x00; i <= 0x7F; i++ )
// 	{
// 		SetI2CAddr( i, 1, 0 );
// 		unsigned char ReadData = 0xFF;
// 		int kRet = ReadI2CReg( &ReadData, 0x01, 1 );
// 
// 		if ( kRet > 0 )
// 			printf( "I2C Addr: 0x%02X, Status: 0x%02X\n", i, ReadData );
// 	}

	// modify icp i2c address
	printf( "============ CMD 0x83 ============\n" );
	constexpr unsigned char RESULT_OLD_I2C_ADDR = 1;
	constexpr unsigned char RESULT_NEW_I2C_ADDR = 2;
	constexpr unsigned char RESULT_ISP_I2C_ADDR = 3;
	constexpr unsigned char RESULT_ERROR = 4;
	auto pfnCheckIsNewICPI2cAddr = [=]( unsigned char newICPI2cAddr ) {
		unsigned char tmp_read = 0xFF;

		SetI2CAddr( oldICPI2cAddr, 2, 0 );
		if ( ReadI2CReg( &tmp_read, 0x01, 1 ) > 0 )
			return RESULT_OLD_I2C_ADDR;

		SetI2CAddr( newICPI2cAddr, 2, 0 );
		if ( ReadI2CReg( &tmp_read, 0x01, 1 ) > 0 )
			return RESULT_NEW_I2C_ADDR;

		SetI2CAddr( IspI2cAddr, 2, 0 );
		if ( ReadI2CReg( &tmp_read, 0x01, 1 ) > 0 )
			return RESULT_ISP_I2C_ADDR;

		return RESULT_ERROR;
	};

#if 0
	unsigned char tmp_Status_1 = 0xFF;
	ReadI2CReg( &tmp_Status_1, 0x01, 1 );
	printf( "[0]Before set new ICP I2C address, status = 0x%02X\n", tmp_Status_1 );

	// 0x83: [ICP I2C Address] [ICP I2C Address_n Error]
	unsigned char cmd_0x83_invalid[]{ 0x83, 0x25, 0x26 };
	WriteCmd( cmd_0x83_invalid, sizeof( cmd_0x83_invalid ) );
	if ( unsigned char res = pfnCheckIsNewICPI2cAddr( 0x25 ); RESULT_ISP_I2C_ADDR != res ) {
		printf( "[1]ICP I2C address, Not ICP I2C address_n ... NG\n" );
		return false;
	}
	printf( "[1]ICP I2C address, Not ICP I2C address_n ... PASS\n" );
	for ( unsigned char i = 0x00; i <= 0x7F; i++ )
	{
		SetI2CAddr( i, 1, 0 );
		unsigned char ReadData = 0xFF;
		int kRet = ReadI2CReg( &ReadData, 0x01, 1 );

		if ( kRet > 0 )
			printf( "I2C Addr: 0x%02X, Status: 0x%02X\n", i, ReadData );
	}

	SetI2CAddr( IspI2cAddr, 2, 0 );
	JumpISP();

	// 2 [ICP I2C Address] [ICP I2C Address_n OK]
	unsigned char cmd_0x83_valid[]{ 0x83, 0x25, 0x25 ^ 0xFF };
	WriteCmd( cmd_0x83_valid, sizeof( cmd_0x83_valid ) );
	if ( unsigned char res = pfnCheckIsNewICPI2cAddr( 0x25 ); RESULT_ISP_I2C_ADDR != res ) {
		SetI2CAddr( 0x25, 2, 0 );
		unsigned char tmp_read = 0xFF;
		if ( ReadI2CReg( &tmp_read, 0x01, 1 ) > 0 )
			return RESULT_NEW_I2C_ADDR;

		printf( "[2]ICP I2C address, ICP I2C address_n ... NG\n" );
		return false;
	}
	printf( "[2]ICP I2C address, ICP I2C address_n ... PASS\n" );
	for ( unsigned char i = 0x00; i <= 0x7F; i++ )
	{
		SetI2CAddr( i, 1, 0 );
		unsigned char ReadData = 0xFF;
		int kRet = ReadI2CReg( &ReadData, 0x01, 1 );

		if ( kRet > 0 )
			printf( "I2C Addr: 0x%02X, Status: 0x%02X\n", i, ReadData );
	}

	SetI2CAddr( IspI2cAddr, 2, 0 );

	unsigned char tmp_Status = 0xFF;
	ReadI2CReg( &tmp_Status, 0x01, 1 );
	printf( "[2]After set new ICP I2C address, status = 0x%02X\n", tmp_Status );
	
	Sleep( 200 );
	JumpISP();

	// 3 [ICP I2C is MCU I2C] [Address_n OK]
	unsigned char cmd_0x83_invalid_2[]{ 0x83, IspI2cAddr, IspI2cAddr ^ 0xFF };
	WriteCmd( cmd_0x83_invalid_2, sizeof( cmd_0x83_invalid_2 ) );
	if ( unsigned char res = pfnCheckIsNewICPI2cAddr( IspI2cAddr ); RESULT_ISP_I2C_ADDR != res && RESULT_NEW_I2C_ADDR != res ) {
		printf( "[3]ICP I2C address is MCU I2C address, ICP I2C address_n ... NG\n" );
		return false;
	}
	printf( "[3]ICP I2C address is MCU I2C address, ICP I2C address_n ... PASS\n" );

	for ( unsigned char i = 0x00; i <= 0x7F; i++ )
	{
		SetI2CAddr( i, 1, 0 );
		unsigned char ReadData = 0xFF;
		int kRet = ReadI2CReg( &ReadData, 0x01, 1 );

		if ( kRet > 0 )
			printf( "I2C Addr: 0x%02X, Status: 0x%02X\n", i, ReadData );
	}

#endif
	SetI2CAddr( IspI2cAddr, 1, 0 );

	unsigned char cmd_RST = 0x80;
	WriteCmd( &cmd_RST, 1 );
	Sleep( 200 );

// 	if ( unsigned char res = pfnCheckIsNewICPI2cAddr( IspI2cAddr ); RESULT_NEW_I2C_ADDR != res ) {
// 		printf( "[4]ICP I2C address is MCU I2C address, ICP I2C address_n ... NG\n" );
// 		return false;
// 	}
// 	printf( "[4]ICP I2C address is MCU I2C address, ICP I2C address_n ... PASS\n" );

	for ( unsigned char i = 0x00; i <= 0x7F; i++ )
	{
		SetI2CAddr( i, 1, 0 );
		unsigned char ReadData = 0xFF;
		int kRet = ReadI2CReg( &ReadData, 0x01, 1 );

		if ( kRet > 0 )
			printf( "I2C Addr: 0x%02X, Status: 0x%02X\n", i, ReadData );
	}

	ResetTP();
	Sleep( 500 );

	for ( unsigned char i = 0x00; i <= 0x7F; i++ )
	{
		SetI2CAddr( i, 1, 0 );
		unsigned char ReadData = 0xFF;
		int kRet = ReadI2CReg( &ReadData, 0x01, 1 );

		if ( kRet > 0 )
			printf( "I2C Addr: 0x%02X, Status: 0x%02X\n", i, ReadData );
	}

	printf( "\n" );
	return true;
}

void RUN_A8018_SW_CMD_TEST( unsigned char oldICPI2cAddr, unsigned char oldMcuI2cAddr )
{
	JumpISP();

	RUN_A8018_SW_CMD_TEST_0x01();
	RUN_A8018_SW_CMD_TEST_0x83( oldICPI2cAddr, oldMcuI2cAddr );
}

bool A8018_Test()
{
	unsigned int addr_1 = 0xF012;
	unsigned int addr_2 = addr_1 - 0xF000;

	unsigned char da = 0x23;
	unsigned char md = da ^ 0xFF;

	// 	const char * ISP_File = "d:\\A8018_ISP_Code_V1.2a.bin";
	//const char * ISP_File = "d:\\Template_20230406_1905.bin";
	//const char * ISP_File = "d:\\A8018_ISP_Code_V1.3.bin";
	//const char * ISP_File = "d:\\FW01040104_20230403_153457.bin";
	//const char * ISP_File = "d:\\A8018_ISP_Code_V1.4c_Debug.bin";
	const char * ISP_File = "d:\\A8018_ISP_Code_V1.5.bin";
	//const char * ISP_File = "d:\\FW01040103_20230403_104738.bin";
	// 
	// FILE * fp_isp = fopen( ISP_File, "rb" );
	FILE * fp_fw = fopen( ISP_File, "rb" );
	// 	if ( !fp_isp || !fp_fw ) return -1;
	// 
	// 	constexpr unsigned int FlashSize64K = 1024 * 64;
	constexpr unsigned int FlashSize48K = 1024 * 63;

	unsigned char bufDumpData[FlashSize64K];
	memset( bufDumpData, 0xFF, FlashSize64K );
	// 
	// 	size_t nReadSize = fread( bufDumpData, 1, FlashSize64K, fp_isp );
	size_t nFwSize = fread( bufDumpData, 1, FlashSize48K, fp_fw );

	// 	std::cout << "File ISP Size: " << nReadSize << ", FW Size: " << nFwSize << std::endl;
	// 	fclose( fp_isp );
	fclose( fp_fw );
	// 
	// 	FILE * fp_new = fopen( "d:\\new_file", "wb" );
	// 	fwrite( bufDumpData, 1, nReadSize, fp_new );
	// 	fclose( fp_new );

#if 0

#endif

	int retNum = USBComm_InitEx( 500, 1 );
	if ( retNum < 0 ) {
		printf( "USB init error\n" );
		USBComm_FinishEx();
		system( "pause" );
		return false;
	}

	retNum = USBComm_CheckBulkInEx();
	if ( retNum == 0 ) {
		printf( "Check bulk in error\n" );
		USBComm_FinishEx();
		system( "pause" );
		return false;
	}

	retNum = USBComm_CheckBulkOutEx();
	if ( retNum == 0 ) {
		printf( "Check bulk out error\n" );
		USBComm_FinishEx();
		system( "pause" );
		return false;
	}

	SetTouchLink3_Voltage( 2.8, 1.8 );
	Sleep( 300 );

	SetProtocoSPI( FALSE );

	SetI2C_Clock_A8018( 700 );
	int nClock_s = Get_I2C_Clock_KHz();

	std::vector< unsigned char > vecI2cAddr;
	for ( unsigned char i = 0x00; i <= 0x7F; i++ )
	{
		SetI2CAddr( i, 1, 0 );
		unsigned char ReadData = 0xFF;
		int kRet = ReadI2CReg( &ReadData, 0x01, 1 );

		if ( kRet > 0 ) {
			vecI2cAddr.push_back( i );
			printf( "I2C Addr: 0x%02X, Status: 0x%02X\n", i, ReadData );
		}
	}

#if defined A8018_HW_TEST
	std::chrono::system_clock::time_point s_pt = std::chrono::system_clock::now();
//	bool res = HFST_CheckIsA8018ICPI2CAddr( 0x18 );
	std::chrono::system_clock::time_point e_pt = std::chrono::system_clock::now();
	printf( "%d ms\r\n", std::chrono::duration_cast<std::chrono::milliseconds>(e_pt - s_pt).count() );

	ResetTP();
// 	if ( vecI2cAddr.size() <= 0 ) {
// 		printf( "Not find i2c addr\n" );
// 		return 1;
// 	}

// 	SetI2CAddr( vecI2cAddr[1], 3, 0 );
// 	SetI2C_Clock_A8018( 2000 );
// 
//  	unsigned char tmp_ChipId = 0xFF;
// 	ReadI2CReg( &tmp_ChipId, 0x01, 0x01 );
//  	ReadI2CReg( &tmp_ChipId, 0xF4, 0x01 );
// 
// 	unsigned char tmp_CFB_GAIN[2]{ 0 };
// 	int res = ReadCommandIOData( 0x04, 0x65C, tmp_CFB_GAIN, 2 );
// 	printf( "Before res: %d, CFB_GAIN: 0x%02X 0x%02X\n", res, tmp_CFB_GAIN[0], tmp_CFB_GAIN[1] );

	SetI2CAddr( vecI2cAddr[0], 0xFF, 0 );
	FlashHW_Test( FALSE, bufDumpData );
	int nClock_e = Get_I2C_Clock_KHz();

// 	SetI2CAddr( vecI2cAddr[1], 3, 0 );
// 	unsigned char tmp_Status = 0xFF;
// 	res = ReadI2CReg( &tmp_Status, 0x01, 1 );
// 
// 	tmp_CFB_GAIN[0] = tmp_CFB_GAIN[1] = 0xFF;
// 	res = ReadCommandIOData( 0x04, 0x65C, tmp_CFB_GAIN, 2 );
// 	printf( "After res: %d, CFB_GAIN: 0x%02X 0x%02X\n", res, tmp_CFB_GAIN[0], tmp_CFB_GAIN[1] );

#elif defined A8018_SW_TEST
	
		// isp in polling mode
	unsigned char i2cAddr = 0x70;
	SetI2CAddr( i2cAddr, 1, 0 );

		for ( int idx = 0; idx < 100; ++idx ) {
			unsigned char ReadData = 0xFF;
			int kRet = ReadI2CReg( &ReadData, 0x01, 1 );

			if ( kRet > 0 ) {
				if ( 0x86 == ReadData ) {
					//unsigned char tmp_Read[0xFF]{ 0 };
					//int rr = ReadFlashHW( tmp_Read, 0xFBF0, 3 );
					unsigned char cmd_ReadTRIM[]{ 0x84, 0x5A, 0xFB, 0xF0, 0x00, 0x03 };
					WriteCmd( cmd_ReadTRIM, sizeof( cmd_ReadTRIM ) );
					Sleep( 5 );
					kRet = ReadI2CReg( &ReadData, 0x01, 1 );
					if ( kRet > 0 ) {
						if ( vecI2cAddr.size() > 1 )
							vecI2cAddr[1] = i2cAddr;
						else
							vecI2cAddr.push_back( i2cAddr );
						printf( "Polling PASS Stop PASS, I2C Addr: 0x%02X\n", i2cAddr );
						break;
					} else {
						printf( "Polling PASS But Not Stop\n" );
					}
				} else {
					printf( "Not polling\n" );
					break;
				}
			}
 		}
// 		
// 		if ( vecI2cAddr.size() <= 1 ) {
// 			ResetTP();
// 			printf( "Not find i2c addr\n" );
// 			return 1;
// 		}
	//}

	//	ResetTP();
	SetI2CAddr( vecI2cAddr[1], 1, 0 );
	SetI2C_Clock_A8018( 2000 );
	FlashSW_Test( FALSE, bufDumpData );
#elif defined A8018_PRAM_TEST
	if ( vecI2cAddr.size() <= 0 ) {
		printf( "Not find i2c addr\n" );
		return 1;
	}
	SetI2CAddr( vecI2cAddr[0], 1, 0 );
	PRAM_Test();
#elif defined A8018_SW_CMD_TEST

	if ( vecI2cAddr.size() <= 1 ) {
		printf( "Not find i2c addr\n" );
		return 1;
	}

	SetI2CAddr( vecI2cAddr[1], 1, 0 );
	SetI2C_Clock_A8018( 2000 );
	RUN_A8018_SW_CMD_TEST( vecI2cAddr[0], vecI2cAddr[1] );
#endif

	//bool res = HFST_CheckIsA8018ICPI2CAddr();

	USBComm_FinishEx();
	return true;
}