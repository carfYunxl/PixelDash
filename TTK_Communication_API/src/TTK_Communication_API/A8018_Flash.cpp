
#include <vector>
#include <utility>
#include <chrono>
#include <thread>
#include <windows.h>
#include <functional>

#include "TTK_Communication_API.h"
#include "BulkDll.h"

#define Flash3k			0xC00		// A8018 ISP Total Size
#define Flash6k			0x1800		// A8018 URAM/PRAM Size
#define Flash48k        0xC000		// A8018 Flash FW Total Size
#define Flash60k        0xF000		// A8018 Flash ISP Start Size
#define Flash61k		0xF400		// A8018 Flash ISP max program / erase size
#define Flash63k		0xFC00		// A8018 Flash Total Valid Size
#define Flash64k        0x10000		// A8018 Flash Total Size
#define PageSize1K      0x400		// A8018 ISP program page size
#define PageSize256		0x100		// A8018 ISP Non-Clock-Stretch program page size

#define A8018_ICP_ADDR_WRITE_FLASH_DATA		0xF552

#define A8018_NON_CLOCK_STRETCH_ISP	// for A8018 ISP V1.6 non clock stretch

constexpr unsigned int nMainBlock48kFFCRC = 0x502394;
extern unsigned char g_sI2cAddr;

extern TouchLinkVersion g_TouchLinkVersion;
enum {
	ERROR_HW_FLASH_UNLOCK = -600, ERROR_HW_FLASH_LOCK, ERROR_HW_ENABLE_CLK, ERROR_HW_CHECK_PASS_KEY, ERROR_HW_CHECK_IFREN, 
	ERROR_HW_TRIGGER_READ, ERROR_HW_TRIGGER_ERASE, ERROR_HW_TRIGGER_WRITE, ERROR_HW_ERASE_MASS_MAIN, ERROR_HW_CHECK_FUNC_DONE,
	ERROR_CHECK_CRC, ERROR_WRITE_DATA,
	ERROR_SW_FLASH_STATUS = -700
};

/*enum { ICP_WRITE_RAM = 0x00, ICP_WRITE_SFR = 0x01, ICP_READ_RAM = 0x80, ICP_READ_SFR = 0x81 };*/

#define CRC24_RADIX64_POLY 0x864CFB
UINT32 RemainderCal( UINT32 poly, UINT32 dat, UINT8 polysize, UINT8 datasize )
{
	UINT8 SHIFT_MAX = datasize - (polysize + 1);
	UINT32 MSBOFDATA = (UINT32)1 << (datasize - 1), tmp;
	poly |= (1 << polysize);
	for ( UINT8 ind = 0; ind <= SHIFT_MAX; ind++ ) {
		if ( (dat << ind) & MSBOFDATA ) { //if MSB == 1
			tmp = poly << (SHIFT_MAX - ind);
			dat ^= tmp; //poly dosen't include the MSB of the divider.
		}
	}
	return dat;// remainder is the lowest N bits
}

UINT32 Crc24Cal( UINT32 poly, UINT8 * dat, UINT32 dat_len )
{
	const char POLYSIZE = 24, DATASIZE = 8;
	UINT32 crc;
	UINT32 ind;
	crc = dat[0];
	for ( ind = 1; ind < dat_len; ind++ ) {
		crc = (crc << DATASIZE) | dat[ind];
		crc = RemainderCal( poly, crc, POLYSIZE, (UINT8)(POLYSIZE + DATASIZE) );
	}
	for ( ind = 0; ind < (POLYSIZE / DATASIZE); ind++ ) {// CRC24, the data should be shifted left 24bits. shift 8bit 3 times and calculate the remainder
		crc = crc << DATASIZE; // CRC24, shift left 8bit
		crc = RemainderCal( poly, crc, POLYSIZE, (UINT8)(POLYSIZE + DATASIZE) );
	}
	return crc;
}

void mSleep( int nMilliseconds )
{
	if ( nMilliseconds > 20 )
		return Sleep( nMilliseconds );

	auto end_point = std::chrono::system_clock::now() + std::chrono::milliseconds( nMilliseconds );
	while ( std::chrono::system_clock::now() < end_point )
		;
}

#pragma region A8018HW
int A8018HW_WriteRAM_TouchLink4( unsigned int nWriteAddr, const unsigned char * pDataBuf, unsigned int nDataSize, unsigned char nWriteType, BOOL bPARAM )
{
	static constexpr unsigned char A8018HW_BLOCK_SIZE = 128;	// After write 128 byte need delay 50us
	const unsigned char nBlockCount = (nDataSize + A8018HW_BLOCK_SIZE - 1) / A8018HW_BLOCK_SIZE;

	for ( int nBlockIdx = 0; nBlockIdx < nBlockCount; ++nBlockIdx ) {
		std::vector< unsigned char > vecCmdBuf;
		vecCmdBuf.reserve( BulkLen );

		int nWriteSize = A8018HW_BLOCK_SIZE;
		if ( (nBlockIdx + 1) * A8018HW_BLOCK_SIZE > nDataSize )
			nWriteSize = nDataSize - nBlockIdx * A8018HW_BLOCK_SIZE;

		vecCmdBuf.emplace_back( Bridge_T_Write_CMD );				// Touch Link CMD
		vecCmdBuf.emplace_back( (nWriteSize + 4) & 0xFF );          // Length L
		vecCmdBuf.emplace_back( ((nWriteSize + 4) >> 8) & 0xFF );   // Length H
		vecCmdBuf.emplace_back( 0x00 );								// I2C CMD
		vecCmdBuf.emplace_back( nWriteType );						// Write Addr
		vecCmdBuf.emplace_back( (nWriteAddr >> 8) & 0xFF );         // Write Addr
		vecCmdBuf.emplace_back( nWriteAddr & 0xFF );                // Write Addr

		size_t pos = 0;
		do {
			for ( ; pos < nWriteSize && vecCmdBuf.size() < BulkLen; ++pos )
				vecCmdBuf.emplace_back( *pDataBuf++ );

			if ( USBComm_WriteToBulkEx( &vecCmdBuf[0], BulkLen ) <= 0 )
				return ERRORMSG_WRITE_BLUK_FAIL;

			if ( pos >= nWriteSize )
				break;

			vecCmdBuf.clear();
			vecCmdBuf.emplace_back( Bridge_T_Continuous );
		} while ( 1 );

		if ( nBlockIdx != (nBlockCount - 1) )
			std::this_thread::sleep_for( std::chrono::microseconds( 100 ) );

		if ( bPARAM )
			nWriteAddr += nWriteSize;
	}
	
	return 1;
}

