#ifndef __TTK_COMMUNICATION_API_HFST_ADB_SDK_WRAPPER_H__
#define __TTK_COMMUNICATION_API_HFST_ADB_SDK_WRAPPER_H__

#include <string>
#include <filesystem>
#include <fstream>
#include "TTK_Communication_API.h"
#include "HFST_ADB_SDK.h"
#include "HFST_ADB_SDK_ErrorCodeDef.h"

extern int ChecksumCalculation( unsigned short * pChecksum, unsigned char * pInData, unsigned long nLen );
static HANDLE g_hAdbContext = nullptr;

constexpr unsigned short MAX_ADB_PKT_SIZE = 0xFF;
constexpr char ADB_CLIENT_CMD[]{ "/data/httu/sitronix_client " };	// transmit data through this interface

#define OLD_PROTOCOL

#define		Flash16k	0x4000
#define		Flash33k	0x8400
#define		Flash48k	0xC000
#define		Flash60k	0xF000
#define		Flash61k	0xF400
#define		Flash63k	0xFC00
#define		Flash64k	0x10000
#define		Flash128k	0x20000

#define		PageSize1k	0x400
#define		PageSize4k	0x1000


int ADB_Init( bool bAdbRunAsRoot, const char * pAdbFilePath, const char * pExtraService )
{
#ifdef OPEN_ADB
	std::filesystem::path adb_path( pAdbFilePath );
	if ( !std::filesystem::exists( adb_path ) )
		return -1;

	adb_path.remove_filename();

	std::filesystem::path client_file = adb_path / "data\\sitronix_client";
	std::filesystem::path server_file = adb_path / "data\\sitronix_server";
	std::filesystem::path cfg_file = adb_path / "data\\HopingTool_CFG.ini";

	if ( !std::filesystem::exists( client_file ) || !std::filesystem::exists( server_file ) || !std::filesystem::exists( cfg_file ) )
		return -2;

	std::string push_client( "push " ), push_server( "push " ), push_cfg( "push " );
	push_client.append( client_file.string() ).append( " /data/httu" );
	push_server.append( server_file.string() ).append( " /data/httu" );
	push_cfg.append( cfg_file.string() ).append( " /data/httu" );
	
	const char * init_cmd[]{ "shell kill -9 `pgrep sitronix`",

							 "shell mkdir /data/httu",
							 "shell rm /data/httu/sitronix_client",
							 "shell rm /data/httu/sitronix_server",
							 "shell rm /data/httu/HopingTool_CFG.ini",

							 push_client.c_str(),
							 push_server.c_str(),
							 push_cfg.c_str(),

							 "shell chmod 777 /data/httu/sitronix_client",
							 "shell chmod 777 /data/httu/sitronix_server",
							 "shell chmod 777 /data/httu/HopingTool_CFG.ini" };

	std::filesystem::path root = std::filesystem::current_path() / "platform-tools" / "adb.exe";

	for (int idx = 0; idx < _countof(init_cmd); ++idx) {
		HFST_ADB_SDK_RunCmd( root.string().c_str(), init_cmd[idx] );
	}
	system("taskkill /F /FI \"IMAGENAME eq adb.exe\"");
	return HFST_ADB_SDK_Open( &g_hAdbContext, bAdbRunAsRoot, pAdbFilePath, pExtraService );
#else
	return 1;
#endif
}

int ADB_UnInit()
{
#ifdef OPEN_ADB
	return HFST_ADB_SDK_Close( g_hAdbContext );
#else
	return 1;
#endif
}

std::string ConvertADB_CMD( const std::vector< unsigned char > & data )
{
	std::string cmd = ADB_CLIENT_CMD;
	for ( const unsigned char v : data ) {
		char tmp_buf[4]{ 0 };
		snprintf( tmp_buf, 4, "%02X", v );
		cmd.append( tmp_buf );
	}
	return cmd;
}

