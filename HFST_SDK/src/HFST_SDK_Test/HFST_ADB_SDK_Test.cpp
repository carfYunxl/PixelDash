// HFST_ADB_SDK_Test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>

#include "../../include/HFST_ADB_SDK/HFST_ADB_SDK.h"
#include "../../include/HFST_ADB_SDK/HFST_ADB_SDK_ErrorCodeDef.h"

#pragma comment(lib, "../Debug/HFST_ADB_SDK.lib")

#include <vector>
#include <string>
#include <filesystem>

// James 2021/07/19 for support 2152 fast read raw data mode(FM: Fast Mode)
typedef struct _FM_RawdataType_ {
	unsigned char dataType;		///< what kind of raw data type that want TouchLink return
	unsigned char dataLength;	///< TouchLink read the data length of dataType 
	unsigned char dataNum;		///< TouchLink read 'dataNum' times of dataType
} FM_RawdataType;

typedef struct _FM_ReadRawdata_ {
	unsigned char type;			///< bit0 ~ bit3: 0: I2C					1: SPI
								///< bit4 ~ bit7: 0: Check Mode(dist use)	1: Repeat Mode( raw data use)
	unsigned char addr;			///< Raw data address, typical 0x40

	/**
	 * To specify what kind of raw data want to read
	 * if you want read mutual, then you should set the type to mutual
	 * and specify the data length and times
	 */
	std::vector< FM_RawdataType >	readRawdataType;

	/**
	 * undefine raw data type default read length. if you only want mutual,
	 * then TouchLink will read other data(header, self, noise) use defaultReadLength
	 * to read data
	 */
	unsigned char defaultReadLength;
} FM_ReadRawdata;

std::string ConvertADB_CMD( const std::vector< unsigned char > & data )
{
	std::string cmd = "/data/httu/sitronix_client ";
	for ( const unsigned char v : data ) {
		char tmp_buf[4]{ 0 };
		snprintf( tmp_buf, 4, "%02X", v );
		cmd.append( tmp_buf );
	}
	return cmd;
}

int ADB_StartRecordRawData( HANDLE hHandle, FM_ReadRawdata * ctx )
{
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
	int res = HFST_ADB_SDK_Write( hHandle, cmd_buf.c_str(), cmd_buf.length() );

	std::cout << "[W5] " << cmd_buf.c_str() << std::endl;
	if ( HFST_ADB_ERROR_OK != res ) {
		std::cout << "[ERR] W5 " << res << std::endl;
		return res;
	}

	unsigned char read_buf[1024]{ 0 };
	res = HFST_ADB_SDK_Read( hHandle, read_buf, 1024 );
	std::cout << "Read " << ((res > 0) ? " PASS" : " NG") << std::endl;
	if ( res <= 0 ) return -1;
	std::cout << "Data: " << read_buf << std::endl << std::endl;

	return 1;
}

int ADB_StopRecordRawData( HANDLE hHandle, const char * pSavePath )
{
	std::vector< unsigned char > vecWriteBuf{ 0x56, 0x01, 0x00, 0x01 };

	std::string cmd_buf = ConvertADB_CMD( vecWriteBuf );
	int res = HFST_ADB_SDK_Write( hHandle, cmd_buf.c_str(), cmd_buf.length() );

	std::cout << ( "[W6] %s\n", cmd_buf.c_str() ) << std::endl;
	if ( HFST_ADB_ERROR_OK != res ) {
		std::cout << "[ERR] W6 " << res << std::endl;
		return res;
	}

	unsigned char read_buf[1024]{ 0 };
	res = HFST_ADB_SDK_Read( hHandle, read_buf, 1024 );
	std::cout << "Read " << ((res > 0) ? " PASS" : " NG") << std::endl;
	if ( res <= 0 ) return -1;
	std::cout << "Data: " << read_buf << std::endl << std::endl;

	std::string cmd( "pull /data/httu/RawData.txt " );
	cmd.append( pSavePath );

	res = HFST_ADB_SDK_RunCmd( "adb\\adb.exe", cmd.c_str() );

	std::string remove_file_buf( "rm /data/httu/RawData.txt" );
	HFST_ADB_SDK_Write( hHandle, remove_file_buf.c_str(), remove_file_buf.length() );

	return res;
}

// adb sdk main test function
int HFST_ADB_SDK_Test()
{
    HANDLE hHandle = nullptr;

	std::filesystem::path curr_path = std::filesystem::current_path();
	std::string adb_file( "adb\\adb.exe" );
    std::string services = "/data/httu/sitronix_server &";

    std::string read_channel_buf( "/data/httu/sitronix_client 700300F40200\r\n" );
	std::string read_raw_buf( "/data/httu/sitronix_client 700300403C00" );
	std::string switch_dev_page_buf( "/data/httu/sitronix_client 710200FFEF" );

	std::filesystem::path adb_path( adb_file );
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

	for ( int idx = 0; idx < _countof( init_cmd ); ++idx )
		HFST_ADB_SDK_RunCmd( adb_file.c_str(), init_cmd[idx] );

    unsigned char read_buf[1024]{ 0 };

	FM_ReadRawdata raw;
	raw.addr = 0x40;
	raw.type = 0;

	FM_RawdataType rt;

	// open adb service
    int res = HFST_ADB_SDK_Open( &hHandle, false, adb_file.c_str(), services.c_str() );
    std::cout << "HFST_ADB_SDK_Open " << ((HFST_ADB_ERROR_OK == res) ? "PASS" : "NG") << std::endl;
    if ( HFST_ADB_ERROR_OK != res ) goto __exit;

	// switch to develop page
	res = HFST_ADB_SDK_Write( hHandle, switch_dev_page_buf.c_str(), switch_dev_page_buf.length() );
	std::cout << "Write Switch develop page " << ((HFST_ADB_ERROR_OK == res) ? " PASS" : " NG") << std::endl;
	if ( HFST_ADB_ERROR_OK != res ) goto __exit;

	res = HFST_ADB_SDK_Read( hHandle, read_buf, 1024 );
	std::cout << "Read Switch develop page " << ((res > 0) ? " PASS" : " NG") << std::endl;
	if ( res <= 0 ) goto __exit;
	std::cout << "Switch develop page result: " << read_buf << std::endl << std::endl;

	// collect 1 frame raw data
	for ( int idx = 0; idx < 30; ++idx ) {
		res = HFST_ADB_SDK_Write( hHandle, read_raw_buf.c_str(), read_raw_buf.length() );
		std::cout << "Write read raw data " << ((HFST_ADB_ERROR_OK == res) ? " PASS" : " NG") << std::endl;
		if ( HFST_ADB_ERROR_OK != res ) goto __exit;

		res = HFST_ADB_SDK_Read( hHandle, read_buf, 1024 );
		std::cout << "Read raw data" << ((res > 0) ? " PASS" : " NG") << std::endl;
		if ( res <= 0 ) goto __exit;
		std::cout << "Read raw data result: " << read_buf << std::endl << std::endl;
	}

	rt.dataType = 0x06;
	rt.dataLength = 0x29;
	rt.dataNum = 0x1C;
	raw.readRawdataType.push_back( rt );

	rt.dataType = 0x04;
	rt.dataLength = 0x3B;
	rt.dataNum = 1;
	raw.readRawdataType.push_back( rt );

	res = ADB_StartRecordRawData( hHandle, &raw );

	Sleep( 5000 );

	res = ADB_StopRecordRawData( hHandle, "D:\\platform-tools\\");

__exit:
	HFST_ADB_SDK_Close( hHandle );

    system( "pause" );
    return 0;
}