int A8018HW_WriteRAM( unsigned int nWriteAddr, const unsigned char * pDataBuf, unsigned int nDataSize, unsigned char nWriteType/* = ICP_WRITE_RAM*/, BOOL bPARAM/* = FALSE*/ )
//int A8018HW_WriteRAM( unsigned short addr, const unsigned char * Write_data, unsigned int length )
{
	// touch link 4 & touch link 5 fw < v2.05 use old protocol
	int nTouchLinkSW_Version = g_TouchLinkVersion.sw_major * 100 + g_TouchLinkVersion.sw_sub;
	if ( (g_TouchLinkVersion.hw <= 0x04) || ((g_TouchLinkVersion.hw == 0x05) && (nTouchLinkSW_Version < 205)) )
		return A8018HW_WriteRAM_TouchLink4( nWriteAddr, pDataBuf, nDataSize, nWriteType, bPARAM );

	std::vector< unsigned char > vecCmdBuf;
	vecCmdBuf.reserve( BulkLen );

	vecCmdBuf.emplace_back( Bridge_T_Write_CMD_A8018 );         // Touch Link CMD
	vecCmdBuf.emplace_back( (nDataSize + 4) & 0xFF );           // Length L
	vecCmdBuf.emplace_back( ((nDataSize + 4) >> 8) & 0xFF );    // Length H
	vecCmdBuf.emplace_back( 0x00 );                             // I2C CMD
	vecCmdBuf.emplace_back( nWriteType );						// Write Addr
	vecCmdBuf.emplace_back( (nWriteAddr >> 8) & 0xFF );         // Write Addr
	vecCmdBuf.emplace_back( nWriteAddr & 0xFF );                // Write Addr

	size_t pos = 0;
	do {
		for ( ; pos < nDataSize && vecCmdBuf.size() < BulkLen; ++pos )
			vecCmdBuf.emplace_back( *pDataBuf++ );

		if ( USBComm_WriteToBulkEx( &vecCmdBuf[0], BulkLen ) <= 0 )
			return ERRORMSG_WRITE_BLUK_FAIL;

		if ( pos >= nDataSize )
			break;

		vecCmdBuf.clear();
		vecCmdBuf.emplace_back( Bridge_T_Continuous );
	} while ( 1 );

	return 1;
}

int A8018HW_ReadRAM( unsigned short nReadAddr, unsigned char * pReadBuf, unsigned int nReadSize, unsigned char nReadType/* = ICP_READ_RAM*/ )
{
	unsigned char tmpWriteCmdBuf[4]{ 0 };
	tmpWriteCmdBuf[0] = 0x00;                 // I2C CMD
	tmpWriteCmdBuf[1] = nReadType;            // Write Addr
	tmpWriteCmdBuf[2] = (nReadAddr >> 8);     // Write Addr
	tmpWriteCmdBuf[3] = (nReadAddr & 0xFF);   // Write Addr

	if ( WriteCmd( tmpWriteCmdBuf, sizeof( tmpWriteCmdBuf ) ) <= 0 )
		return -1;

	unsigned char tmpReadCmdBuf[5]{ 0 };
	tmpReadCmdBuf[0] = Bridge_T_Read_CMD;		// CMD Read
	tmpReadCmdBuf[1] = 0x02;					// CMD Length L 
	tmpReadCmdBuf[2] = 0x00;					// CMD Length H
	tmpReadCmdBuf[3] = (nReadSize + 1) & 0xFF;	// Read Length L, A8018 ICP receive data have a dummy data, need read more 1 byte
	tmpReadCmdBuf[4] = ((nReadSize + 1) >> 8);	// Read Length H

	if ( WriteCmd_Advancd( tmpReadCmdBuf, sizeof( tmpReadCmdBuf ) ) <= 0 )
		return -2;
	
	unsigned int nCurrentReadSize = 0;
	unsigned char tmpReadDataBuf[BulkLen]{ 0 };
	auto s_time = std::chrono::system_clock::now();
	do {
		memset( tmpReadDataBuf, 0, BulkLen );
		if ( BulkLen != USBComm_ReadFromBulkEx( tmpReadDataBuf, BulkLen ) )
			return -3;

		switch ( tmpReadDataBuf[0] ) {
			case Bridge_R_I2C_Data:
				for ( int i = 5; i < BulkLen && nCurrentReadSize < nReadSize; ++i, ++nCurrentReadSize )
					pReadBuf[nCurrentReadSize] = tmpReadDataBuf[i];
				break;
			case Bridge_R_Continuous:
				for ( int i = 1; i < BulkLen && nCurrentReadSize < nReadSize; ++i, ++nCurrentReadSize )
					pReadBuf[nCurrentReadSize] = tmpReadDataBuf[i];
				break;
			default:
				printf( "result type: %02X\n", tmpReadDataBuf[0] );
				std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
				break;
		}

		if ( nCurrentReadSize >= nReadSize )
			break;

		auto e_time = std::chrono::system_clock::now();
		if ( std::chrono::duration_cast<std::chrono::milliseconds>(e_time - s_time).count() > 100 )
			return -3;
	} while ( 1 );

	return 1;
}

int A8018_SetPeripheralCLK_Reset( unsigned char nResetVal )
{
	constexpr unsigned short nResetAddr = 0xF678;
	unsigned char bufValue[]{ 0x00, nResetVal };

	if ( A8018HW_WriteRAM( nResetAddr, bufValue, sizeof( bufValue ) ) <= 0 )
		return -1;

	return 1;
}

int A8018_SetPeripheralCLK_Enable( unsigned char nEnableVal )
{
	constexpr unsigned short nEnableAddr = 0xF676;
	unsigned char bufValue[]{ 0x00, nEnableVal };

	if ( A8018HW_WriteRAM( nEnableAddr, bufValue, sizeof( bufValue ) ) <= 0 )
		return -1;

	return 1;
}

int A8018HW_FlashUnLock()
{
	// cmd format: mode | addr0 | addr1 | addr2 | data ...

	// 1. switch to address mode
	unsigned char cmd_1[]{ 0x00, 0x53, 0x71, 0x23, 0xA5, 0x3C };
	if ( WriteCmd( cmd_1, sizeof( cmd_1 ) ) <= 0 ) return -1;

	// 2. AFE Unlock
	unsigned char cmd_2[]{ 0x00, 0x53, 0x71, 0x23, 0x14, 0x55 };
	if ( WriteCmd( cmd_2, sizeof( cmd_2 ) ) <= 0 ) return -2;

	// 3. ICP Unlock
	unsigned char cmd_3[]{ 0x00, 0x53, 0x71, 0x23, 0x94, 0x55 };
	if ( WriteCmd( cmd_3, sizeof( cmd_3 ) ) <= 0 ) return -3;

	return 1;
}

int A8018HW_URAMUnLock()
{
	// 1. switch to address mode
	unsigned char cmd_1[]{ 0x00, 0x53, 0x71, 0x23, 0xA5, 0x3C };
	if ( WriteCmd( cmd_1, sizeof( cmd_1 ) ) <= 0 ) return -1;

	// 2. AFE Unlock
	unsigned char cmd_2[]{ 0x00, 0x53, 0x71, 0x23, 0x14, 0x55 };
	if ( WriteCmd( cmd_2, sizeof( cmd_2 ) ) <= 0 ) return -2;

	// 3. ICP Unlock
	unsigned char cmd_3[]{ 0x00, 0x53, 0x71, 0x23, 0x55, 0x55 };
	if ( WriteCmd( cmd_3, sizeof( cmd_3 ) ) <= 0 ) return -3;

	return 1;
}

int A8018HW_FlashLock()
{
	// 1. ICP Unlock
	unsigned char cmd_1[]{ 0x00, 0x53, 0x71, 0x23, 0x49, 0x4C };
	if ( WriteCmd( cmd_1, sizeof( cmd_1 ) ) <= 0 ) return -1;

	// 2. Set flash clock disable
	constexpr unsigned short nCLK_Addr = 0xF554;
	constexpr unsigned char cmd_2[]{ 0x00, 0x00 };
	if ( A8018HW_WriteRAM( nCLK_Addr, cmd_2, sizeof( cmd_2 ) ) <= 0 ) return -2;

	// 3. AFE Lock
	unsigned char cmd_3[]{ 0x00, 0x53, 0x71, 0x23, 0x41, 0x4C };
	if ( WriteCmd( cmd_3, sizeof( cmd_3 ) ) <= 0 ) return -3;

	// 4. Switch to CMD mode
	unsigned char cmd_4[]{ 0x00, 0x53, 0x71, 0x23, 0x5A, 0xC3 };
	if ( WriteCmd( cmd_4, sizeof( cmd_4 ) ) <= 0 ) return -4;

	return 1;
}