/**
 * @brief Read adb data from server
 * @param data receive data buffer
 * @param len  the data length of result
 * @return 1 pass < 0 fail
 * @note 1. read cmd have 2 result format
 *		   1.1 [HFST] PASS dataSize data1, data2, ...
 *		   1.2 [HFST] FAIL errCode
 *		 2. write cmd have 2 result format
 *		   2.1 [HFST] PASS
 *		   2.2 [HFST] FAIL errCode
 * if write cmd read data, the param 'data' must nullptr
 */
int ReadPackage_ADB( unsigned char * data, unsigned int len )
{
#ifdef OPEN_ADB
	DWORD dwStartTime = GetTickCount();
	int nDataSize = 0;

	constexpr unsigned short ReadSize_ADB = 0x400;			// read size of every adb read
	unsigned char tmp_ReadBuf[ReadSize_ADB]{ 0 };

	CString strRemainReadData = _T( "" );					// adb data recv from cmd windows by text, so data usually
															// not a complete data, need user manual combination
	do {
		// if success, res is current read data length
		int res = HFST_ADB_SDK_Read( g_hAdbContext, tmp_ReadBuf, sizeof( tmp_ReadBuf ) );

		if ( res <= 0 ) {
			_cprintf( "[ERR-R]\r\n" );
			return res;
		}

		// Append last remain data
		CString tmpStrTouchData( (char *)tmp_ReadBuf, res );
		if ( tmpStrTouchData.IsEmpty() ) continue;

		if ( !strRemainReadData.IsEmpty() ) {
			tmpStrTouchData = strRemainReadData + tmpStrTouchData;
			strRemainReadData = _T( "" );
		}

		// Valid: [HFST] PASS dataSize 0x1234 0x2345 ...
		//        [HFST] FAIL errcode
		int pos = 0;
		CString tmpStrLineData = tmpStrTouchData.Tokenize( _T( "\r\n" ), pos );
		if ( pos > tmpStrTouchData.GetLength() ) {
			// not receive complete data
			strRemainReadData = tmpStrTouchData;
			continue;
		}

		int linePos = 0;

		// parse [HFST] 
		CString tmpStrDataTag = tmpStrLineData.Tokenize( _T( " " ), linePos );
		if ( "[HFST]" != tmpStrDataTag ) {
			// not adb server return data, show message remind we check whether regular
			_cprintf( "[Adb-Tag] invalid data: %s\n", (LPSTR)CT2A( tmpStrLineData ) );
			break;
		}

		// PASS or FAIL
		CString tmpStrResultTag = tmpStrLineData.Tokenize( " ", linePos );
		if ( -1 == linePos || tmpStrResultTag.IsEmpty() ) {
			_cprintf( "[Adb-Result] invalid data: %s\n", (LPSTR)CT2A( tmpStrLineData ) );
			break;
		}

		// if success, write cmd only have tag & result
		if ( ("PASS" == tmpStrResultTag) && !data )
			return 1;

		// dataSize or errCode
		CString tmpStrDataSize = tmpStrLineData.Tokenize( " ", linePos ).Trim();
		if ( -1 == linePos || tmpStrDataSize.IsEmpty() ) {
			_cprintf( "[Adb-Size] invalid data: %s\n", (LPSTR)CT2A( tmpStrLineData ) );
			break;
		}

		int nResultSize = _tcstol( tmpStrDataSize, nullptr, 16 );

		// if error, data size if error code
		if ( "FAIL" == tmpStrResultTag ) {
			_cprintf( "[Adb-FAIL] data: %s\n", (LPSTR)CT2A( tmpStrLineData ) );
			return nResultSize;
		}

		// parse data
		do {
			CString strValue = tmpStrLineData.Tokenize( " ", linePos );
			if ( -1 == linePos || strValue.IsEmpty() ) break;

			data[nDataSize++] = _tcstol( strValue, nullptr, 16 );
		} while ( 1 );	// parse line data

		break;
	} while ( GetTickCount() - dwStartTime < 500 );	// once write must read all data

	if ( nDataSize != len )
		return -1;
#endif
	return 1;
}