int A8018HW_URM_PassKey( unsigned short URAM_PASS_KEY)
{
	constexpr unsigned short nURAMPassKeyAddr = 0xF67A;

	unsigned char szPassKeyValue[] = { 0x00, URAM_PASS_KEY >> 8, 0x00, URAM_PASS_KEY & 0xFF };
	if ( A8018HW_WriteRAM( nURAMPassKeyAddr, szPassKeyValue, sizeof( szPassKeyValue ), ICP_WRITE_RAM ) <= 0 )
		return -1;
	
	return 1;
}

int A8018HW_CheckPassKey()
{
	constexpr unsigned short nPassKeyAddr = 0xF736;

	unsigned char bufRead[2]{ 0 };
	if ( A8018HW_ReadRAM( nPassKeyAddr, bufRead, 2 ) <= 0 )
		return -1;

	if ( 0x83 != bufRead[1] )
		return -2;

	return 1;
}

int A8018HW_CheckIFREN( bool bIFREN )
{
	constexpr unsigned short nIFREN_Addr = 0xF55A;

	unsigned char bufRead[2]{ 0 };
	if ( A8018HW_ReadRAM( nIFREN_Addr, bufRead, 2 ) <= 0 )
		return -1;

	unsigned char IFREN = bIFREN ? 0x01 : 0x00;
	if ( IFREN != bufRead[1] )
		return -2;

	return 1;
}

int A8018HW_SetStartAddrAndCheckIFREN( unsigned int nAddr, bool bIFREN )
{
	if ( bIFREN )
		nAddr -= Flash60k;
	nAddr /= 2;

	// Set block & start addr
	constexpr unsigned short nStartAddr = 0xF55A;
	unsigned char bufValue[]{ 0x04, bIFREN, (nAddr >> 8) & 0xFF, nAddr & 0xFF };

	if ( A8018HW_WriteRAM( nStartAddr, bufValue, sizeof( bufValue ) ) <= 0 )
		return -1;

	if ( A8018HW_CheckIFREN( bIFREN ) <= 0 )
		return -2;

	return 1;
}

int A8018HW_SetWriteDataSizeAndCheckIFREN( unsigned int nWriteSize, unsigned int nWriteAddr, bool bIFREN )
{
	constexpr unsigned short nWriteDataSizeAddr = 0xF558;

	unsigned char bufWriteValue[6]{ 0 };
	bufWriteValue[0] = ((nWriteSize >> 8) & 0x7F);  // Data Length
	bufWriteValue[1] = nWriteSize & 0xFF;

	bufWriteValue[2] = 0x04;						// Block selection
	bufWriteValue[3] = bIFREN;

	if ( bIFREN )
		nWriteAddr -= Flash60k;
	nWriteAddr /= 2;

	bufWriteValue[4] = ((nWriteAddr >> 8) & 0x7F);	// Data Addr
	bufWriteValue[5] = nWriteAddr & 0xFF;

	if ( A8018HW_WriteRAM( nWriteDataSizeAddr, bufWriteValue, sizeof( bufWriteValue ) ) <= 0 )
		return -1;

	if ( A8018HW_CheckIFREN( bIFREN ) <= 0 )
		return -2;

	return 1;
}

int A8018HW_SetReadTimeAndTrggerRead()
{
	unsigned char bufSetValue[6]{ 0 };
	bufSetValue[0] = 0x00;      // Tmain MSB
	bufSetValue[1] = 0x00;

	bufSetValue[2] = 0x00;      // Tmain LSB
	bufSetValue[3] = 0x02;

	bufSetValue[4] = 0x51;      // TimingParameter
	bufSetValue[5] = 0x55;

	constexpr unsigned short nSetReadTimeAddr = 0xF56C;
	if ( A8018HW_WriteRAM( nSetReadTimeAddr, bufSetValue, sizeof( bufSetValue ) ) <= 0 )
		return -1;

	constexpr unsigned short nTriggerReadAddr = 0xF56A;
	constexpr unsigned char bufTriggerValue[]{ 0x00, 0x01 };
	if ( A8018HW_WriteRAM( nTriggerReadAddr, bufTriggerValue, sizeof( bufTriggerValue ) ) <= 0 )
		return -2;

	return 1;
}

int A8018HW_SetCLKAndICPMode()
{
	unsigned char bufCLKValue[4]{ 0 };
	bufCLKValue[0] = 0x00;  // CLK enable
	bufCLKValue[1] = 0x01;

	bufCLKValue[2] = 0x00;  // ICP word mode
	bufCLKValue[3] = 0x01;

	constexpr unsigned short nCLKAddr = 0xF554;
	if ( A8018HW_WriteRAM( nCLKAddr, bufCLKValue, sizeof( bufCLKValue ) ) <= 0 )
		return -1;

	return 1;
}

int A8018HW_SetEraseTimeAndTrigger( bool bMassErase )
{
	unsigned char bufTimeValue[6]{ 0 };
	bufTimeValue[0] = 0x00;      // Tmain MSB
	bufTimeValue[1] = 0x0B;

	bufTimeValue[2] = 0x71;      // Tmain LSB
	bufTimeValue[3] = 0xB0;

	bufTimeValue[4] = 0x51;      // TimingParameter
	bufTimeValue[5] = 0x55;

	constexpr unsigned short nEraseTimeAddr = 0xF56C;
	if ( A8018HW_WriteRAM( nEraseTimeAddr, bufTimeValue, sizeof( bufTimeValue ) ) <= 0 )
		return -1;

	// Set ICP Flash pass words for erase
	unsigned char bufPassWord[6]{ 0 };
	bufPassWord[0] = 0x00;    // Program
	bufPassWord[1] = 0xA5;

	bufPassWord[2] = 0x00;    // Mass erase
	bufPassWord[3] = bMassErase ? 0x5A : 0xA5;

	bufPassWord[4] = 0x00;    // Erase
	bufPassWord[5] = bMassErase ? 0xA5 : 0x5A;

	constexpr unsigned short nPassWordAddr = 0xF55E;
	if ( A8018HW_WriteRAM( nPassWordAddr, bufPassWord, sizeof( bufPassWord ) ) <= 0 )
		return -2;

	// Set ICP Flash erase trigger
	unsigned short nEraseTriggerAddr = bMassErase ? 0xF566 : 0xF568;
	constexpr unsigned char bufTriggerVal[]{ 0x00, 0x01 };
	if ( A8018HW_WriteRAM( nEraseTriggerAddr, bufTriggerVal, sizeof( bufTriggerVal ) ) <= 0 )
		return -3;

	return 1;
}

int A8018HW_SetWriteAddrAndTrigger()
{
	unsigned char bufTimeValue[6]{ 0 };
	bufTimeValue[0] = 0x00;                    // Tmain MSB
	bufTimeValue[1] = 0x00;

	bufTimeValue[2] = 0x02;                    // Tmain LSB
	bufTimeValue[3] = 0x28;

	bufTimeValue[4] = 0x51;                    // TimingParameter
	bufTimeValue[5] = 0x55;

	constexpr unsigned short nTimeAddr = 0xF56C;
	if ( A8018HW_WriteRAM( nTimeAddr, bufTimeValue, sizeof( bufTimeValue ) ) <= 0 )
		return -1;

	unsigned char bufWriteValue[8]{ 0 };
	bufWriteValue[0] = 0x00;                          // Program
	bufWriteValue[1] = 0x5A;

	bufWriteValue[2] = 0x00;                          // Mass Erase
	bufWriteValue[3] = 0xA5;

	bufWriteValue[4] = 0x00;                          // Erase
	bufWriteValue[5] = 0xA5;

	bufWriteValue[6] = 0x00;                         // Set ICP Flash Program Trigger
	bufWriteValue[7] = 0x01;

	constexpr unsigned short nSetWriteAddr = 0xF55E;
	if ( A8018HW_WriteRAM( nSetWriteAddr, bufWriteValue, sizeof( bufWriteValue ) ) <= 0 )
		return -2;

	return 1;
}