int ReadFlash_ADB_Private( unsigned char * data, unsigned int addr, unsigned int len )
{
#ifdef OPEN_ADB
	std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.push_back(SWISP_Operation);				// read cmd
	vecWriteBuf.push_back(0x05);						// package length low
	vecWriteBuf.push_back(0x00);						// package length high
	vecWriteBuf.push_back(ADB_SW_ISP_READ);				// adb SW Flash read
	vecWriteBuf.push_back(addr & 0xFF);					// read address low
	vecWriteBuf.push_back((addr >> 8) & 0xFF);			// read address high
	vecWriteBuf.push_back(len & 0xFF);					// read length low
	vecWriteBuf.push_back((len >> 8) & 0xFF);			// read length high
	std::string cmd_buf = ConvertADB_CMD(vecWriteBuf);
	int res = HFST_ADB_SDK_Write(g_hAdbContext, cmd_buf.c_str(), cmd_buf.length());
	if (res < 0) return res;

	std::filesystem::path sPath = std::filesystem::current_path();
	std::string cmd("pull /data/httu/flash.dump ");
	cmd.append(sPath.string());

	std::filesystem::path adbPath = sPath / "platform-tools" / "adb.exe";
	sPath /= "flash.dump";

	// check every 0.5 seconds to see whether pull flash.dump success
	int nWaitTime = 0;
	while (!std::filesystem::exists(sPath))
	{
		res = HFST_ADB_SDK_RunCmd(adbPath.string().c_str(), cmd.c_str());
		if (HFST_ADB_ERROR_OK != res) {
			_cprintf("[ERR-%d] W1 %s\r\n", res, cmd.c_str());
			return res;
		}
		Sleep(500);
		nWaitTime++;
		_cprintf("[read] waiting for read finish, now cost: %.1fs\r\n", nWaitTime * 0.5);

		if (nWaitTime == 40) return ERRORMSG_ADB_READ_TIMEOUT;
	};

	std::fstream ifs(sPath, std::ios::in | std::ios::binary);
	if (!ifs.is_open())
	{
		return ERRORMSG_ADB_FILE_OPEN_FILE;
	}

	ifs.seekg(0, std::ios::end);
	size_t size = ifs.tellg();
	if (size == -1)
	{
		return ERRORMSG_ADB_READ_LENGTH_ERROR;
	}
	ifs.seekg(0, std::ios::beg);
	ifs.read((char*)data, size);
	ifs.close();

	// delete temporary file : flash.dump
	std::filesystem::remove_all(sPath);

	//// delete temporary file of adb: /data/httu/flash.dump
	cmd = "shell rm //data//httu//flash.dump";
	res = HFST_ADB_SDK_RunCmd(adbPath.string().c_str(), cmd.c_str());

	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d] W1 %s\r\n", res, cmd_buf.c_str());
		return res;
	}
	res = ReadPackage_ADB(nullptr, len);
	return res;
#else
	return 1;
#endif
}

int WriteFlash_ADB_Private(unsigned char* data, unsigned int addr, unsigned int len)
{
#ifdef OPEN_ADB
	std::filesystem::path sPath = std::filesystem::current_path() / "flash.dump";
	if (std::filesystem::exists(sPath))
		std::filesystem::remove_all(sPath);

	std::ofstream ofs(sPath, std::ofstream::out | std::ofstream::binary);
	if (!ofs.is_open())
	{
		return ERRORMSG_ADB_FILE_OPEN_FILE;
	}
	ofs.write((const char*)data, len);
	ofs.close();

	std::string cmd("push ");
	cmd.append(sPath.string());
	cmd.append(" /data/httu/ ");

	std::filesystem::path adbPath = std::filesystem::current_path() / "platform-tools" / "adb.exe";
	int res = HFST_ADB_SDK_RunCmd(adbPath.string().c_str(), cmd.c_str());

	// 这里要判断文件是否已经推送到整机，再给整机下CMD
	Sleep(30);

	std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.push_back(SWISP_Operation);				// Flash R/W cmd
	vecWriteBuf.push_back(0x05);						// package length low
	vecWriteBuf.push_back(0x00);						// package length high
	vecWriteBuf.push_back(ADB_SW_ISP_WRITE);			// adb SW Flash write
	vecWriteBuf.push_back(addr & 0xFF);					// write address low
	vecWriteBuf.push_back((addr >> 8) & 0xFF);			// write address high
	vecWriteBuf.push_back(len & 0xFF);					// write length low
	vecWriteBuf.push_back((len >> 8) & 0xFF);			// write length high

	std::string cmd_buf = ConvertADB_CMD(vecWriteBuf);
	res = HFST_ADB_SDK_Write(g_hAdbContext, cmd_buf.c_str(), cmd_buf.length());

	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d] W1 %s\r\n", res, cmd_buf.c_str());
		return res;
	}

	if (std::filesystem::exists(sPath))
		std::filesystem::remove_all(sPath);

	 //delete temporary file of adb: /data/httu/flash.dump
	cmd = "shell rm //data//httu//flash.dump";
	res = HFST_ADB_SDK_RunCmd(adbPath.string().c_str(), cmd.c_str());

	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d] W1 %s\r\n", res, cmd_buf.c_str());
		return res;
	}

	res = ReadPackage_ADB(nullptr, len);
	return res;