int A8018HW_CheckFuncDone(const char * tag)
{
	constexpr unsigned short nFuncDoneAddr = 0xF572;

	unsigned char bufRead[2]{ 0 };

	for ( int i = 0; i < 10; ++i ) {
		if ( A8018HW_ReadRAM( nFuncDoneAddr, bufRead, 2 ) <= 0 )
			return -1;

		if ( 0x01 == bufRead[1] )
			break;

		std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
		printf( "%s retry ... %d\n", tag, i );
	}

	if ( 0x01 != bufRead[1] )
		return -2;

	return 1;
}

int A8018HW_EraseFlashHW_Mass( bool bMainBlock )
{
	unsigned short nBlockSelectionAddr = 0xF55A;
	unsigned char bufBlockValue[2]{ 0x40, 0x00 };

	if ( !bMainBlock ) {
		nBlockSelectionAddr = 0xF558;
		bufBlockValue[1] = 0x01;
	}

	if ( A8018HW_WriteRAM( nBlockSelectionAddr, bufBlockValue, sizeof( bufBlockValue ) ) <= 0 )
		return -1;

	if ( A8018HW_CheckIFREN( !bMainBlock ) <= 0 )
		return -2;

	if ( A8018HW_SetEraseTimeAndTrigger( bMainBlock ) <= 0 )
		return -3;

	std::this_thread::sleep_for( std::chrono::milliseconds( 40 ) );

	if ( A8018HW_CheckFuncDone("EraseFlashHW_Mass") <= 0 )
		return -4;

	return 1;
}

int A8018HW_CheckCRC( unsigned int nAddr, unsigned int nSize, unsigned int nCheckCRC )
{
	// Set enable check data
	constexpr unsigned short nEnableCheckAddr = 0xF58A;
	unsigned char bufValue[]{ 0x5A, 0x07 };
	if ( A8018HW_WriteRAM( nEnableCheckAddr, bufValue, sizeof( bufValue ) ) <= 0 )
		return -1;

	// Set ICP read all mode
	constexpr unsigned short nReadModeAddr = 0xF556;
	bufValue[0] = 0x0B;
	bufValue[1] = 0x01;

	if ( A8018HW_WriteRAM( nReadModeAddr, bufValue, sizeof( bufValue ) ) <= 0 )
		return -2;

	// Set ICP flash read Tmain
	unsigned char bufSetValue[6]{ 0 };
	bufSetValue[0] = 0x00;      // Tmain MSB
	bufSetValue[1] = 0x00;

	bufSetValue[2] = 0x00;      // Tmain LSB
	bufSetValue[3] = 0x01;

	bufSetValue[4] = 0x50;      // TimingParameter
	bufSetValue[5] = 0x55;

	constexpr unsigned short nSetReadTimeAddr = 0xF56C;
	if ( A8018HW_WriteRAM( nSetReadTimeAddr, bufSetValue, sizeof( bufSetValue ) ) <= 0 )
		return -3;

	// Set IFREN & read start address
	constexpr unsigned short nIFRENAddr = 0xF55A;
	bufSetValue[0] = 0x04;
	bufSetValue[1] = (nAddr >= Flash60k) ? 0x01 : 0x00;

	if ( nAddr >= Flash60k ) {
		bufSetValue[2] = (((nAddr - Flash60k) / 2) >> 8) & 0xFF;	// Start Addr
		bufSetValue[3] = ((nAddr - Flash60k) / 2) & 0xFF;
	} else {
		bufSetValue[2] = ((nAddr / 2) >> 8) & 0xFF;	// Start Addr
		bufSetValue[3] = (nAddr / 2) & 0xFF;
	}

	if ( A8018HW_WriteRAM( nIFRENAddr, bufSetValue, 4 ) <= 0 )
		return -4;

	// Set ICP read flash data length
	unsigned short nDataLengthAddr = 0xF57A;
	//if ( nAddr >= Flash60k ) nDataLengthAddr = 0xF57A;

	nSize = nSize / 2 - 1;
	bufSetValue[0] = (nSize >> 8) & 0xFF;
	bufSetValue[1] = nSize & 0xFF;
	if ( A8018HW_WriteRAM( nDataLengthAddr, bufSetValue, 2 ) <= 0 )
		return -5;

	// Set ICP read flash trig
	constexpr unsigned short nTrigAddr = 0xF56A;
	bufSetValue[0] = 0x00;
	bufSetValue[1] = 0x01;
	if ( A8018HW_WriteRAM( nTrigAddr, bufSetValue, 2 ) <= 0 )
		return -6;

	std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );

	constexpr unsigned short nCRCDoneAddr = 0xF580;
	unsigned char bufRead[4]{ 0 };
	for ( int i = 0; i < 10; ++i ) {
		if ( A8018HW_ReadRAM( nCRCDoneAddr, bufRead, 4 ) <= 0 )
			return -7;

		if ( 0x80 == bufRead[0] )
			break;

		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}

	if ( 0x80 != bufRead[0] )
		return -8;

	unsigned int nReadCRC = (bufRead[1] << 16) | (bufRead[2] << 8) | bufRead[3];
	if ( nReadCRC != nCheckCRC )
		return -9;

	return 1;
}

void A8018HW_ResetI2C_Clock( bool bSetClock, int nOriginalI2C_Clock )
{
	if ( bSetClock ) {
		// before hw operator flash, set i2c clock
		if ( g_TouchLinkVersion.hw < 5 ) {
			if ( 1400 != nOriginalI2C_Clock )
				SetI2C_Clock( 1400 );
		} else {
			if ( 700 != nOriginalI2C_Clock )
				SetI2C_Clock( 700 );
		}
	} else {
		// after hw operator flash, restore i2c clock
		SetI2C_Clock( nOriginalI2C_Clock );
	}
}

struct ScopeReleaseAction {
	ScopeReleaseAction( std::function<void()> func ) {
		_func = func;
	}
	~ScopeReleaseAction() {
		if ( _func )
			_func();
	}
	std::function<void()> _func;
};