#else
	return 1;
#endif
}

int ReadFlashSW_ADB(unsigned char* data, unsigned int addr, unsigned int len)
{
	if (!g_hAdbContext)
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;
	else if (!data)
		return ERRORMSG_DATA_NULL;

	if (addr < 0) return ERRORMSG_DATA_NULL;

	unsigned int maxSize = 0;
	int res = 0;

#if IC_Module == IC_Module_A8018
	maxSize = 0xFC00;
#elif IC_Module == IC_Module_A8015
	maxSize = 0x8400;
#elif IC_Module == IC_Module_A8008
	maxSize = 0x4000;
#elif IC_Module == IC_Module_A8010
	maxSize = 0x10000;
#elif IC_Module == IC_Module_ST1802
	maxSize = 0x20000;
#endif
	int maxLen = addr + len;
	if (maxLen > maxSize) return ERRORMSG_DATA_NULL;

#if IC_Module == IC_Module_A8018
	if ( maxLen <= Flash48k )
	{
		res = ReadFlash_ADB_Private(data, addr, len);
	}
	else if (maxLen > Flash48k && maxLen <= Flash60k) //如果读取的Flash区间包含了invalid area,则要特别处理
	{
		res = ReadFlash_ADB_Private(data, addr, Flash48k-addr);
		memset( &data[Flash48k - addr], 0xFF, maxLen - Flash48k);//填充0xFF
	}
	else
	{
		if (addr < Flash48k)
		{
			res = ReadFlash_ADB_Private( data, addr, Flash48k-addr );
			memset( &data[Flash48k - addr], 0xFF, Flash60k- Flash48k);//填充0xFF
			res = ReadFlash_ADB_Private( &data[Flash60k-addr], Flash60k, maxLen-Flash60k );
		}
		else if(addr < Flash60k)
		{
			memset( data, 0xFF, Flash60k-addr );//填充0xFF
			res = ReadFlash_ADB_Private( &data[Flash60k - addr], Flash60k, maxLen-Flash60k );
		}
		else
		{
			res = ReadFlash_ADB_Private( data, addr, len );
		}
	}
#else
	res = ReadFlash_ADB_Private(data, addr, len);
#endif

	return res;
}

int WriteFlash_ADB(unsigned char* data, unsigned int addr, unsigned int len)
{
	if (!g_hAdbContext)
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;
	if ( !data || len <= 0 )
		return ERRORMSG_DATA_NULL;

	int nMaxFlashSize = 0;
	int nPageSize = 0x400;
	int res = 0;
#if IC_Module == IC_Module_A8018
	nMaxFlashSize = Flash63k;
#elif IC_Module == IC_Module_A8015
	nMaxFlashSize = Flash33k;
#elif IC_Module == IC_Module_A8008
	nMaxFlashSize = Flash16k;
#elif IC_Module == IC_Module_A8010
	nMaxFlashSize = Flash64k;
#elif IC_Module == IC_Module_ST1802
	nMaxFlashSize = Flash128k;
	nPageSize = PageSize4k;
#endif
	if ( len > Flash61k )
		return ERRORMSG_DATA_NULL;

	if ((addr % nPageSize) || (len % nPageSize))
		return ERRORMSG_DATA_NULL;

	res = WriteFlash_ADB_Private(data, addr, len);

	return res;
}