// 读取地址可以随意，读取大小也可以随意
// 读取范围不能全在 IA 内
int ReadFlashHW_A8018( unsigned char * pReadData, unsigned int nAddr, unsigned int nReadSize )
{
	if ( !pReadData || (nReadSize <= 0) || (nAddr >= Flash63k) )
		return ERRORMSG_DATA_NULL;
	if ( (nAddr >= Flash48k) && ((nAddr + nReadSize) <= Flash60k) )
		return ERRORMSG_DATA_NULL;

	// 获取 Touch Link 版本，TL6 之前使用旧的通信命令
	if ( GetTouchLinkVersion( &g_TouchLinkVersion ) <= 0 )
		return ERRORMSG_BRIDGE_STATUS_BUS_FAULT;

	int nI2C_Clock = Get_I2C_Clock_KHz();
	A8018HW_ResetI2C_Clock( true, nI2C_Clock );
	
	ScopeReleaseAction tmpRestoreI2C_Clock( [nI2C_Clock] {
		A8018HW_ResetI2C_Clock( false, nI2C_Clock ); 
	} );

	int res = A8018HW_FlashUnLock();
	if ( res <= 0 ) return ERROR_HW_FLASH_UNLOCK;

	if ( A8018HW_SetCLKAndICPMode() <= 0 ) {
		A8018HW_FlashLock();
		return ERROR_HW_ENABLE_CLK;
	}

	if ( A8018HW_CheckPassKey() <= 0 ) {
		A8018HW_FlashLock();
		return ERROR_HW_CHECK_PASS_KEY;
	}

	unsigned int nMaxReadAddr = nAddr + nReadSize;
	nMaxReadAddr = min( nMaxReadAddr, Flash63k );

	bool bIFREN = false;
	unsigned int nCurrentReadSize = 0;

	unsigned short nRawDataAddr = 0xF550;
	while ( (nAddr < nMaxReadAddr) && (nAddr < Flash63k) )
	{
		if ( (nAddr < Flash48k) || (nAddr >= Flash60k) )
		{
			if ( nAddr >= Flash60k )
				bIFREN = true;

			if ( A8018HW_SetStartAddrAndCheckIFREN( nAddr, bIFREN ) <= 0 ) {
				A8018HW_FlashLock();
				return ERROR_HW_CHECK_IFREN;
			}

			if ( A8018HW_SetReadTimeAndTrggerRead() <= 0 ) {
				A8018HW_FlashLock();
				return ERROR_HW_TRIGGER_READ;
			}

			if ( nReadSize - nCurrentReadSize >= PageSize1K ) {
				A8018HW_ReadRAM( nRawDataAddr, &pReadData[nCurrentReadSize], PageSize1K );
			} else {
				A8018HW_ReadRAM( nRawDataAddr, &pReadData[nCurrentReadSize], nReadSize - nCurrentReadSize );
			}
			
			if ( A8018HW_CheckFuncDone( "ReadFlashHW_A8018" ) <= 0 ) {
				A8018HW_FlashLock();
				return ERROR_HW_CHECK_FUNC_DONE;
			}
		}
		
		nCurrentReadSize += PageSize1K;
		nAddr += PageSize1K;
	}

	A8018HW_FlashLock();
	return 1;
}

// 擦除必须从 1K 起始位置开始，并且烧录大小必须为 1K 整数倍
// 擦除范围不能全在 IA 内
// bNormalExitLockFlash: 接口正常退出时是否 Lock Flash
//	1. 若单独使用此库的擦除接口，此参数必须设为 true，
//	2. 若在此库的烧录接口中使用此接口需设为 falsh，否则接口结束后的 lock 会让 code 重跑可能引起 I2C 地址冲突的错误
//	3. 此参数仅在接口执行成功时生效，接口失败后默认会 Lock Flash
int EraseFlashHW_A8018( unsigned int nAddr, unsigned int nSize, bool bNormalExitLockFlash  )
{
	if ( (nAddr % PageSize1K) || (nSize % PageSize1K) || (nAddr >= Flash63k) )
		return ERRORMSG_DATA_NULL;

	// 范围不能全在 IA 内
	if ( (nAddr >= Flash48k) && ((nAddr + nSize) <= Flash60k) )
		return ERRORMSG_DATA_NULL;

	// 获取 Touch Link 版本，TL6 之前使用旧的通信命令
	if ( GetTouchLinkVersion( &g_TouchLinkVersion ) <= 0 )
		return ERRORMSG_BRIDGE_STATUS_BUS_FAULT;

	int nI2C_Clock = Get_I2C_Clock_KHz();
	A8018HW_ResetI2C_Clock( true, nI2C_Clock );

	ScopeReleaseAction tmpRestoreI2C_Clock( [nI2C_Clock] {
		A8018HW_ResetI2C_Clock( false, nI2C_Clock );
	} );

	int res = A8018HW_FlashUnLock();
	if ( res <= 0 ) return ERROR_HW_FLASH_UNLOCK;

	// Set Flash CLK enable
	constexpr unsigned short nCLKAddr = 0xF554;
	unsigned char bufCLKValue[]{ 0x00, 0x01 };
	if ( A8018HW_WriteRAM( nCLKAddr, bufCLKValue, sizeof( bufCLKValue ) ) <= 0 ) {
		A8018HW_FlashLock();
		return ERROR_HW_ENABLE_CLK;
	}

	if ( res = A8018HW_CheckPassKey(); res <= 0 ) {
		A8018HW_FlashLock();
		return ERROR_HW_CHECK_PASS_KEY;
	}

	std::vector< std::pair<unsigned int, unsigned int> > vecCheckCRC_Area;

	unsigned int nMaxEraseAddr = nAddr + nSize;
	nMaxEraseAddr = min( nMaxEraseAddr, Flash63k );

	if ( nAddr < Flash48k ) {
		unsigned nCRCSize = (nMaxEraseAddr > Flash48k) ? Flash48k - nAddr : nSize;
		vecCheckCRC_Area.push_back( { nAddr, nCRCSize } );
	}

	if ( nMaxEraseAddr > Flash60k ) {
		if ( nAddr >= Flash60k ) {
			vecCheckCRC_Area.push_back( { nAddr, nMaxEraseAddr >= Flash63k ? Flash63k - nAddr : nSize } );
		} else {
			vecCheckCRC_Area.push_back( { Flash60k, nMaxEraseAddr >= Flash63k ? Flash3k : nMaxEraseAddr - Flash60k } );
		}
	}
	
	unsigned int nCurrentEraseSize = 0;
	if ( (nAddr < PageSize1K) && (nSize >= Flash48k) ) {
		if ( A8018HW_EraseFlashHW_Mass( true ) <= 0 ) {
			A8018HW_FlashLock();
			return ERROR_HW_ERASE_MASS_MAIN;
		}
		nCurrentEraseSize += Flash48k;
		nAddr = Flash48k;
	}

	bool bIFREN = false;
	while ( nAddr < nMaxEraseAddr && nAddr < Flash63k ) {
		if ( (nAddr < Flash48k) || (nAddr >= Flash60k) )
		{
			if ( nAddr >= Flash60k )
				bIFREN = true;

			if ( A8018HW_SetStartAddrAndCheckIFREN( nAddr, bIFREN ) <= 0 ) {
				A8018HW_FlashLock();
				return ERROR_HW_CHECK_IFREN;
			}

			if ( A8018HW_SetEraseTimeAndTrigger( false ) <= 0 ) {
				A8018HW_FlashLock();
				return ERROR_HW_TRIGGER_ERASE;
			}

			// spec say at least delay 30ms
			std::this_thread::sleep_for( std::chrono::milliseconds( 35 ) );

			if ( A8018HW_CheckFuncDone("EraseFlashHW_A8018") <= 0 ) {
				A8018HW_FlashLock();
				return ERROR_HW_CHECK_FUNC_DONE;
			}
		}
		nAddr += PageSize1K;
	}

	unsigned char nDefaultFF[Flash48k];
	memset( nDefaultFF, 0xFF, Flash48k );

	for ( size_t idx = 0; idx < vecCheckCRC_Area.size(); ++idx ) {
		unsigned int nCheckCRC = Crc24Cal( CRC24_RADIX64_POLY, nDefaultFF, vecCheckCRC_Area[idx].second );
		if ( A8018HW_CheckCRC( vecCheckCRC_Area[idx].first, vecCheckCRC_Area[idx].second, nCheckCRC ) <= 0 ) {
			A8018HW_FlashLock();
			return ERROR_CHECK_CRC;
		}
	}

	if ( bNormalExitLockFlash )
		A8018HW_FlashLock();
	return 1;
}