int ReadI2CReg_ProtocolA_ADB( unsigned char * data, unsigned int addr, unsigned int len )
{
#ifdef OPEN_ADB
	if ( !g_hAdbContext || len > MAX_ADB_PKT_SIZE )
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.clear();

	vecWriteBuf.push_back( Bridge_T_Read_Reg );				// read cmd
	vecWriteBuf.push_back( 0x03 );							// package length low
	vecWriteBuf.push_back( 0x00 );							// package length high
	vecWriteBuf.push_back( addr & 0xFF );					// read address
	vecWriteBuf.push_back( len & 0xFF );					// read length low
	vecWriteBuf.push_back( len >> 8 );						// read length high

	std::string cmd_buf = ConvertADB_CMD( vecWriteBuf );
	int res = HFST_ADB_SDK_Write( g_hAdbContext, cmd_buf.c_str(), cmd_buf.length() );

	if ( HFST_ADB_ERROR_OK != res ) {
		_cprintf( "[ERR-%d] W1 %s\r\n", res, cmd_buf.c_str() );
		return res;
	}

	res = ReadPackage_ADB(data, len);
	if ( res < 0 )
	{
		for (int i = 0; i < 3; ++i)
		{
			res = ReadPackage_ADB(data, len);
			if (res > 0)
			{
				break;
			}
		}

	}
	return res;
#else
	return 1;
#endif
}

int WriteI2CReg_ProtocolA_ADB( unsigned char * data, unsigned int addr, unsigned int len )
{
#ifdef OPEN_ADB
	if ( !g_hAdbContext || len > MAX_ADB_PKT_SIZE )
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	int nWriteSize = len + 1;

	std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.clear();

	vecWriteBuf.push_back( Bridge_T_Write_Reg );    // write command
	vecWriteBuf.push_back( nWriteSize & 0xFF );		// write data size
	vecWriteBuf.push_back( nWriteSize >> 8 );		// address high
	vecWriteBuf.push_back( addr & 0xFF );			// address low

	// copy write data
	for ( int i = 0; i < len; ++i )
		vecWriteBuf.push_back( data[i] );

	std::string cmd_buf = ConvertADB_CMD( vecWriteBuf );
	int res = HFST_ADB_SDK_Write( g_hAdbContext, cmd_buf.c_str(), cmd_buf.length() );

	if ( HFST_ADB_ERROR_OK != res ) {
		_cprintf( "[ERR-%d] W2 %s\r\n", res, cmd_buf.c_str() );
		return res;
	}

	res = ReadPackage_ADB( nullptr, len );
	return res;
#else
	return 1;
#endif
}

int WriteCmd_ADB( unsigned char * data, unsigned short len )
{
#ifdef OPEN_ADB
	if ( !g_hAdbContext || len > MAX_ADB_PKT_SIZE )
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.clear();
	
	vecWriteBuf.push_back( Bridge_T_Write_CMD );    // write command
	vecWriteBuf.push_back( len & 0xFF );			// write data length low
	vecWriteBuf.push_back( len >> 8 );				// write data length high

	// copy write data
	for ( int i = 0; i < len; ++i )
		vecWriteBuf.push_back( data[i] );

	std::string cmd_buf = ConvertADB_CMD( vecWriteBuf );
	int res = HFST_ADB_SDK_Write( g_hAdbContext, cmd_buf.c_str(), cmd_buf.length() );

	if ( HFST_ADB_ERROR_OK != res ) {
		_cprintf( "[ERR-%d] W3 \r\n", res );
		return res;
	}

	res = ReadPackage_ADB( nullptr, len );
	return res;
#else
	return 1;
#endif
}