// 烧录必须从 1K 起始位置开始，并且烧录大小必须为 1K 整数倍
// 烧录范围不能全在 IA 内
int WriteFlashHW_A8018( unsigned char * pWriteData, unsigned int nAddr, unsigned int nWriteSize )
{
	if ( !pWriteData || (nWriteSize <= 0) || (nAddr >= Flash63k) )
		return ERRORMSG_DATA_NULL;

	// 烧录必须从 1K 起始位置开始，并且烧录大小必须为 1K 整数倍
	if ( (nAddr % PageSize1K) || (nWriteSize % PageSize1K) )
		return ERRORMSG_DATA_NULL;

	// 范围不能全在 IA 内
	if ( (nAddr >= Flash48k) && ((nAddr + nWriteSize) <= Flash60k) )
		return ERRORMSG_DATA_NULL;

	if ( int ret = EraseFlashHW_A8018( nAddr, nWriteSize, false ); ret <= 0 )
		return ret;

	int nI2C_Clock = Get_I2C_Clock_KHz();
	A8018HW_ResetI2C_Clock( true, nI2C_Clock );

	ScopeReleaseAction tmpRestoreI2C_Clock( [nI2C_Clock] {
		A8018HW_ResetI2C_Clock( false, nI2C_Clock );
	} );

// 	int res = A8018HW_FlashUnLock();
// 	if ( res <= 0 ) return ERROR_HW_FLASH_UNLOCK;

// 	if ( res = A8018HW_CheckPassKey(); res <= 0 ) {
// 		A8018HW_FlashLock();
// 		return ERROR_HW_CHECK_PASS_KEY;
// 	}

	if ( A8018HW_SetCLKAndICPMode() <= 0 ) {
		A8018HW_FlashLock();
		return ERROR_HW_ENABLE_CLK;
	}

	unsigned char * pCheckData = pWriteData;
	unsigned int nMaxWriteAddr = nAddr + nWriteSize;
	nMaxWriteAddr = min( nMaxWriteAddr, Flash63k );

	std::vector< std::pair<unsigned int, unsigned int> > vecCheckCRC_Area;
	if ( nAddr < Flash48k ) {
		unsigned nCRCSize = (nMaxWriteAddr > Flash48k) ? Flash48k - nAddr : nWriteSize;
		vecCheckCRC_Area.push_back( { nAddr, nCRCSize } );
	}

	if ( nMaxWriteAddr > Flash60k ) {
		if ( nAddr >= Flash60k ) {
			vecCheckCRC_Area.push_back( { nAddr, nMaxWriteAddr >= Flash63k ? Flash63k - nAddr : nWriteSize } );
		} else {
			vecCheckCRC_Area.push_back( { Flash60k, nMaxWriteAddr >= Flash63k ? Flash3k : nMaxWriteAddr - Flash60k } );
		}
	}

	bool bIFREN = false;
	while ( (nAddr < nMaxWriteAddr) && (nAddr < Flash63k) ) {
		if ( (nAddr < Flash48k) || (nAddr >= Flash60k) )
		{
			if ( nAddr >= Flash60k )
				bIFREN = true;

			// icp program by words
			// program length = real words length - 1
			if ( int res = A8018HW_SetWriteDataSizeAndCheckIFREN( PageSize1K / 2 - 1, nAddr, bIFREN ); res <= 0 ) {
				A8018HW_FlashLock();
				return res;
			}

			if ( A8018HW_SetWriteAddrAndTrigger() <= 0 ) {
				A8018HW_FlashLock();
				return ERROR_HW_TRIGGER_WRITE;
			}

			if ( A8018HW_WriteRAM( A8018_ICP_ADDR_WRITE_FLASH_DATA, pWriteData, PageSize1K ) <= 0 ) {
				A8018HW_FlashLock();
				return ERROR_WRITE_DATA;
			}

			if ( A8018HW_CheckFuncDone( "WriteFlashHW_A8018" ) <= 0 ) {
				A8018HW_FlashLock();
				return ERROR_HW_CHECK_FUNC_DONE;
			}
		}
		pWriteData += PageSize1K;
		nAddr += PageSize1K;
	}

	for ( size_t idx = 0; idx < vecCheckCRC_Area.size(); ++idx ) {
		unsigned int nCheckCRC = Crc24Cal( CRC24_RADIX64_POLY, 
										   (0 == idx) ? pCheckData : &pCheckData[vecCheckCRC_Area[idx].first], 
										   vecCheckCRC_Area[idx].second );
		if ( A8018HW_CheckCRC( vecCheckCRC_Area[idx].first, vecCheckCRC_Area[idx].second, nCheckCRC ) <= 0 ) {
			A8018HW_FlashLock();
			return ERROR_CHECK_CRC;
		}
	}

	A8018HW_FlashLock();
	return 1;
}

// Word Write, only valid for information block
int WriteFlashHW_WR( unsigned char * data, unsigned int addr, unsigned int len )
{
	if ( !data || (len <= 0) || (addr < Flash60k) )
		return ERRORMSG_DATA_NULL;

	// 烧录地址与长度必须为偶数地址，最大允许 1K
	if ( (addr % 2) || (len % 2) || (len >= 0x400) )
		return ERRORMSG_DATA_NULL;

	// 获取 Touch Link 版本，TL6 之前使用旧的通信命令
	if ( GetTouchLinkVersion( &g_TouchLinkVersion ) <= 0 )
		return ERRORMSG_BRIDGE_STATUS_BUS_FAULT;

	int nI2C_Clock = Get_I2C_Clock_KHz();
	A8018HW_ResetI2C_Clock( true, nI2C_Clock );

	ScopeReleaseAction tmpRestoreI2C_Clock( [nI2C_Clock] {
		A8018HW_ResetI2C_Clock( false, nI2C_Clock );
	} );

	int res = A8018HW_FlashUnLock();
	if ( res <= 0 ) return ERROR_HW_FLASH_UNLOCK;

	if ( res = A8018HW_CheckPassKey(); res <= 0 ) {
		A8018HW_FlashLock();
		return ERROR_HW_CHECK_PASS_KEY;
	}

	if ( A8018HW_SetCLKAndICPMode() <= 0 ) {
		A8018HW_FlashLock();
		return ERROR_HW_ENABLE_CLK;
	}

	// icp program by words
	// program length = real words length - 1
	// information block WR address is base on 0xF000
	if ( res = A8018HW_SetWriteDataSizeAndCheckIFREN( len / 2 - 1, addr, true ); res <= 0 ) {
		A8018HW_FlashLock();
		return res;
	}

	if ( A8018HW_SetWriteAddrAndTrigger() <= 0 ) {
		A8018HW_FlashLock();
		return ERROR_HW_TRIGGER_WRITE;
	}

	if ( A8018HW_WriteRAM( A8018_ICP_ADDR_WRITE_FLASH_DATA, data, len ) <= 0 ) {
		A8018HW_FlashLock();
		return ERROR_WRITE_DATA;
	}

	if ( A8018HW_CheckFuncDone( "WriteFlashHW_A8018" ) <= 0 ) {
		A8018HW_FlashLock();
		return ERROR_HW_CHECK_FUNC_DONE;
	}	

	unsigned int nCheckCRC = Crc24Cal( CRC24_RADIX64_POLY, data, len );
	if ( A8018HW_CheckCRC( addr, len, nCheckCRC ) <= 0 ) {
		A8018HW_FlashLock();
		return ERROR_CHECK_CRC;
	}
	
	A8018HW_FlashLock();
	return 1;
}