int ReadCmd_ADB( unsigned char * data, unsigned short len )
{
#ifdef OPEN_ADB
	if ( !g_hAdbContext || len > MAX_ADB_PKT_SIZE )
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.clear();

	vecWriteBuf.push_back( Bridge_T_Read_CMD );				// read cmd
	vecWriteBuf.push_back( 0x02 );							// package length low
	vecWriteBuf.push_back( 0x00 );							// package length high
	vecWriteBuf.push_back( len & 0xFF );					// read length low
	vecWriteBuf.push_back( len >> 8 );						// read length high

	std::string cmd_buf = ConvertADB_CMD( vecWriteBuf );
	int res = HFST_ADB_SDK_Write( g_hAdbContext, cmd_buf.c_str(), cmd_buf.length() );

	if ( HFST_ADB_ERROR_OK != res ) {
		_cprintf( "[ERR-%d] W4 %s\r\n", res, cmd_buf.c_str() );
		return res;
	}

	res = ReadPackage_ADB( data, len );
	return res;
#else
	return 1;
#endif
}

int EXPORT_API ADB_StartRecordRawData( FM_ReadRawdata * ctx )
{
#ifdef OPEN_ADB
	if ( !g_hAdbContext || !ctx )
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	std::vector< unsigned char > vecWriteBuf{ 0x56, 0x00, 0x00, 0x00 };

	vecWriteBuf.push_back( ctx->type );  // interface type
	vecWriteBuf.push_back( ctx->addr );  // read raw data address

	// user want read raw data type, length and times
	for ( const auto & data : ctx->readRawdataType ) {
		vecWriteBuf.push_back( data.dataType );      // raw data type, typical mutual(04), self(06) ...
		vecWriteBuf.push_back( data.dataLength );    // raw data type length
		vecWriteBuf.push_back( data.dataNum );       // raw data type read times
	}

	vecWriteBuf.push_back( ctx->defaultReadLength ); // undefined raw data type length to read
	unsigned int tmpValidDataSize = vecWriteBuf.size() - 3; // exclude command size

	vecWriteBuf[1] = (tmpValidDataSize & 0xFF);
	vecWriteBuf[2] = ((tmpValidDataSize & 0xFF00) >> 8);

	std::string cmd_buf = ConvertADB_CMD( vecWriteBuf );
	int res = HFST_ADB_SDK_Write( g_hAdbContext, cmd_buf.c_str(), cmd_buf.length() );

	if ( HFST_ADB_ERROR_OK != res ) {
		_cprintf( "[ERR-%d] W5 %s\r\n", res, cmd_buf.c_str() );
		return res;
	}

	res = ReadPackage_ADB( nullptr, 0 );
	return res;
#else
	return 1;
#endif
}

int EXPORT_API ADB_StopRecordRawData( const char * pAdbFilePath, const char * pSavePath )
{
#ifdef OPEN_ADB
	if ( !pAdbFilePath || !g_hAdbContext || !pSavePath )
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	std::vector< unsigned char > vecWriteBuf{ 0x56, 0x01, 0x00, 0x01 };

	std::string cmd_buf = ConvertADB_CMD( vecWriteBuf );
	int res = HFST_ADB_SDK_Write( g_hAdbContext, cmd_buf.c_str(), cmd_buf.length() );

	if ( HFST_ADB_ERROR_OK != res ) {
		_cprintf( "[ERR-%d] W6 %s\r\n", res, cmd_buf.c_str() );
		return res;
	}

	res = ReadPackage_ADB( nullptr, 0 );

	std::string cmd( "pull /data/httu/RawData.txt " );
	cmd.append( pSavePath );

	res = HFST_ADB_SDK_RunCmd( pAdbFilePath, cmd.c_str() );
	if ( HFST_ADB_ERROR_OK != res ) {
		_cprintf( "[ERR-%d]Pull raw data error\n", res );
		return res;
	}

	std::string remove_file_buf( "rm /data/httu/RawData.txt" );
	HFST_ADB_SDK_Write( g_hAdbContext, remove_file_buf.c_str(), remove_file_buf.length() );

	return res;
#else
	return 1;
#endif
}

#endif // __TTK_COMMUNICATION_API_HFST_ADB_SDK_WRAPPER_H__