int HFST_A8018_SDK_WriteURAMSwitchToPRAM( unsigned char * pWriteBuf, unsigned int nWriteSize )
{
	if ( !pWriteBuf || (nWriteSize <= 0) || (nWriteSize > Flash6k) )
		return ERRORMSG_DATA_NULL;

	// 获取 Touch Link 版本，TL6 之前使用旧的通信命令
	if ( GetTouchLinkVersion( &g_TouchLinkVersion ) <= 0 )
		return ERRORMSG_BRIDGE_STATUS_BUS_FAULT;

	int nI2C_Clock = Get_I2C_Clock_KHz();
	A8018HW_ResetI2C_Clock( true, nI2C_Clock );

	constexpr unsigned short URAM_PASS_KEY_ENABLE = 0x5A00;
	constexpr unsigned short URAM_PASS_KEY_SWITCH_FLASH = 0xA500;
	constexpr unsigned short URAM_PASS_KEY_SWITCH_PRAM = 0x5A01;

	ScopeReleaseAction tmpRestoreI2C_Clock( [nI2C_Clock] {
		A8018HW_ResetI2C_Clock( false, nI2C_Clock );
	} );

	int res = A8018HW_URAMUnLock();
	if ( res <= 0 ) return ERROR_HW_FLASH_UNLOCK;

	A8018_SetPeripheralCLK_Reset( 0x47 );				// hold Perif & Core Rstb = 0
	A8018_SetPeripheralCLK_Enable( 0x30 );				// Enable Perif & Core Rstb         
	A8018HW_URM_PassKey( URAM_PASS_KEY_ENABLE );        // Enable Passkey
	A8018HW_URM_PassKey( URAM_PASS_KEY_SWITCH_FLASH );  // Switch to Flash

	A8018_SetPeripheralCLK_Reset( 0x67 );
	A8018_SetPeripheralCLK_Enable( 0x30 );

	// Write data to URAM
	unsigned char bufRAMData[Flash6k];
	memset( bufRAMData, 0xFF, Flash6k );
	memcpy( bufRAMData, pWriteBuf, nWriteSize );

	unsigned int nWriteAddr = 0;
	do {
		if ( A8018HW_WriteRAM( nWriteAddr, &bufRAMData[nWriteAddr], PageSize1K, ICP_WRITE_RAM, TRUE ) <= 0 ) {
			A8018HW_FlashLock();
			return ERROR_WRITE_DATA;
		}
		nWriteAddr += PageSize1K;
	} while ( nWriteAddr < Flash6k );

	// Check URAM CRC
	constexpr unsigned short nSFR_Addr_D2 = 0x01A4;
	unsigned char nSFR_Value[]{ 0x00, 0x03 };

	// $D2[bit0] = 1 CMOD CRC mode select : memory (CRC24)
	// $D2[bit1] = 1 CRC24 memory select : RAM
	if ( A8018HW_WriteRAM( nSFR_Addr_D2, nSFR_Value, sizeof( nSFR_Value ), ICP_WRITE_SFR ) <= 0 )
		return -1;

	// CRC ROM Start Address Low
	constexpr unsigned short nSFR_Addr_D6 = 0x01AC;
	nSFR_Value[1] = 0x00;
	if ( A8018HW_WriteRAM( nSFR_Addr_D6, nSFR_Value, sizeof( nSFR_Value ), ICP_WRITE_SFR ) <= 0 )
		return -1;

	// CRC ROM Start Address High
	constexpr unsigned short nSFR_Addr_D7 = 0x01AE;
	if ( A8018HW_WriteRAM( nSFR_Addr_D7, nSFR_Value, sizeof( nSFR_Value ), ICP_WRITE_SFR ) <= 0 )
		return -1;

	// CRC Calculate Length High(Total 6K)
	constexpr unsigned short nSFR_Addr_D9 = 0x01B2;
	nSFR_Value[1] = 0x0C;
	if ( A8018HW_WriteRAM( nSFR_Addr_D9, nSFR_Value, sizeof( nSFR_Value ), ICP_WRITE_SFR ) <= 0 )
		return -1;

	// CRC Calculate Length Low(Note: Write $D8 trigger CRC24 calculation)
	constexpr unsigned short nSFR_Addr_D8 = 0x01B0;
	nSFR_Value[1] = 0x00;
	if ( A8018HW_WriteRAM( nSFR_Addr_D8, nSFR_Value, sizeof( nSFR_Value ), ICP_WRITE_SFR ) <= 0 )
		return -1;

	std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

	// Read CRC Status
	unsigned char nReadSFR_D2[2] = { 0 }, nRetryCount = 0;
	do {
		if ( A8018HW_ReadRAM( nSFR_Addr_D2, nReadSFR_D2, 2, ICP_READ_SFR ) <= 0 )
			return -1;

		//printf( "%s URAM CRC Status ... [0] = %02X [1] = %02X\n", __FUNCTION__, nReadSFR_D2[0], nReadSFR_D2[1] );
		if ( 0x02 == nReadSFR_D2[0] )
			break;
		
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	} while ( nRetryCount++ < 50 );

	// Read CRC
	constexpr unsigned short nSFR_Addr_D3 = 0x01A6;
	unsigned char nSFR_CRC_Value[6] = { 0 };
	if ( A8018HW_ReadRAM( nSFR_Addr_D3, nSFR_CRC_Value, 6, ICP_READ_SFR ) <= 0 )
		return -1;

	unsigned int nCheckCRC = Crc24Cal( CRC24_RADIX64_POLY, bufRAMData, Flash6k );
	unsigned int nHW_CRC = nSFR_CRC_Value[0] | (nSFR_CRC_Value[2] << 8) | (nSFR_CRC_Value[4] << 16);
	if ( nHW_CRC != nCheckCRC ) {
		A8018_SetPeripheralCLK_Reset( 0x77 );
		A8018_SetPeripheralCLK_Enable( 0x00 );
		return -1;
	}

	A8018_SetPeripheralCLK_Reset( 0x47 );
	A8018_SetPeripheralCLK_Reset( 0x67 );
	A8018HW_URM_PassKey( URAM_PASS_KEY_SWITCH_PRAM );
	A8018_SetPeripheralCLK_Reset( 0x77 );
	//A8018_SetPeripheralCLK_Enable( 0x00 );

	return 1;
}
#pragma endregion A8018HW

#pragma region A8018SW

enum A8018FlashStatus { Flash_NotReady = 0, Flash_Program, Flash_Erase, Flash_MassErase, Flash_Read };
int A8018_CheckFlashStatusCRCSW( unsigned char nFlashStatus, unsigned int nCheckCRC, unsigned char nDelayMS )
{
	unsigned char bufStatusCmd = 0x49;
	if ( WriteCmd( &bufStatusCmd, 0x01 ) <= 0 )
		return ERRORMSG_WRITE_BLUK_FAIL;

	if ( nDelayMS > 0 )
		mSleep( nDelayMS );

	unsigned char bufReadStatus[8]{ 0 };
	if ( ReadCmd( bufReadStatus, 8 ) <= 0 )
		return ERRORMSG_READ_BLUK_FAIL;

	if ( bufReadStatus[0] != nFlashStatus )
		return ERROR_SW_FLASH_STATUS;

	unsigned int nReadCRC = (bufReadStatus[1] << 16) | (bufReadStatus[2] << 8) | bufReadStatus[3];
	if ( nCheckCRC != nReadCRC )
		return ERROR_CHECK_CRC;

	return 1;
}

int A8018_ReadFlashSW_Block( unsigned char * pReadData, unsigned int nAddr, unsigned int nReadSize )
{
	// ISP read length by words
	// read length must 4 * n words
	unsigned int nISPReadSize = nReadSize / 2;
	if ( nReadSize % 8 ) nISPReadSize = (nReadSize + 7) / 8 * 4;

	int nCopyIdx = 0;
	if ( (nAddr < Flash48k) && ((nAddr + nISPReadSize * 2) > Flash48k) ) {
		nCopyIdx = nISPReadSize * 2 - nReadSize;
		nAddr = Flash48k - nISPReadSize * 2;
	}

	unsigned char bufReadFlash[6]{ 0 };
	bufReadFlash[0] = 0x84;							// CMD
	bufReadFlash[1] = 0x5A;							// Pass Code
	bufReadFlash[2] = (nAddr >> 8) & 0xFF;			// Start addr H
	bufReadFlash[3] = (nAddr & 0xFF);				// Start addr L
	bufReadFlash[4] = (nISPReadSize >> 8) & 0xFF;	// Read size H
	bufReadFlash[5] = (nISPReadSize & 0xFF);		// Read size L

	if ( WriteCmd( bufReadFlash, sizeof( bufReadFlash ) ) <= 0 )
		return ERRORMSG_WRITE_BLUK_FAIL;

	unsigned int nCheckCRC = 0;
	if ( nReadSize % 8 ) {
		unsigned char tmpReadData[PageSize1K];
		memset( tmpReadData, 0xFF, PageSize1K );

		if ( ReadCmd( tmpReadData, nISPReadSize * 2 ) <= 0 )
			return ERRORMSG_READ_PACKET_ERROR;

		nCheckCRC = Crc24Cal( CRC24_RADIX64_POLY, tmpReadData, nISPReadSize * 2 );
		if ( int res = A8018_CheckFlashStatusCRCSW( Flash_Read, nCheckCRC, 1 ); res <= 0 )
			return res;

		memcpy( pReadData, &tmpReadData[nCopyIdx], nReadSize );
 	} else {
 		if ( ReadCmd( pReadData, nReadSize ) <= 0 )
 			return ERRORMSG_READ_PACKET_ERROR;
 
 		nCheckCRC = Crc24Cal( CRC24_RADIX64_POLY, pReadData, nReadSize );
 		if ( int res = A8018_CheckFlashStatusCRCSW( Flash_Read, nCheckCRC, 1 ); res <= 0 )
 			return res;
 	}
	
	return 1;
}

int A8018_WriteFlashSW_Page( unsigned char nPageNum, unsigned char * pPageData, unsigned int nI2C_ClockKHz )
{
	unsigned char bufWriteCmd[3]{ 0x65, 0x5A, nPageNum };
	if ( nPageNum >= 0x3C ) bufWriteCmd[1] = 0xA5;	// isp block

	unsigned char nPageDelayTime = 1.0 / nI2C_ClockKHz * 9 * (PageSize256 + 1) + 6;

	// write cmd
	if ( WriteCmd( bufWriteCmd, sizeof( bufWriteCmd ) ) <= 0 )
		return ERRORMSG_WRITE_BLUK_FAIL;

#ifdef A8018_NON_CLOCK_STRETCH_ISP
	for ( int idx = 0; idx < 4; ++idx ) {
		// write data
		if ( WriteCmd( pPageData + PageSize256 * idx, PageSize256 ) <= 0 )
			return ERRORMSG_WRITE_BLUK_FAIL;

		mSleep( nPageDelayTime );
	}
#else

	// write data
	if ( WriteCmd( pPageData, PageSize1K ) <= 0 )
		return ERRORMSG_WRITE_BLUK_FAIL;

#endif

	// check crc
 	unsigned int nCheckCRC = Crc24Cal( CRC24_RADIX64_POLY, pPageData, PageSize1K );
 	if ( A8018_CheckFlashStatusCRCSW( Flash_Program, nCheckCRC, 0 ) <= 0 )
 		return ERROR_CHECK_CRC;

	return 1;
}

// 读取起始地址及大小可以任意
// 读取起始地址必须小于 0xFC00
// 读取范围不能全在 Illegal Areas 内
int ReadFlashSW_A8018( unsigned char * pReadData, unsigned int nAddr, unsigned int nReadSize )
{
	if ( !pReadData || (nReadSize <= 0) || (nAddr >= Flash63k) )
		return ERRORMSG_DATA_NULL;
	if ( nAddr >= Flash48k && ((nAddr + nReadSize) <= Flash60k) )
		return ERRORMSG_DATA_NULL;

	unsigned int nCurrentReadSize = 0;
	unsigned int nCheckCRC = 0;

	unsigned int nMaxReadAddr = nAddr + nReadSize;
	nMaxReadAddr = min( nMaxReadAddr, Flash63k );

	int res = 1;
	int nPageSize = PageSize1K;

	while ( (nAddr < nMaxReadAddr) && (nAddr < Flash63k) ) 
	{
		if ( (nAddr < Flash48k) || (nAddr >= Flash60k) )
		{
			if ( nCurrentReadSize + PageSize1K > nReadSize )
				nPageSize = nReadSize - nCurrentReadSize;

			res = A8018_ReadFlashSW_Block( &pReadData[nCurrentReadSize], nAddr, nPageSize ); 
			if ( res <= 0 ) return res;
		}
		nCurrentReadSize += nPageSize;
		nAddr += PageSize1K;
	}

	return 1;
}

// 烧录起始地址必须为 1K 起始位置，大小必须为 1K 整数倍
// 烧录起始地址必须小于 0xF400
int WriteFlashSW_A8018( unsigned char * pWriteData, unsigned int nWriteAddr, unsigned int nWriteSize )
{
	// 烧录必须从 1K 起始位置开始，并且烧录大小必须为 1K 整数倍
	if ( !pWriteData || (nWriteSize <= 0) || (nWriteAddr >= Flash61k) )
		return ERRORMSG_DATA_NULL;

	if ( (nWriteAddr % PageSize1K) || (nWriteSize % PageSize1K) )
		return ERRORMSG_DATA_NULL;

	int res = 1;
	if ( res = EraseFlashSW( nWriteAddr, nWriteSize ); res <= 0 )
		return res;

	int nI2C_ClockKHz = Get_I2C_Clock_KHz();
	nI2C_ClockKHz = max( 100, min( nI2C_ClockKHz, 2000 ) );

	unsigned int nMaxWriteAddr = nWriteAddr + nWriteSize;
	do {
		// program main block and isp first page
		if ( (nWriteAddr < Flash48k) || (nWriteAddr >= Flash60k) ) {
			if ( res = A8018_WriteFlashSW_Page( nWriteAddr / PageSize1K, pWriteData, nI2C_ClockKHz ); res <= 0 )
				return res;
		}

		nWriteAddr += PageSize1K;
		pWriteData += PageSize1K;
	} while ( (nWriteAddr < Flash61k) && (nWriteAddr < nMaxWriteAddr) );

	return 1;
}

// 擦除起始地址必须为 1K 起始位置，大小必须为 1K 整数倍
// 擦除起始地址必须小于 0xC000
int EraseFlashSW_A8018_Mass()
{
	unsigned char bufMassErase[]{ 0x9A, 0x5A };
	if ( WriteCmd( bufMassErase, sizeof( bufMassErase ) ) <= 0 )
		return ERRORMSG_WRITE_BLUK_FAIL;

	Sleep( 30 );

	if ( int res = A8018_CheckFlashStatusCRCSW( Flash_MassErase, nMainBlock48kFFCRC, 0 ); res <= 0 )
		return res;

	return 1;
}
#pragma endregion A8018SW

bool HFST_CheckIsA8018ICPI2CAddr( unsigned char i2cAddr )
{
	unsigned char lastI2cAddr = g_sI2cAddr;
	int lastI2cClock = Get_I2C_Clock_KHz();

	SetI2CAddr( i2cAddr, 1, false );
	SetI2C_Clock( 700 );

	bool res = true;
	if ( A8018HW_FlashUnLock() <= 0 ) {
		res = false;
		goto __exit;
	}

	if ( A8018_SetPeripheralCLK_Reset( 0x47 ) <= 0 ) {
		res = false;
		goto __exit;
	}

	if ( A8018_SetPeripheralCLK_Enable( 0x30 ) <= 0 ) {
		res = false;
		goto __exit;
	}

	if ( A8018HW_CheckPassKey() <= 0 ) {
		res = false;
		goto __exit;
	}

__exit:
	A8018HW_FlashLock();

	SetI2CAddr( lastI2cAddr, 0xFF, 0 );
	SetI2C_Clock( lastI2cClock );

	return res;
}