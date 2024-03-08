#include "pch.h"
#include "TTK_Communication_API.h"
#include "HFST_ADB_SDK_Wrapper.hpp"

#define Revision        (2)
#define Flash1k         (0x400)
#define Flash15k        (0x3C00)
#define Flash16k        (0x4000)
#define Flash30k        (0x7800)
#define Flash32k        (0x8000)
#define Flash48k        (0xC000)
#define Flash60k        (0xF000)
#define Flash63k        (0xFC00)
#define Flash61k        (0xF400)
#define Flash64k        (0x10000)
#define ReadTimeout     (20000)
#define RetryTimes      (3)
#define PageSize        (0x100)
#define PageSize512     (0x200)
#define PageSize1K      (0x400)
#define PageSize4K      (0x1000)
#define BlockSize       (0x200)
#define SWISPLength     (8)
#define PageSize_HID    (0xFC)//(0xFF)  

enum //SWISP
{    
    SWISP_Flash_Advance	= 0x12, // Advance for A8002

    SWISP_Response          = 0x31,
    SWISP_Erase             = 0x80,
    SWISP_Write_Data        = 0x81,
    SWISP_Write_Flash       = 0x82,
    SWISP_Read_Flash        = 0x83,
    SWISP_Reset             = 0x84,
    SWISP_Get_FWVer         = 0x88, //for ST1572
    SWISP_Run_APROM         = 0x89, //for ST1572
    SWISP_Get_Flash_Mode    = 0x89, //for ST1572
    SWISP_Read_Checksum     = 0x8B, //for ST1572
    SWISP_Boot_Sel          = 0x8C, //for ST1572
    SWISP_Read_DataFlash    = 0x8D, //for ST1572
    SWISP_UNLOCK    = 0x87, //for ST1572
    SWISP_ReadSFR    = 0x88, 
    SWISP_Ready    = 0x8F, 
};

enum{ //ST1801
    ST1801_SWISP_Unlock_Flash       =     0x10,
    ST1801_SWISP_lock_Flash       =     0x11,
    ST1801_SWISP_Chip_Erase       =     0x12,
    ST1801_SWISP_Sector_Erase       =     0x13,
    ST1801_SWISP_Page_Program       =     0x14,
    ST1801_SWISP_Read_Flash       =     0x15,
    ST1801_SWISP_32K_Erase       =     0x16,
    ST1801_SWISP_Unlock_Flash_ISP       =     0x90,
    ST1801_SWISP_Unlock_Boot_Loader       =     0x1F,
};

BOOL fUseNewINTProtocol;

int nSleepTime = 1;
int nSPIClock=6;
unsigned char g_sI2cAddr = 0;
constexpr unsigned int g_CRC24_PAGE_1K_FF = 0xCB0117;

TouchLinkVersion g_TouchLinkVersion;
#if IC_Module==IC_Module_A8008
const int UsbMaxReadLen=512;
#elif  IC_Module==IC_Module_A8010
const int UsbMaxReadLen=1024;
#elif  IC_Module==IC_Module_A8015
const int UsbMaxReadLen=512;
#elif  IC_Module==IC_Module_ST1801
const int UsbMaxReadLen=256;
#elif  IC_Module==IC_Module_ST1802
const int UsbMaxReadLen=256;
#elif  IC_Module == IC_Module_A8018
const int UsbMaxReadLen = 256;
#endif

#if IC_Module==IC_Module_A8010
BYTE DEFAULTDELAY1=0x05;
#elif IC_Module==IC_Module_ST1801
BYTE DEFAULTDELAY1=0x14;
#elif IC_Module==IC_Module_ST1802
BYTE DEFAULTDELAY1=0x14;
#else
BYTE DEFAULTDELAY1=0x05;
#endif
BYTE DEFAULTDELAY2=0x0D;
BYTE DELAY1=DEFAULTDELAY1;//0x05;   //0x14
BYTE DELAY2=DEFAULTDELAY2;//0x0D;
BOOL fSPI_ISPMode=false;

BOOL fProtocolC=false;
BOOL fSPIMode=false;
BYTE pFlashWritePageSize=0xFF;
//int pFlashErasePageSize = 4096;
BOOL fWriteflashNoRead = false;
static int nMultiDeviceID = -1;

// Forward declaration
int     ST1802ReadFlashHW( unsigned char * Data, unsigned int Addr, unsigned int Len );
bool    Sector_Erase_A8010_SPI_ISP( int Addr, int nLen );
int     ChecksumCalculation( unsigned short * pChecksum, unsigned char * pInData, unsigned long Len );
bool    CheckST1801_SPI_FLASH_Busy();
bool    CheckST1802_FlashEraseBusyWait();
bool    ST1801FlashWakeUp();
int     EraseFlashHW_ST1801( unsigned int Addr );
int     ST1801_SPI_FLASH_LOCK( BOOL fLock );
bool    ST1801ResetTouchLinkPullHight();
bool    ExWriteFlashPage_Bulk_ST1801( unsigned short Addr, unsigned char * Data );
bool    UnlockST1801BootLoader( BOOL fUse16Bit = false );
bool    Sector_Erase_ST1801_SW_ISP( int Addr, int nLen );
bool    Write_ST1801_SW_ISP( unsigned int Addr, unsigned char * Data );
//bool    WriteI2CData_2Bytes_ST1801( unsigned char * Addr, unsigned char * Data );

bool    EraseFlashBlock_SW( unsigned int Addr );
bool    WriteFlashBlock_SW( unsigned int Addr, unsigned char * Data );
bool    ReadFlashBlock_SW( unsigned int Addr, unsigned char * Data );

bool    EraseFlashPage_SW( unsigned int Addr );
int     WriteFlashPage_SW( unsigned int Addr, unsigned char * Data );
bool    ReadFlashPage_SW( unsigned int Addr, unsigned char * Data );
bool    ReadFlashPage_SW_Advance( unsigned int Addr, unsigned char * Data );

#if IC_Module == IC_Module_A8018
extern int  ReadFlashHW_A8018( unsigned char * pReadBuf, unsigned int nAddr, unsigned int nReadSize );
extern int  EraseFlashHW_A8018( unsigned int nAddr, unsigned int nSize, bool bNormalExitLockFlash );
extern int  WriteFlashHW_A8018( unsigned char * pWriteData, unsigned int nAddr, unsigned int nWriteSize );

extern int  ReadFlashSW_A8018( unsigned char * pReadBuf, unsigned int nAddr, unsigned int nReadSize );
extern int  WriteFlashSW_A8018( unsigned char * pWriteData, unsigned int nAddr, unsigned int nWriteSize );
extern int  EraseFlashSW_A8018_Mass();
#endif

// James 2020/07/24
typedef struct _I2C_CommandIO_Packet_ {
    unsigned char cmdID;
    unsigned char validDataSize;
    unsigned char cmdData[30];
} I2C_CommandIO_Packet;

static int CommandIO_Write( _I2C_CommandIO_Packet_ * pkt );
static int CommandIO_SetReady();
static int CommandIO_CheckReady();
static int CommandIO_Read( _I2C_CommandIO_Packet_ * pkt );


// Communication with HID sdk wrap
// James 2020/11/04
static int g_pro = 0;                       // Read/Write Flash progress
static int g_hidLastErrorCode = 0;
static char g_errorBuffer[MAX_PATH] = { 0 };
enum class CommunicationMode {
	CommunicationMode_TouchLink_Bulk = 0,	// default bulk mode
	CommunicationMode_Phone_ADB,		    // phone adb communication 
	CommunicationMode_HID_OverI2C			// only support ST1802 for touch pad
};

static CommunicationMode g_emCommunicationMode = CommunicationMode::CommunicationMode_TouchLink_Bulk;
typedef struct _TTK_HFST_HID_CONTEXT_ {
    bool bSupportVariableSize;
    int nHidReadMaxSize;
    int nHidWriteMaxSize;
    HANDLE hHidHandle;
    int nHidReportId;
} TTK_HFST_HID_CONTEXT;

static TTK_HFST_HID_CONTEXT g_hidContext{ false, 0, 0, nullptr, -1 };
bool CommunicationModeSelect( int cm )
{
    bool res = true;
    switch ( cm ) {
        case 0:
            g_emCommunicationMode = CommunicationMode::CommunicationMode_TouchLink_Bulk;
            break;
        case 1:
            g_emCommunicationMode = CommunicationMode::CommunicationMode_Phone_ADB;
            break;
        case 2:
            g_emCommunicationMode = CommunicationMode::CommunicationMode_HID_OverI2C;
            break;
        default:
            res = false;
            break;
    }
	return res;
}

bool HID_Init( int nProductId, int nVenorId, int nReportId, int nUsagePage )
{
    if ( g_hidContext.hHidHandle )
        return true;

	g_hidLastErrorCode = HFST_HID_SDK_Open( &g_hidContext.hHidHandle, nProductId, nVenorId, nUsagePage );
    if ( HFST_HID_ERROR_OK == g_hidLastErrorCode ) {
        HFST_HID_SDK_GetCommunicationMaxDataLength( g_hidContext.hHidHandle,
                                                    &g_hidContext.nHidReadMaxSize, 
                                                    &g_hidContext.nHidWriteMaxSize );
        g_hidContext.nHidReportId = nReportId;
        return true;
    }
	return false;
}

void HID_UnInit()
{
	HFST_HID_SDK_Close( g_hidContext.hHidHandle );
    g_hidContext.hHidHandle = nullptr;
}

int HID_GetLastErrorCode()
{
    return g_hidLastErrorCode&0xFFFFFF;
}

char * HID_GetLastErrorMessage()
{
    HFST_HID_SDK_GetErrorDetailMessage( g_hidLastErrorCode, g_errorBuffer, MAX_PATH );
    return g_errorBuffer;
}

int HID_ReadRawdata( unsigned char * rawdata, int nReadSize )
{
    // This is special command
    // direct send 0x0A(report id) to hid fw, fw direct return rawdata
	// hid feature data length is 0x08, so read data length max over 8
    // do this just for speed read raw data
    //if ( !rawdata || nReadSize > 0x56 )
    //if ( !rawdata || nReadSize > 512 )
    if ( !rawdata || nReadSize < 0x08 )
        return -1;
	if ( !g_hidContext.hHidHandle )
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	std::vector< unsigned char > vecWriteBuf;

    static constexpr int nRawdataReportId = 0x0A;
    //static constexpr int nRawdataMaxLength = 0x50 + 3;  // 0x50: real data length, 3: 2 byte fw return length + 1 byte report id
    int nRawdataMaxLength = nReadSize + 3;

	//unsigned char * tmp = new unsigned char[nRewdataMaxLength];
	memset( rawdata, 0, nRawdataMaxLength );
    rawdata[0] = nRawdataReportId;

	g_hidLastErrorCode = HFST_HID_SDK_Read( g_hidContext.hHidHandle, rawdata, nRawdataMaxLength );
	if ( HFST_HID_ERROR_OK != g_hidLastErrorCode ) return ERRORMSG_HID_READ - g_hidLastErrorCode;
    memcpy( &rawdata[0], &rawdata[1], nRawdataMaxLength );
    //rawdata = &rawdata[1];
	//memcpy( &rawdata[0], &tmp[1], nRewdataMaxLength );

	//delete[] tmp;
	return nRawdataMaxLength;
}

int ReadI2CReg_ProtocolA_HID( unsigned char *data, unsigned int addr, unsigned int len )
{
    if ( !g_hidContext.hHidHandle )
        return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	std::vector< unsigned char > vecWriteBuf;

    unsigned char * tmp = new unsigned char[PageSize_HID + 3];
//    unsigned char * tmp = new unsigned char[g_hidContext.nHidReadMaxSize + 1];
	int nReadSize = 0, nCmdSize = 0, nDataSize = 0;
	do {
        //if ( len > g_hidContext.nHidReadMaxSize - 3 ) nReadSize = g_hidContext.nHidReadMaxSize - 3;
        if ( len > PageSize_HID ) nReadSize = PageSize_HID;
        else nReadSize = len;

        vecWriteBuf.clear();
		vecWriteBuf.push_back( g_hidContext.nHidReportId );
		vecWriteBuf.push_back( 0x1F );          // Read command
		vecWriteBuf.push_back( (addr >> 8) );   // Addr High
		vecWriteBuf.push_back( addr & 0xFF );   // Addr Low
        vecWriteBuf.push_back( nReadSize );     // Read Size

        nCmdSize = g_hidContext.bSupportVariableSize ? vecWriteBuf.size() : g_hidContext.nHidWriteMaxSize;
        g_hidLastErrorCode = HFST_HID_SDK_Write( g_hidContext.hHidHandle, &vecWriteBuf[0], nCmdSize );
		if ( HFST_HID_ERROR_OK != g_hidLastErrorCode ) return ERRORMSG_HID_WRITE - g_hidLastErrorCode;

        memset( tmp, 0, g_hidContext.nHidReadMaxSize );
        tmp[0] = g_hidContext.nHidReportId;

        nCmdSize = g_hidContext.bSupportVariableSize ? nReadSize : g_hidContext.nHidReadMaxSize;
        if ( nReadSize > g_hidContext.nHidReadMaxSize ) nCmdSize = nReadSize + 3;   // report id 
        g_hidLastErrorCode = HFST_HID_SDK_Read( g_hidContext.hHidHandle, tmp, nCmdSize );
		if ( HFST_HID_ERROR_OK != g_hidLastErrorCode ) return ERRORMSG_HID_READ - g_hidLastErrorCode;
        memcpy( &data[nDataSize], &tmp[1], nReadSize );

        len -= nReadSize;
        nDataSize += nReadSize;
        addr += nReadSize;
	} while ( len > 0 );

    delete [] tmp;
	return nDataSize;
}

int WriteI2CReg_ProtocolA_HID( unsigned char *data, unsigned int addr, unsigned int len )
{
    if ( !g_hidContext.hHidHandle )
        return ERRORMSG_HID_WRITE_DEVICE_NOT_OPEN;

    std::vector< unsigned char > vecWriteBuf;
    
	int nWriteSize = 0, nDataSize = 0, nCmdSize = 0;
	do {
		if ( len > g_hidContext.nHidReadMaxSize - 5 ) nWriteSize = g_hidContext.nHidReadMaxSize - 5;
		else nWriteSize = len;

        vecWriteBuf.clear();
		vecWriteBuf.push_back( g_hidContext.nHidReportId );
		vecWriteBuf.push_back( 0x0F );          // Write command
		vecWriteBuf.push_back( (addr >> 8) );   // Addr High
		vecWriteBuf.push_back( addr & 0xFF );   // Addr Low
		vecWriteBuf.push_back( nWriteSize );    // Write Size

        // Copy write data
        for ( int i = 0; i < nWriteSize; ++i, ++nDataSize )
            vecWriteBuf.push_back( data[nDataSize] );

        nCmdSize = g_hidContext.bSupportVariableSize ? vecWriteBuf.size() : g_hidContext.nHidWriteMaxSize;
		g_hidLastErrorCode = HFST_HID_SDK_Write( g_hidContext.hHidHandle, &vecWriteBuf[0], nCmdSize );
		if ( HFST_HID_ERROR_OK != g_hidLastErrorCode ) return ERRORMSG_HID_WRITE - g_hidLastErrorCode;

		len -= nWriteSize;
        addr += nWriteSize;
	} while ( len > 0 );

	return nDataSize;
}
int WriteCmd_HID( unsigned char *data, unsigned short len )
{
    std::vector< unsigned char > vecWriteBuf;
    vecWriteBuf.push_back( g_hidContext.nHidReportId );
    vecWriteBuf.push_back( 0x0F );
    vecWriteBuf.push_back( Bridge_T_Write_CMD );
    vecWriteBuf.push_back( (len & 0xFF) );
    vecWriteBuf.push_back( (len >> 8) );

    int nWriteSize = 0, nDataSize = 0, nCmdSize = 0;

	if ( len > g_hidContext.nHidReadMaxSize - 5 ) nWriteSize = g_hidContext.nHidReadMaxSize - 5;
	else nWriteSize = len;

	do {
		// Copy write data
        for ( int i = 0; i < nWriteSize; ++i, ++nDataSize ) 
            vecWriteBuf.push_back( data[nDataSize] );

        nCmdSize = g_hidContext.bSupportVariableSize ? vecWriteBuf.size() : g_hidContext.nHidWriteMaxSize;
        g_hidLastErrorCode = HFST_HID_SDK_Write( g_hidContext.hHidHandle, &vecWriteBuf[0], nCmdSize );
		if ( HFST_HID_ERROR_OK != g_hidLastErrorCode ) return ERRORMSG_HID_WRITE - g_hidLastErrorCode;

		len -= nWriteSize;

        vecWriteBuf.clear();
        vecWriteBuf.push_back( g_hidContext.nHidReportId );
        vecWriteBuf.push_back( Bridge_T_Continuous );

		if ( len > g_hidContext.nHidReadMaxSize - 2 ) nWriteSize = g_hidContext.nHidReadMaxSize - 2;
		else nWriteSize = len;

	} while ( len > 0 );

    return nDataSize;
}

int ReadCmd_HID( unsigned char *data, unsigned short len )
{
    std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.push_back( g_hidContext.nHidReportId );
	vecWriteBuf.push_back( 0x1F );
	vecWriteBuf.push_back( Bridge_T_Read_CMD );
	vecWriteBuf.push_back( 0x02 );
	vecWriteBuf.push_back( 0x00 );
	vecWriteBuf.push_back( len & 0xFF );
	vecWriteBuf.push_back( len >> 8 );

	int nReadSize = 0, nDataSize = 0, nCmdSize = 0;
	do {
		int nWriteSize = g_hidContext.bSupportVariableSize ? vecWriteBuf.size() : g_hidContext.nHidWriteMaxSize;
        g_hidLastErrorCode = HFST_HID_SDK_Write( g_hidContext.hHidHandle, &vecWriteBuf[0], nWriteSize );
		if ( HFST_HID_ERROR_OK != g_hidLastErrorCode ) return ERRORMSG_HID_WRITE - g_hidLastErrorCode;

		if ( len > g_hidContext.nHidReadMaxSize - 3 ) nReadSize = g_hidContext.nHidReadMaxSize - 3;
		else nReadSize = len;

        nCmdSize = g_hidContext.bSupportVariableSize ? nReadSize + 3 : g_hidContext.nHidReadMaxSize;
        g_hidLastErrorCode = HFST_HID_SDK_Read( g_hidContext.hHidHandle, &data[nDataSize], nCmdSize );
		if ( HFST_HID_ERROR_OK != g_hidLastErrorCode ) return ERRORMSG_HID_READ - g_hidLastErrorCode;

		len -= nReadSize;
		nDataSize += nReadSize;
	} while ( len > 0 );

    return nDataSize;
}
/// Communication with HID sdk wrap end


//-----------------------------------------------------------------
void SleepInProgram(int nSleep)
{
    LARGE_INTEGER ticksPerSecond, start_tick, end_tick;
    QueryPerformanceFrequency(&ticksPerSecond); // CPU –禲碭 ticks 
    QueryPerformanceCounter(&start_tick); 
    LONGLONG diff=0;
    do 
    {
        QueryPerformanceCounter(&end_tick); // 挡璸计竚 
        diff = (LONGLONG)(end_tick.QuadPart - start_tick.QuadPart)*(LONGLONG)1000000 / ticksPerSecond.QuadPart;        
        diff/=1000;
        if((diff) > nSleep){
            break;
        }
    } while (1);
}
int   GetDllVersion(void)
{
    return Revision;
}

#define CRC8_POLY 0xD5
#define CRC16_POLY 0x1021
typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;
u32 RemainderCal(u32 poly, u32 dat, u8 polysize, u8 datasize)
{
    const u8 SHIFT_MAX = datasize-(polysize+1);
    u8 ind;
    u32 MSBOFDATA = (u32)1<<(datasize-1);
    u32 tmp;
    poly |= (1<<polysize);
    for(ind=0; ind<=SHIFT_MAX; ind++){
        if( (dat<<ind) & MSBOFDATA){ //if MSB == 1
            tmp = poly << (SHIFT_MAX-ind);
            dat ^= tmp; //poly dosen't include the MSB of the divider.
        }
    }
    return dat;// remainder is the lowest N bits
}
u16 Crc16Cal(u16 poly, u8* dat, u32 dat_len)
{
    const u8 POLYSIZE = 16, DATASIZE = 8;
    u32 crc,ind;
    crc = dat[0];
    for(ind = 1; ind<dat_len; ind++){
        crc = (crc<<DATASIZE) | dat[ind];
        crc = RemainderCal(poly, crc, POLYSIZE, (u8)(POLYSIZE+DATASIZE));// 16 bit CRC
    }
    crc = crc<<DATASIZE; // CRC16, the data should be shifted left 16bits. Shift 8 bit first
    crc = RemainderCal(poly, crc, POLYSIZE, (u8)(POLYSIZE+DATASIZE));
    crc = crc<<(POLYSIZE-DATASIZE); // Shift the rest 8bits
    crc = RemainderCal(poly, crc, POLYSIZE, (u8)(POLYSIZE+DATASIZE));
    return crc;
}
u16 Crc8Cal(u8 poly, u8* dat, u16 dat_len)
{
    const u8 POLYSIZE = 8, DATASIZE = 8;
    u32 crc,ind;
    crc = dat[0];
    for(ind = 1; ind<dat_len; ind++){
        crc = (crc<<DATASIZE) | dat[ind];
        crc = RemainderCal(poly, crc, POLYSIZE, (u8)(POLYSIZE+DATASIZE));// 16 bit CRC
    }

    crc = crc<<POLYSIZE;
    crc=RemainderCal(poly,crc,POLYSIZE,(u8)(POLYSIZE+DATASIZE));
    return crc;
}
void Get_CRC16_Calc(unsigned short *pChecksum,unsigned char *pInData,unsigned long Len)
{
    *pChecksum = Crc16Cal(CRC16_POLY,pInData,Len);
}
u8 Get_CRC8_Calc(unsigned char *pInData,unsigned long Len)
{
    return  Crc8Cal(CRC8_POLY,pInData,Len);
}
//-----------------------------------------------------------------

int WriteUsbAutoRetry(unsigned char *data, unsigned int ilen, unsigned int &rtlen)
{

    int ret;
    int retry = 10;
    while(retry--) {
        ret=WriteUsb(data,ilen,rtlen);
        if(ret>0)	retry=0;
        SleepInProgram(nSleepTime);
    }
    if(ret<0)
        return ERRORMSG_USB_RETRY_TIMEOUT_ERROR;
    if(rtlen!=ilen)  {
        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }    
    return ret;
}

//-----------------------------------------------------------------
bool ResetBridgeStatus_Bulk(void)
{
    int ret;
    unsigned char WBuffer[BulkLen]={0};
    WBuffer[0] = Bridge_T_ResetBridge;
    WBuffer[1] = 0; //Length L
    WBuffer[2] = 0; //Length H

    ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
    if(ret<0)
    {
        ////ShowMessage("USB_Write_Fail");
        return false;
    }
    if(ret!=BulkLen)
    {
        ////ShowMessage("Non-complete");
        return false;
    }
    return true;
}

//-----------------------------------------------------------------
int ResetTP(void)
{
    int ret;
    unsigned char WBuffer[BulkLen]={0};
    WBuffer[0] = Bridge_T_Reset_Touch;
    WBuffer[1] = 0; //Length L
    WBuffer[2] = 0; //Length H

    ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
    if(ret<0)
    {
        ////ShowMessage("USB_Write_Fail");
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    if(ret!=BulkLen)
    {
        ////ShowMessage("Non-complete");
        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }
    return true;
}

//-----------------------------------------------------------------
int ResetBridge(void)
{
    int ret;
    unsigned char WBuffer[BulkLen]={0};
    WBuffer[0] = Bridge_T_ResetBridge;
    WBuffer[1] = 0; //Length L
    WBuffer[2] = 0; //Length H

    ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
    if(ret<0)
    {
        ////ShowMessage("USB_Write_Fail");
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    if(ret!=BulkLen)
    {
        ////ShowMessage("Non-complete");
        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }
    return true;
}

//-----------------------------------------------------------------
//mode 0: Jump Bulk
//mode 1: Jump HID
//-----------------------------------------------------------------
int SwitchBridge(unsigned char mode)
{
    int ret;
    unsigned char WBuffer[BulkLen]={0};
    WBuffer[0] = Bridge_T_System_OP;
    WBuffer[1] = 2; //Length L
    WBuffer[2] = 0; //Length H
    WBuffer[3] = 0x0F;  //Bridge Switch
    WBuffer[4] = mode;
    ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
    if(ret<0)
    {
        ////ShowMessage("USB_Write_Fail");
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    if(ret!=BulkLen)
    {
        ////ShowMessage("Non-complete");
        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }
    return true;
}
/*
//-----------------------------------------------------------------
int SetI2CAddr(unsigned char Addr)
{
int ret;
unsigned char WBuffer[BulkLen]={0};
WBuffer[0] = Bridge_T_I2C_Addr;
WBuffer[1] = 1; //Length L
WBuffer[2] = 0; //Length H
WBuffer[3] = (Addr<<1);

ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
if(ret<0)
{
//ShowMessage("USB_Write_Fail");
return ERRORMSG_WRITE_BLUK_FAIL;
}
if(ret!=BulkLen)
{
////ShowMessage("Non-complete");
return ERRORMSG_NON_COMPLETE_TRANSFER;
}
return 1;
}
*/
//---------------------------------------------------------------------------
// |0x74|WLengthL|WLengthH|RLengthL|RLengthH|Data0|~|Data58|------------59bytes
// |0x7F|Data69 |~|Data121|---------------------------------------------63bytes
// |0x7F|Data122|~|Data184|--------------------------------------------63bytes
// ...
//---------------------------------------------------------------------------
int WriteReadCmd(unsigned char *WBuffer, unsigned short WLength, unsigned short RLength)
{
    unsigned char WRCMD[BulkLen]={0};
    //char StrShow[100];
    int WriteL=0,ret,i=0;

    memset(WRCMD,0,BulkLen);
    WRCMD[0]=Bridge_T_WriteRead_CMD;
    WRCMD[1]=(WLength+2) & 0xFF;  //RLength 2 Bytes
    WRCMD[2]=(WLength+2) >> 8;
    WRCMD[3]=(RLength+4) & 0xFF;  //Header 3 bytes checksum 1 byte
    WRCMD[4]=(RLength+4) >> 8;
    while(WriteL<WLength)
    {
        if(WriteL==0)
        {
            for(i=5 ; (i<BulkLen)&&(WriteL<WLength) ; i++,WriteL++)
            {
                WRCMD[i] = WBuffer[WriteL];
            }
        }
        else
        {
            WRCMD[0]=Bridge_T_Continuous;
            for(i=1 ; (i<BulkLen)&&(WriteL<WLength) ; i++,WriteL++)
            {
                WRCMD[i] = WBuffer[WriteL];
            }
        }

        ret = USBComm_WriteToBulkEx(WRCMD,BulkLen);
        if(ret<0)
        {
            ////ShowMessage("USB_Write_Fail");
            return ERRORMSG_WRITE_BLUK_FAIL;
        }
        if(ret!=BulkLen)
        {
            ////ShowMessage("Non-complete");
            return ERRORMSG_NON_COMPLETE_TRANSFER;
        }        
    }
    return 1;
}




//----------------------------------------------------------------
// |0x72|LengthL|LengthH|Data0|~|Data60|------------61bytes
// |0x7F|Data61 |~|Data123|-------------------------63bytes
// |0x7F|Data124|~|Data186|-------------------------63bytes
// ...
//----------------------------------------------------------------
int WriteCmd(unsigned char *WBuffer, unsigned short WLength)
{
    switch ( g_emCommunicationMode ) {
        case CommunicationMode::CommunicationMode_TouchLink_Bulk:
            break;
        case CommunicationMode::CommunicationMode_Phone_ADB:
            return WriteCmd_ADB( WBuffer, WLength );
        case CommunicationMode::CommunicationMode_HID_OverI2C:
            return WriteCmd_HID( WBuffer, WLength );
    }

    unsigned char WCMD[BulkLen]={0};
    int WriteL=0,ret,i;

    memset(WCMD,0,BulkLen);
    WCMD[0]= Bridge_T_Write_CMD;
    WCMD[1]=WLength & 0xFF;
    WCMD[2]=WLength >> 8;
    while(WriteL<WLength)
    {
        if(WriteL==0)
        {
            for(i=3 ; (i<BulkLen)&&(WriteL<WLength) ; i++,WriteL++)
            {
                WCMD[i] = WBuffer[WriteL];
            }
        }
        else
        {
            WCMD[0]=Bridge_T_Continuous;
            for(i=1 ; (i<BulkLen)&&(WriteL<WLength) ; i++,WriteL++)
            {
                WCMD[i] = WBuffer[WriteL];
            }
        }
        ret = USBComm_WriteToBulkEx(WCMD,BulkLen);
        if(ret<0)
            return ERRORMSG_WRITE_BLUK_FAIL;

        if(ret!=BulkLen)
            return ERRORMSG_NON_COMPLETE_TRANSFER;

    }
    return true;
}

int WriteCmd_Advancd(unsigned char *WBuffer, unsigned short WLength)
{
    unsigned char WCMD[BulkLen]={0};
    int WriteL=0,ret,i;

    memset(WCMD,0,BulkLen);
    for ( int i = 0; i < WLength; ++i )
        WCMD[i] = WBuffer[i];

    //while(WriteL<WLength)
    {
        ret = USBComm_WriteToBulkEx(WCMD,BulkLen);
        if(ret<0)
            return ERRORMSG_WRITE_BLUK_FAIL;

        if(ret!=BulkLen)
            return ERRORMSG_NON_COMPLETE_TRANSFER;

    }
    return true;
}
//----------------------------------------------------------------
int Read_Packet_Bulk(unsigned char *RBuffer, unsigned short RLength)
{
    unsigned char RData[UsbMaxReadLen]={0};
    int ReadL=0,ret,timeout=ReadTimeout,i=0;
    int done = false;
    int countFO=0;
    int usbReadNum;
    DWORD dwStartTick=GetTickCount();
    do
    {
        usbReadNum = UsbMaxReadLen;
        timeout=ReadTimeout;
        done = false;      
        int numReadUsbReadBytes = 0;
        int numReadUsbUsedBytes = 0;
        int numReadTime=0;
        if(ReadL==0){ // first one
            for(int k=0;k<UsbMaxReadLen/60;k++){
                numReadUsbReadBytes += 64;
                numReadTime++;
                if(k==0) {
                    numReadUsbUsedBytes+=60;
                }
                else {
                    numReadUsbUsedBytes+=63;
                }
                if(numReadUsbReadBytes >= UsbMaxReadLen){
                    break;
                }
                if(numReadUsbUsedBytes >= (RLength-ReadL)){
                    break;
                }
            }
        }
        else {
            for(int k=0;k<100000;k++){
                numReadUsbReadBytes += 64;
                numReadTime++;
                numReadUsbUsedBytes+=63;
                if(numReadUsbReadBytes >= UsbMaxReadLen){
                    break;
                }
                if(numReadUsbUsedBytes >= (RLength-ReadL)){
                    break;
                }
            }
        }
        usbReadNum = numReadUsbReadBytes;
        ret = USBComm_ReadFromBulkEx(RData,usbReadNum);
        if(ret<0)
        {
            return ERRORMSG_BRIDGE_STATUS_TIMEOUT;    ;
        }

        int bankNum = ((ret-1)/64)+1;
        int bankCnt=0;
        if(fSPIMode){
            //  if(RData[0]==Bridge_R_SPI_Data_Packet && RData[4]==0x87 && RData[5]==0x00){
            if((RData[0]==Bridge_R_SPI_Data_Packet || RData[0]==Bridge_R_HWISP_Data ) ||(RData[0]==0xff)){

            }else{
                return ERRORMSG_READ_PACKET_ERROR;
            }
        }
#if( (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802))
        if(RData[0]!=0x82){
            //return ERRORMSG_READ_PACKET_ERROR;
        }
#endif
        while ( (bankCnt<bankNum) && (ReadL<RLength) ){
            int nErrorTimeOut=1000;
            do
            {               
                switch(RData[0+bankCnt*64])
                {
                case Bridge_R_SPI_Data_Packet:
                case Bridge_R_Data:
                case Bridge_R_I2C_Data:
                case Bridge_R_HWISP_Data:
                    if(fSPIMode && RData[0]==Bridge_R_SPI_Data_Packet){
                        for(i=7 ; (i<64)&&(ReadL<RLength) ; i++,ReadL++)
                        {
                            RBuffer[ReadL] = RData[i+(bankCnt*64)];                        
                        }   
                    }else{
                        for(i=4 ; (i<64)&&(ReadL<RLength) ; i++,ReadL++)
                        {
                            RBuffer[ReadL] = RData[i+(bankCnt*64)];                        
                        }   
                    }
                    done = true;
                    bankCnt++;
                    break;
                case Bridge_R_Continuous:
                    for(i=1 ; (i<64)&&(ReadL<RLength) ; i++,ReadL++)
                    {
                        RBuffer[ReadL] = RData[i+(bankCnt*64)];

                    }
                    done = true;	
                    bankCnt++;
                    break;
                case Bridge_R_Status:				
                    countFO++;
                    memcpy(&RBuffer[0],&RData[0],RLength);
                    switch(RData[4])
                    {
                    case BridgeStatus_TouchPanel_Detect_Mode:
                        return ERRORMSG_BRIDGE_STATUS_SCANNING;
                        break;
                    case Bridge_Status_Running:
                        break;
                    case Bridge_Status_Initialization:
                        break;
                    case Bridge_Status_Ready:
                        switch(RData[5])
                        {
                        case I2C_Status_ACK:
                            done=true;
                            return ERRORMSG_TIMEOUT;
                            break;
                        case I2C_Status_Initialization:         
                            done=true;
                            return ERRORMSG_TIMEOUT;
                            break;
                        case I2C_Status_NAK:
                            return ERRORMSG_BRIDGE_STATUS_NAK;
                        case I2C_Status_Timeout:
                            return ERRORMSG_BRIDGE_STATUS_TIMEOUT;
                        case I2C_Status_Arbitration_Loss:
                            return ERRORMSG_BRIDGE_STATUS_ARB_Loss;
                        case I2C_Status_Bus_Fault:
                            return ERRORMSG_BRIDGE_STATUS_BUS_FAULT;
                        default:
                            return ERRORMSG_BRIDGE_STATUS_ERROR;
                            break; 
                        }

                        break;
                    case Bridge_Status_Error:
                        switch(RData[8])
                        {
                        case Error_Bulk_Length:
                            break;
                        case Error_First_Packet:
                            break;
                        case Error_Continuous_Packet:
                            break;
                        case Error_Unexpected:
                            break;
                        case Error_UnknownCmd:
                            break;
                        case Error_ReadTimeout:
                            break;
                        case Error_VddGndShort:
                            return ERRORMSG_BRIDGE_STATUS_ERROR_VDDGND_Short;
                            break;
                        }
                        ResetBridgeStatus_Bulk();
                        return ERRORMSG_BRIDGE_STATUS_ERROR;
                    default:
                        break; 
                    }                    
                    break;
                default:
                    return ERRORMSG_READ_PACKET_ERROR;
                }          
            }while(!done ); 
        }
        if(timeout <= 0)
        {
            return ERRORMSG_TIMEOUT;
        }
    }while( (ReadL<RLength) && (timeout--) &&((GetTickCount()-dwStartTick)<3000) );    

    return 1;
}
//----------------------------------------------------------------
int Read_Packet_INT_Bulk(unsigned char *RBuffer, unsigned short RLength)
{
    unsigned char RData[BulkLen]={0};
    char StrShow[100];
    int ReadL=0,ret,i=0;    

    ret = USBComm_ReadFromBulkEx(RData,BulkLen);   

    if(ret<0)    {
        return ERRORMSG_READ_BLUK_FAIL;
    }
    if(ret!=BulkLen)    {
        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }

    memset(RBuffer, 0, sizeof(RBuffer));
    switch(RData[0])    {
    case Bridge_R_INT_Detect_Data_Packet_Mode2:
        for(i=0 ; (i<64)&&(ReadL<RLength) ; i++,ReadL++)
        {
            RBuffer[ReadL] = RData[i];                        
        }
        return ret;
        break;
    case Bridge_R_INT_Detect_Data_Packet:
        for(i=4 ; (i<64)&&(ReadL<RLength) ; i++,ReadL++)
        {
            RBuffer[ReadL] = RData[i];                        
        }
        return 1;
        break;
    case Bridge_R_Data:
    case Bridge_R_I2C_Data:
    case Bridge_R_HWISP_Data:
        for(i=4 ; (i<64)&&(ReadL<RLength) ; i++,ReadL++)
        {
            RBuffer[ReadL] = RData[i];                        
        }
        break;
    case Bridge_R_Continuous:            
    case Bridge_R_Status:

        switch(RData[4])
        {
        case BridgeStatus_TouchPanel_Detect_Mode:
            {

            }
            break;
        case Bridge_Status_Running:
            break;
        case Bridge_Status_Initialization:
            break;
        case Bridge_Status_Ready:
            switch(RData[5])
            {
            case I2C_Status_ACK:
                break;
            case I2C_Status_Initialization:
                break;
            case I2C_Status_NAK:
                return ERRORMSG_BRIDGE_STATUS_NAK;
            case I2C_Status_Timeout:
                return ERRORMSG_BRIDGE_STATUS_TIMEOUT;
            case I2C_Status_Arbitration_Loss:
                return ERRORMSG_BRIDGE_STATUS_ARB_Loss;
            case I2C_Status_Bus_Fault:
                return ERRORMSG_BRIDGE_STATUS_BUS_FAULT;
            default:
                break; 
            }

            break;
        case Bridge_Status_Error:
            switch(RData[8])
            {
            case Error_Bulk_Length:
                break;
            case Error_First_Packet:
                break;
            case Error_Continuous_Packet:
                break;
            case Error_Unexpected:
                break;
            case Error_UnknownCmd:
                break;
            case Error_ReadTimeout:
                break;
            }
            ResetBridgeStatus_Bulk();
            return ERRORMSG_BRIDGE_STATUS_ERROR;
        default:
            break; 
        }
        return -1;  //No Data
    default:
        return ERRORMSG_READ_PACKET_ERROR;
    }
    return 1;
}
int Read_TouchPanelDetectStatus()
{
    unsigned char RData[BulkLen]={0};
    char StrShow[100];
    int ReadL=0,ret=0,i=0;    

    ret = USBComm_ReadFromBulkEx(RData,BulkLen);   
    if(ret<0)
    {
        return ERRORMSG_READ_BLUK_FAIL;
    }
    if(ret!=BulkLen)
    {
        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }

    if(RData[0]==Bridge_R_INT_Detect_Data_Packet_Mode2){
        if(RData[0x03]==BridgeStatus_TouchPanel_Detect_Mode){//0x06 = 
            if(RData[0x04]==0x01){
                ret  = (RData[5]<<8)+(RData[6]<<4)+RData[7];
            }else{
                ret = ERRORMSG_TP_Not_Connect;
            }
        }
    }else{
        ret = ERRORMSG_Not_Touch_Panel_Detect_Mode_Status;
    }
    return ret;
}



//----------------------------------------------------------------
// |0x82|LengthL|LengthH|Reserved|Data0|~|Data59|---60bytes
// |0xFF|Data60 |~|Data122|-------------------------63bytes
// |0xFF|Data123|~|Data185|-------------------------63bytes
// ...
//----------------------------------------------------------------
int ReadCmd(unsigned char *RBuffer, unsigned short RLength)
{
    switch ( g_emCommunicationMode ) {
        case CommunicationMode::CommunicationMode_TouchLink_Bulk:
            break;
        case CommunicationMode::CommunicationMode_Phone_ADB:
            return -1;
        case CommunicationMode::CommunicationMode_HID_OverI2C:
            return ReadCmd_HID( RBuffer, RLength );
    }

    unsigned char RCMD[BulkLen]={0};
    short Retry=RetryTimes;
    char StrShow[100];
    int ret; 

    LARGE_INTEGER Freq,diff;
    LARGE_INTEGER start,end;


    memset(RCMD,0,BulkLen);
    RCMD[0]=Bridge_T_Read_CMD;
    RCMD[1]=0x02;
    RCMD[2]=0x00;
    RCMD[3]=RLength & 0xFF;
    RCMD[4]=RLength >> 8;

    while(Retry--)
    {        
        ret = USBComm_WriteToBulkEx(RCMD, BulkLen);        
        if(ret<0)
            return ERRORMSG_WRITE_BLUK_FAIL;
        if(ret!=BulkLen)
            return ERRORMSG_NON_COMPLETE_TRANSFER;

        ret = Read_Packet_Bulk(RBuffer, RLength);       
        if( ((ret<0)&&(ret!=ERRORMSG_BRIDGE_STATUS_NAK))) //When status is NAK, it retries several times.
            return ret;
        else
            return true;
    }
    if(Retry==0)
        return ERRORMSG_RETRY_TIMEOUT;    

    return true;
}

int ReadCmd_Advance(unsigned char *RBuffer, unsigned short RLength)
{
    unsigned char RCMD[BulkLen]={0};
    short Retry=RetryTimes;
    char StrShow[100];
    int ret; 

    LARGE_INTEGER Freq,diff;
    LARGE_INTEGER start,end;
    memset(RCMD,0,BulkLen);
    RCMD[0]=0x82;
    RCMD[1]=0x00;
    RCMD[2]=0x04;
    ret = WriteCmd_Advancd(RCMD, BulkLen);   
    while(Retry--)
    {        
        if(ret<0)
            return ERRORMSG_WRITE_BLUK_FAIL;
        if(ret!=BulkLen)
            return ERRORMSG_NON_COMPLETE_TRANSFER;

        ret = Read_Packet_Bulk(RBuffer, RLength);       
        if( ((ret<0)&&(ret!=ERRORMSG_BRIDGE_STATUS_NAK))) //When status is NAK, it retries several times.
            return ret;
        else
            return true;
    }
    if(Retry==0)
        return ERRORMSG_RETRY_TIMEOUT;    

    return true;
}
//-----------------------------------------------------------------
void SetProtocolC(BOOL fEnableProtocolC)
{
    fProtocolC = fEnableProtocolC;
}
void SetProtocoSPI(BOOL fEnableSPI)
{
    fSPIMode = fEnableSPI;
}

int ReadI2CReg_ProtocolA_Bulk( unsigned char *data, unsigned int addr, unsigned int len )
{
	unsigned char WBuffer[BulkLen] = { 0 };
	unsigned char RBuff[UsbMaxReadLen * 4] = { 0 };
	short Retry = 1;//RetryTimes;
	time_t start, end;
	int ret;

	if ( len == 0 )
		return -1;
	if ( fSPIMode ) {
		WBuffer[0] = Bridge_SPI_Read_Command_Packet;
		WBuffer[1] = 0x08; //Length L
		WBuffer[2] = 0x00; //Length H
		WBuffer[3] = DELAY1; //DataBuf[0] = Delay1. 4us * 10(0x0A) = 40us
		WBuffer[4] = DELAY2; //DataBuf[1] = Delay2. 4us * 25(0x19)=100us
		WBuffer[5] = 0x87;   //DataBuf[2] = Signature number.
		WBuffer[6] = 0x00;  // DataBuf[3] = Transfer method.
		WBuffer[7] = len & 0xFF;  // Length L
		WBuffer[8] = len >> 8;  // Length H
		WBuffer[9] = (addr >> 8) | 0x80; //addr H
		WBuffer[10] = addr & 0xFF; //addr     
	} else {
		WBuffer[0] = Bridge_T_Read_Reg;
		WBuffer[1] = 3; //Length L
		WBuffer[2] = 0; //Length H
		WBuffer[3] = (unsigned char)addr;
		WBuffer[4] = (unsigned char)(len & 0xFF); //Register Length L
		WBuffer[5] = (unsigned char)(len >> 8);
	}

	while ( Retry-- )
	{
		ret = USBComm_WriteToBulkEx( WBuffer, BulkLen );
		if ( ret < 0 ) {
			return ERRORMSG_WRITE_BLUK_FAIL;
		}
		if ( ret != BulkLen ) {
			return ERRORMSG_NON_COMPLETE_TRANSFER;
		}
		ret = Read_Packet_Bulk( RBuff, (unsigned short)(len & 0xFFFF) );
		if ( ret == ERRORMSG_BRIDGE_STATUS_TIMEOUT ) {
			return ret;
		}
		if ( ret == ERRORMSG_BRIDGE_STATUS_ERROR ) {
			return ret;
		}
		if ( ret == true ) {
			memcpy( data, RBuff, len );
			return true;
		}
	}
	if ( Retry < 0 ) {
		return ret;
	}
	return ERRORMSG_READ_I2CREG_FAIL;
}

int ReadI2CReg_ProtocolA(unsigned char *data, unsigned int addr, unsigned int len)
{
	switch ( g_emCommunicationMode ) {
		case CommunicationMode::CommunicationMode_TouchLink_Bulk:
			return ReadI2CReg_ProtocolA_Bulk( data, addr, len );

		case CommunicationMode::CommunicationMode_Phone_ADB:
            return ReadI2CReg_ProtocolA_ADB( data, addr, len );

		case CommunicationMode::CommunicationMode_HID_OverI2C:
			return ReadI2CReg_ProtocolA_HID( data, addr, len );

		default:
			break;
	}
    return ERRORMSG_PROTOCOL_NOT_SUPPORT;
}
int ReadI2CReg_ProtocolC(unsigned char *data, unsigned int addr, unsigned int len)
{
    unsigned char WBuffer[255]={0};
    unsigned char RBuff[UsbMaxReadLen] ={0};
    unsigned char chk;

    if(len>128){
        WBuffer[0]=addr;
        WBuffer[1]=128;
        WBuffer[2] = Get_CRC8_Calc(WBuffer,2);
        WriteCmd(WBuffer,3);
        ReadCmd(RBuff,129);
        chk=Get_CRC8_Calc(RBuff,128);
        if(RBuff[128]!=chk){
            return ERRORMSG_READ_I2CREG_FAIL;
        }
        for(int i=0;i<128;i++){
            data[i]=RBuff[i];
        }
        WBuffer[0]=addr+128;
        WBuffer[1]=len-128;
        WBuffer[2] = Get_CRC8_Calc(WBuffer,2);
        WriteCmd(WBuffer,3);
        ReadCmd(RBuff,len-128+1);
        chk=Get_CRC8_Calc(RBuff,len-128);
        if(RBuff[len-128]!=chk){
            return ERRORMSG_READ_I2CREG_FAIL;
        }
        for(int i=128;i<len;i++){
            data[i]=RBuff[i-128];
        }
    }else{
        WBuffer[0]=addr;
        WBuffer[1]=len;
        WBuffer[2] = Get_CRC8_Calc(WBuffer,2);
        WriteCmd(WBuffer,3);
        ReadCmd(RBuff,len+1);
        chk=Get_CRC8_Calc(RBuff,len);
        if(RBuff[len]!=chk){    
            return ERRORMSG_READ_I2CREG_FAIL;
        }
        for(int i=0;i<len;i++){
            data[i]=RBuff[i];
        }
    }
    return TRUE;
}
int ReadI2CReg_2Bytes_ST1801(unsigned char *data, unsigned int addr, unsigned int len)
{
    if(fSPI_ISPMode){
        return ReadI2CReg(data,addr,len);
    }

    if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
        return ReadI2CReg_ProtocolA_HID( data, addr, len );

    if ( CommunicationMode::CommunicationMode_Phone_ADB == g_emCommunicationMode )
        return ReadI2CReg_ProtocolA_ADB( data, addr, len );

    unsigned char WBuffer[255]={0};
    unsigned char RBuff[PageSize1K] ={0};
    unsigned char chk;
    int ret=0;
    WBuffer[0] = (unsigned char)(addr>>8);
    WBuffer[1] = (unsigned char)addr;
    WriteCmd(WBuffer,2);
    if(ReadCmd(RBuff,len)>0){
        for(int i=0;i<len;i++){
            data[i]=RBuff[i];
        }
    }else {
        return false;
    }
    return TRUE;

}
int ReadI2CReg(unsigned char *data, unsigned int addr, unsigned int len)
{
    int nRet = TRUE;
#if  (IC_Module==IC_Module_A8010)
    if(fSPIMode){
        nRet = ReadI2CReg_ProtocolA(data,addr,len);
        if(nRet==true){
            SetProtocolC(false);
        }
        return nRet;
    }
    if(fProtocolC){
        nRet = ReadI2CReg_ProtocolC(data,addr,len);
        if(nRet<1){
            nRet = ReadI2CReg_ProtocolA(data,addr,len);
            if(nRet==true){
                SetProtocolC(false);
            }
        }
    }else{
        nRet = ReadI2CReg_ProtocolA(data,addr,len);
        if(nRet<1){
            nRet = ReadI2CReg_ProtocolC(data,addr,len);
            if(nRet==true){
                SetProtocolC(true);
            }
        }
    }
#elif( (IC_Module==IC_Module_ST1801)|| (IC_Module==IC_Module_ST1802))
    //nRet = ReadI2CReg_ST1801(data,addr,len);
    nRet = ReadI2CReg_ProtocolA(data,addr,len);
#else
    if(fSPIMode){
        nRet = ReadI2CReg_ProtocolA(data,addr,len);
        if(nRet==true){
            SetProtocolC(false);
        }
        return nRet;
    }else{
        nRet = ReadI2CReg_ProtocolA(data,addr,len);
    }
#endif
    return nRet;
}

//-----------------------------------------------------------------
bool WriteI2CReg_Bulk(unsigned char Addr,unsigned char Data)
{
    int ret;
    unsigned char WBuffer[BulkLen]={0};
    WBuffer[0] = Bridge_T_Write_Reg;
    WBuffer[1] = 2; //Length L
    WBuffer[2] = 0; //Length H
    WBuffer[3] = Addr;    
    WBuffer[4] = Data;

    ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
    if(ret<0)
    {        return false;
    }
    if(ret!=BulkLen)    {
        return false;
    }
    return true;
}

//-----------------------------------------------------------------
//|0x71|LengthL|LengthH|Addr|Data0|~|Data59|------------------60bytes
//|0x7F|Data60|~|Data123|------------------------------------63bytes
//-----------------------------------------------------------------
int WriteI2CReg_ProtocolA_Bulk(unsigned char *data, unsigned int addr, unsigned int len)
{  
    unsigned int WriteL=0,i=0,totallen=(len+1);
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    unsigned char WBuffer[UsbMaxReadLen*2]={0};
#elif IC_Module==IC_Module_A8010
    unsigned char WBuffer[UsbMaxReadLen*2]={0};
#elif (IC_Module==IC_Module_A8008) ||(IC_Module==IC_Module_A8015)
    unsigned char WBuffer[1024]={0};
#elif (IC_Module==IC_Module_A8018)
    unsigned char WBuffer[1024] = { 0 };
#endif

    int ret;

    if(len==0)
        return -1;

    if(data == NULL)
        return -3;
    if(fSPIMode){  
        totallen = (len + 5);
        WBuffer[0] = Bridge_SPI_Write_Command_Packet;            
        WBuffer[1] = (unsigned char)(totallen & 0xFF); //Length L
        WBuffer[2] = (unsigned char)(totallen >> 8);    //Length H
        WBuffer[3] = DELAY1;//DataBuf[0] = Delay1.            
        WBuffer[4] = DELAY2;//DataBuf[1] = Delay2.
        //         WBuffer[3] = 0X0A; //DataBuf[0] = Delay1. 4us * 10(0x0A) = 40us
        //         WBuffer[4] = 0X19; //DataBuf[1] = Delay2. 4us * 25(0x19)=100us
        WBuffer[5] = (addr>>8)&0x7F;//Addr H
        WBuffer[6] = addr&0xFF;//Addr L
        i=7;
        int pos=0;
        while(WriteL<len){
            if(WriteL==0){
                for( ; (i<BulkLen)&&(WriteL<len) ; i++,WriteL++)            {
                    WBuffer[pos+i] = data[WriteL];
                }
            }else{
                pos=0;
                WBuffer[pos]=Bridge_T_Continuous;
                for(i=1 ; (i<BulkLen)&&(WriteL<len) ; i++,WriteL++)            {
                    WBuffer[pos+i] = data[WriteL];
                }
            }
            pos+=BulkLen;

            ret = USBComm_WriteToBulkEx(WBuffer,pos);
            if(ret<0)
            {
                return ERRORMSG_WRITE_BLUK_FAIL;
            }
            if(ret!=pos)
            {
                return ERRORMSG_NON_COMPLETE_TRANSFER;
            }
        }
    }else{
        WBuffer[0] = Bridge_T_Write_Reg;
        WBuffer[1] = (unsigned char)(totallen & 0xFF); //Length L
        WBuffer[2] = (unsigned char)(totallen >> 8);    //Length H
        WBuffer[3] = (unsigned char)(addr&0xFF);
        i=4;
        int pos=0;
        while(WriteL<len){
            if(WriteL==0){
                for( ; (i<BulkLen)&&(WriteL<len) ; i++,WriteL++)            {
                    WBuffer[pos+i] = data[WriteL];
                }
            }else{
                WBuffer[pos]=Bridge_T_Continuous;
                for(i=1 ; (i<BulkLen)&&(WriteL<len) ; i++,WriteL++)            {
                    WBuffer[pos+i] = data[WriteL];
                }
            }
            pos+=BulkLen;
        }
        ret = USBComm_WriteToBulkEx(WBuffer,pos);
        if(ret<0)
        {
            return ERRORMSG_WRITE_BLUK_FAIL;
        }
        if(ret!=pos)
        {
            return ERRORMSG_NON_COMPLETE_TRANSFER;
        }
    }
    return true;
}

int WriteI2CReg_ProtocolA( unsigned char *data, unsigned int addr, unsigned int len )
{
	switch ( g_emCommunicationMode ) {
		case CommunicationMode::CommunicationMode_TouchLink_Bulk:
			return WriteI2CReg_ProtocolA_Bulk( data, addr, len );

		case CommunicationMode::CommunicationMode_Phone_ADB:
            return WriteI2CReg_ProtocolA_ADB( data, addr, len );

		case CommunicationMode::CommunicationMode_HID_OverI2C:
            return WriteI2CReg_ProtocolA_HID( data, addr, len );

        default:
			break;
	}
    return ERRORMSG_PROTOCOL_NOT_SUPPORT;
}

int WriteI2CReg_ProtocolC(unsigned char *data, unsigned int addr, unsigned int len)
{
    int i=0;
    unsigned char WBuffer[256]={0};
    unsigned char RBuff[UsbMaxReadLen] ={0};
    if(len>(125)){
        WBuffer[0]=addr;
        WBuffer[1]=125;        
        for(i=0;i<125;i++){
            WBuffer[i+2]=data[i];
        }
        WBuffer[i+2] = Get_CRC8_Calc(&WBuffer[0],125+2);
        WriteCmd(WBuffer,125+3);      
        WBuffer[0]=125;
        WBuffer[1]=len-125;        
        for(i=0;i<len-125;i++){
            WBuffer[i+2]=data[i];
        }
        WBuffer[i+2] = Get_CRC8_Calc(&WBuffer[0],len-125+2);
        WriteCmd(WBuffer,len-125+3);     
    }else{
        WBuffer[0]=addr;
        WBuffer[1]=len;        
        for(i=0;i<len;i++){
            WBuffer[i+2]=data[i];
        }
        WBuffer[i+2] = Get_CRC8_Calc(&WBuffer[0],len+2);
        WriteCmd(WBuffer,len+3);      
    }
    return TRUE;
}
int WriteI2CData_2Bytes_ST1801(unsigned char *data, unsigned int addr, unsigned int len)
{
    if ( CommunicationMode::CommunicationMode_HID_OverI2C == g_emCommunicationMode )
        return WriteI2CReg_ProtocolA_HID( data, addr, len );

    if ( CommunicationMode::CommunicationMode_Phone_ADB == g_emCommunicationMode )
        return WriteI2CReg_ProtocolA_ADB( data, addr, len );

    unsigned char WBuffer[PageSize1K*2]={0};
    unsigned char RBuff[PageSize1K*2] ={0};
    unsigned char chk;
    int ret=0;
    WBuffer[0] = (unsigned char)(addr>>8);
    WBuffer[1] = (unsigned char)addr;
    memcpy(&WBuffer[2],data,len);
    WriteCmd(WBuffer,len+2);   
    return TRUE;
}
int WriteI2CReg_2Bytes_ST1801(unsigned char *data, unsigned int addr, unsigned int len)
{
    if(fSPI_ISPMode){
        return WriteI2CReg(data,addr,len);
    }

	if ( CommunicationMode::CommunicationMode_HID_OverI2C == g_emCommunicationMode )
		return WriteI2CReg_ProtocolA_HID( data, addr, len );

    if ( CommunicationMode::CommunicationMode_Phone_ADB == g_emCommunicationMode )
        return WriteI2CReg_ProtocolA_ADB( data, addr, len );

    int i=0;
    unsigned char WBuffer[PageSize1K*2]={0};
    unsigned char RBuff[PageSize1K*2] ={0};
    WBuffer[0]=addr>>8;
    WBuffer[1]=addr;
    for(i=0;i<len;i++){
        WBuffer[i+2]=data[i];
    }
    int nRet = WriteCmd(WBuffer,len+3);      
    return nRet;      
}

int WriteI2CReg(unsigned char *data, unsigned int addr, unsigned int len)
{
    int nRet = true;
#if  (IC_Module==IC_Module_A8010)
    if(fSPIMode){
        nRet = WriteI2CReg_ProtocolA(data,addr,len);
        if(nRet==true){
            SetProtocolC(false);
        }
        return nRet;
    }
    if(fProtocolC){
        nRet = WriteI2CReg_ProtocolC(data,addr,len);
        if(nRet<1){
            nRet = WriteI2CReg_ProtocolA(data,addr,len);
            if(nRet==1){
                SetProtocolC(false);
            }
        }
    }else{
        nRet = WriteI2CReg_ProtocolA(data,addr,len);
        if(nRet<1){
            nRet = WriteI2CReg_ProtocolC(data,addr,len);
            if(nRet==1){
                SetProtocolC(true);
            }
        }
    }
#elif( (IC_Module==IC_Module_ST1801)|| (IC_Module==IC_Module_ST1802))
    //nRet = WriteI2CReg_ST1801(data,addr,len);
    nRet = WriteI2CReg_ProtocolA(data,addr,len);
#else
    if(fSPIMode){
        nRet = WriteI2CReg_ProtocolA(data,addr,len);
        if(nRet==true){
            SetProtocolC(false);
        }
        return nRet;
    }else{
        nRet = WriteI2CReg_ProtocolA(data,addr,len);
    }
#endif
    return nRet;
}


//---------------------------------------------------------------------
//Write
//|0x10|0x06|0x00|0x02|AddressH|AddressL|16K|0x00|0x01|
//Read
//|0x82|RLengthL|RLengthH|Reserved|Data0 |~|Data59|--------------60bytes
//|0xFF|Data60 |~|Data122|--------------------------------------63bytes
//|0xFF|Data123|~|Data185|--------------------------------------63bytes
//|0xFF|Data186|~|Data248|--------------------------------------63bytes
//|0xFF|Data249|~|Data255|---------------------------------------7bytes
//---------------------------------------------------------------------
bool ST1801ResetTouchLinkPullHight()
{
#if (IC_Module==IC_Module_ST1802) 
    BYTE EBuffer[BulkLen]={0},RData[BulkLen]={0};
    //Reset Pull Low
    EBuffer[0] = 0x14;	//Miscellaneous Operation
    EBuffer[1] = 01;		//Length L
    EBuffer[2] = 00;		//Length H
    EBuffer[3] = 0x04;	//GPIO Control    

    int ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
#elif (IC_Module==IC_Module_ST1801) 
    BYTE EBuffer[BulkLen]={0},RData[BulkLen]={0};
    int ret=0,SpiClock=0;
    EBuffer[0] = 0x50;	//Miscellaneous Operation
    EBuffer[1] = 4;		//Length L
    EBuffer[2] = 0;		//Length H

    EBuffer[3] = 0x02;	//GPIO Control
    EBuffer[4] = 0x02;	//Set GPIO
    EBuffer[5] = 0x02;	//Port C
    EBuffer[6] = 0x08;	//Bit 8

    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
#endif
    return true;
}
bool ST1801_6MHz(int nClk)
{
    char strOut[255]={0};
    BYTE EBuffer[BulkLen]={0},RData[BulkLen]={0};
    int ret=0,SpiClock=0;
    nSPIClock = nClk;
    if(fSPIMode)
    {
        //Read SPI Clock
        EBuffer[0] = 0x50;	//Miscellaneous Operation
        EBuffer[1] = 2;		//Length L
        EBuffer[2] = 0;		//Length H

        EBuffer[3] = 0x04;	//SPI Function
        EBuffer[4] = 0x01;	//Read SPI Clock

        ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        if(ret<0)    {

            return false;
        }
        ret = USBComm_ReadFromBulkEx(RData,64);
        SpiClock = RData[4]|(RData[5]<<8)|(RData[6]<<16)|(RData[7]<<24);
#ifdef _DEBUG

        sprintf(&strOut[0],"SPI Clock1: %d\r\n",SpiClock);
        OutputDebugString(strOut);
#endif // _DEBUG
        if(ret<0)    {
            return false;
        }
    }
    //Set SPI Clock
    if(fSPIMode) {
        EBuffer[0] = 0x50;	//Miscellaneous Operation
        EBuffer[1] = 6;		//Length L
        EBuffer[2] = 0;		//Length H
        nClk*=(1000*1000);
        EBuffer[3] = 0x04;	//SPI Function
        EBuffer[4] = 0x00;	//Set Clock
        EBuffer[5] = nClk&0xFF;	//SPI Clock = 10MHz
        EBuffer[6] =( nClk&0xFF00)>>8;
        EBuffer[7] = nClk>>16;
        EBuffer[8] = nClk>>24;
        ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        if(ret<0)    {
            return false;
        }
    }else{
        //Set SPI Clock
        //EBuffer[0] = 0x50;	//Miscellaneous Operation
        //EBuffer[1] = 5;		//Length L
        //EBuffer[2] = 0;		//Length H
        //nClk*=1000;
        //EBuffer[3] = 0x03;	//I2C Function
        //EBuffer[4] = 0x00;	//Set Clock
        //EBuffer[5] = nClk&0xFF;	//SPI Clock = 10MHz
        //EBuffer[6] =( nClk&0xFF00)>>8;
        //EBuffer[7] = nClk>>16;
        //EBuffer[8] = nClk>>24;
        //ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        //if(ret<0)    {
        //    return false;
        //}       
    }
    if(fSPIMode) {
        //Read SPI Clock
        EBuffer[0] = 0x50;	//Miscellaneous Operation
        EBuffer[1] = 2;		//Length L
        EBuffer[2] = 0;		//Length H

        EBuffer[3] = 0x04;	//SPI Function
        EBuffer[4] = 0x01;	//Read SPI Clock

        ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        if(ret<0)    {
            return false;
        }
        ret = USBComm_ReadFromBulkEx(RData,64);
        SpiClock = RData[4]|(RData[5]<<8)|(RData[6]<<16)|(RData[7]<<24);
        sprintf(&strOut[0],"SPI Clock2: %d\r\n",SpiClock);
        OutputDebugString(strOut);

        if(ret<0)    {
            return false;
        }
    }
    return true;
}
bool ST1801FlashWakeUp()
{
    int ret=0,SpiClock=0;
    BYTE EBuffer[BulkLen]={0},RData[BulkLen]={0};
#if (IC_Module==IC_Module_A8010) || (IC_Module==IC_Module_A8008)  ||(IC_Module==IC_Module_A8015)
    return true;
#endif
#if (IC_Module==IC_Module_ST1802) 

#else
    if(fSPIMode==false)  return true;
#endif

    //Read SPI Clock
    EBuffer[0] = 0x50;	//Miscellaneous Operation
    EBuffer[1] = 2;		//Length L
    EBuffer[2] = 0;		//Length H
    if(fSPIMode)
        EBuffer[3] = 0x04;	//SPI Function
    else
        EBuffer[3] = 0x03;	//SPI Function
    EBuffer[4] = 0x01;	//Read SPI Clock
    if(fSPIMode){
        ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        if(ret<0)    {
            return false;
        }
        ret = USBComm_ReadFromBulkEx(RData,64);
        SpiClock = RData[4]|(RData[5]<<8)|(RData[6]<<16)|(RData[7]<<24);

        if(ret<0)    {
            return false;
        }
    }
    //Set SPI Clock
    // if(fSPIMode)
    ST1801_6MHz(nSPIClock);

#if (IC_Module==IC_Module_ST1801)
    //Reset Pull Low
    EBuffer[0] = 0x50;	//Miscellaneous Operation
    EBuffer[1] = 4;		//Length L
    EBuffer[2] = 0;		//Length H
    EBuffer[3] = 0x02;	//GPIO Control
    EBuffer[4] = 0x03;	//Clear GPIO
    EBuffer[5] = 0x02;	//Port C
    EBuffer[6] = 0x08;	//Bit 8
#elif (IC_Module==IC_Module_ST1802)
    //Reset Pull Low
    memset(EBuffer,0x00,64);
    EBuffer[0] = 0x14;	//Miscellaneous Operation
    EBuffer[1] = 01;		//Length L
    EBuffer[2] = 00;		//Length H

    // 1. touch link must 6 or later
    // 2. touch link fw version must over 1.03
    constexpr unsigned char ST1802_NEW_ENTER_ICP_PROTOCOL_SW_VERSION = 103;
    int nTouchLinkSW_Version = g_TouchLinkVersion.sw_major * 100 + g_TouchLinkVersion.sw_sub;
    if(fSPIMode==false){
        // Enter I2C ICP Mode 
        if ( (g_TouchLinkVersion.hw < 6) || ((6 == g_TouchLinkVersion.hw) && (nTouchLinkSW_Version < ST1802_NEW_ENTER_ICP_PROTOCOL_SW_VERSION) ) ) {
            EBuffer[3] = 0x03;
        } else {
            EBuffer[1] = 02;    // length
            EBuffer[3] = 0x12;  // command
            EBuffer[4] = 0x64;  // delay time, recommend 100us
        }
    }else{
        // Enter SPI ICP Mode
        if ( (g_TouchLinkVersion.hw < 6) || ((6 == g_TouchLinkVersion.hw) && (nTouchLinkSW_Version < ST1802_NEW_ENTER_ICP_PROTOCOL_SW_VERSION)) ) {
            EBuffer[3] = 0x01;
        } else {
            EBuffer[1] = 02;    // length
            EBuffer[3] = 0x11;  // command
            EBuffer[4] = 0x64;  // delay time, recommend 100us
        }
    }  

#endif
    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
   
    //Release PD
    if(fSPIMode){
        EBuffer[0] = Bridge_SPI_AvdRead_Command_Packet;	//SPI Read
        EBuffer[1] = 9;		//Length L
        EBuffer[2] = 0;		//Length H
        EBuffer[3] = 0;		//Delay1
        EBuffer[4] = 0;		//Delay2
        EBuffer[5] = 0x40;	//Signature Number
        EBuffer[6] = 0;		//Transfer Method
        EBuffer[7] = 2;		//Read Length L
        EBuffer[8] = 0;		//Read Length H
        EBuffer[9] = 0;		//Reserved
        EBuffer[10] = 0;	//Reserved
        EBuffer[11] = 0xAB;	//Release PowerDown
    }else{
        EBuffer[0] = 0x14;	//SPI Read
        EBuffer[1] = 0x02;		//Length L
        EBuffer[2] = 0x00;		//Length H      

        EBuffer[3] = 0x05;		
        EBuffer[4] = 0xAB;		     
    }
    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }

    //Read Flash ID
    if(fSPIMode){
        EBuffer[0] = Bridge_SPI_AvdRead_Command_Packet;	//SPI Read
        EBuffer[1] = 12;	//Length L
        EBuffer[2] = 0;		//Length H

        EBuffer[3] = 0;		//Delay1
        EBuffer[4] = 0;		//Delay2
        EBuffer[5] = 0x55;	//Signature Number
        EBuffer[6] = 0;		//Transfer Method
        EBuffer[7] = 2;		//Read Length L
        EBuffer[8] = 0;		//Read Length H
        EBuffer[9] = 0;		//Reserved
        EBuffer[10] = 0;	//Reserved

        EBuffer[11] = 0x9F;	//Read Flash ID Support MX25U5121E
        EBuffer[12] = 0x00;	//Dummy
        EBuffer[13] = 0x00;	//Dummy
        EBuffer[14] = 0x00;	//00H
    }else{
        EBuffer[0] = 0x14;	//SPI Read
        EBuffer[1] = 0x04;	//Length L
        EBuffer[2] = 0x00;	//Length H

        EBuffer[3] = 0x06;		//I2C ICP Read Data Command.
        EBuffer[4] = 0x03;		// Read data length. (L)
        EBuffer[5] = 0x00;	    // Read data length. (H)
        EBuffer[6] = 0X9f;		//Write Data DeviceID.
        EBuffer[7] = 0x00;		//Write Data Dummy.
        EBuffer[8] = 0x00;		//Write Data Dummy.
        EBuffer[9] = 0x00;		//Write Data 0x00.
    }
    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    ret = USBComm_ReadFromBulkEx(RData,64);
#ifdef _DEBUG
    char strOut[255]={0};
    sprintf(&strOut[0],"Flash ID: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n",RData[0],RData[1],RData[2],RData[3],RData[4],RData[5],RData[6],RData[7],RData[8],RData[9]);
    OutputDebugString(strOut);
#endif // _DEBUG
    if(ret<0)    {
        return false;
    }
    
    BYTE pFlashData[5]={0};
    if(RData[0x00]==0x81){ //I2C
        for(int i=0;i<3;i++){
            pFlashData[i] = RData[0x04+i];
        }
    }else if(RData[0x00]==0x88){ //SPI
        for(int i=0;i<3;i++){
            pFlashData[i] = RData[0x05+i];
        }
    }
    pFlashWritePageSize=0xFF;
    if((pFlashData[0] == 0xC2) && (pFlashData[1] == 0x25) && (pFlashData[2] == 0x11)){
        pFlashWritePageSize = 0x20;
    }
    if((pFlashData[0] == 0xC2) && (pFlashData[1] == 0x25) && (pFlashData[2] == 0x31)){
        pFlashWritePageSize = 0x20;
    }
   
#if (IC_Module==IC_Module_ST1802)
    Sleep(10);
#endif
    if(CheckST1801_SPI_FLASH_Busy()==false) return false;
    return true;
}
bool CheckST1802_FlashEraseBusyWait()
{
    BYTE EBuffer[BulkLen]={0},RData[BulkLen]={0},FlashStatus=0x00,TimeOut=50;
    int ret =0;    
    do {
        //Read Flash Status
        if(fSPIMode){
            EBuffer[0] = Bridge_SPI_AvdRead_Command_Packet;	//SPI Read
            EBuffer[1] = 9;		//Length L
            EBuffer[2] = 0;		//Length H
            EBuffer[3] = 0;		//Delay1
            EBuffer[4] = 0;		//Delay2
            EBuffer[5] = 0x80;	//Signature Number
            EBuffer[6] = 0;		//Transfer Method
            EBuffer[7] = 1;		//Read Length L
            EBuffer[8] = 0;		//Read Length H
            EBuffer[9] = 0;		//Reserved
            EBuffer[10] = 0;	//Reserved
            EBuffer[11] = 0x05;	//Read Status
        }else{
            EBuffer[0] = 0x14;	//I2C ICP Cmd
            EBuffer[1] = 0x04;	//CMD Len L
            EBuffer[2] = 0x00;	//CMD Len H

            EBuffer[3] = 0x06;		//I2C ICP Read Status Command.
            EBuffer[4] = 0x01;		//Read data length. (L)
            EBuffer[5] = 0x00;	    //Read data length. (H)
            EBuffer[6] = 0x05;		// Write Command.
            EBuffer[7] = 0x00;		// Write Command.

        }
        ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        if(ret<0)    {
            return false;
        }
        ret = USBComm_ReadFromBulkEx(RData,64);
        if(ret<0)    {
            return false;
        }
        if(fSPIMode)
            FlashStatus = RData[5];
        else
            FlashStatus = RData[4];
#ifdef _DEBUG
        CString strTmp;
        strTmp.Format("Flash Status = 0x%02X\r\n",FlashStatus);
        OutputDebugString(strTmp);
#endif // _DEBUG
        if(FlashStatus==0x00)   break;
        if(TimeOut==0){
            return false;
        }else{
            TimeOut--;
            SleepInProgram(1);
        }
    }while(FlashStatus!=0x00);
    return true;
}
bool CheckST1801_SPI_FLASH_Busy()
{
    BYTE EBuffer[BulkLen]={0},RData[BulkLen]={0},FlashStatus=0x00,TimeOut=500;
    int ret =0;    
    do {
        //Read Flash Status
        if(fSPIMode){
            EBuffer[0] = Bridge_SPI_AvdRead_Command_Packet;	//SPI Read
            EBuffer[1] = 9;		//Length L
            EBuffer[2] = 0;		//Length H
            EBuffer[3] = 0;		//Delay1
            EBuffer[4] = 0;		//Delay2
            EBuffer[5] = 0x80;	//Signature Number
            EBuffer[6] = 0;		//Transfer Method
            EBuffer[7] = 1;		//Read Length L
            EBuffer[8] = 0;		//Read Length H
            EBuffer[9] = 0;		//Reserved
            EBuffer[10] = 0;	//Reserved
            EBuffer[11] = 0x05;	//Read Status
        }else{
            EBuffer[0] = 0x14;	//I2C ICP Cmd
            EBuffer[1] = 0x04;	//CMD Len L
            EBuffer[2] = 0x00;	//CMD Len H

            EBuffer[3] = 0x06;		//I2C ICP Read Status Command.
            EBuffer[4] = 0x01;		//Read data length. (L)
            EBuffer[5] = 0x00;	    //Read data length. (H)
            EBuffer[6] = 0x05;		// Write Command.
            EBuffer[7] = 0x00;		// Write Command.

        }
        ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        if(ret<0)    {
            return false;
        }
        ret = USBComm_ReadFromBulkEx(RData,64);
        if(ret<0)    {
            return false;
        }
        if(fSPIMode)
            FlashStatus = RData[5];
        else
            FlashStatus = RData[4];
#ifdef _DEBUG
        CString strTmp;
        strTmp.Format("Flash Status = 0x%02X\r\n",FlashStatus);
        OutputDebugString(strTmp);
#endif // _DEBUG
        if(TimeOut==0){
            return false;
        }else{
            TimeOut--;
//             SleepInProgram(nSleepTime);
//             if(nSleepTime<5){
//                 nSleepTime++;
//             }
        }
    }while(FlashStatus&0x01);
    return true;
}
bool ExReadFlashPage_Bulk_I2C_ST1802(int Addr,unsigned char *Data)
{
    int ret=0;
    BYTE EBuffer[BulkLen]={0},RBuffer[BulkLen]={0},OutPutData[PageSize1K *2]={0};
    //Read Flash Data
    EBuffer[0] = 0x14;	//SPI Read
   
    EBuffer[1] = 0x07;	//Length L
    EBuffer[2] = 0x00;		//Length H
    EBuffer[3] = 0x07;
    EBuffer[4] = 0x00;
    EBuffer[5] = 0x04;
    EBuffer[6] = 0x03;	//Read Data
    EBuffer[7] = (unsigned char)(Addr>>16);	//A23-A16
    EBuffer[8] = (unsigned char)(Addr>>8);	//A15-A8
    EBuffer[9] = (unsigned char)(Addr);		//A7-A0
    
    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    int nIndex=0;
    do{
        ret = USBComm_ReadFromBulkEx(RBuffer,64);
        if(ret<0){
            return false;
        }
        if(RBuffer[0]==0x81){
            for(int j=4;j<64;j++){
                OutPutData[nIndex++]=RBuffer[j];
            }
        }else{
            if(RBuffer[0]==0xFF){
                for(int j=1;j<64;j++){
                    OutPutData[nIndex++]=RBuffer[j];
                }
            }else{
                return false;
            }
        }
    } while (nIndex<1024);
    memcpy(Data,OutPutData, PageSize1K );
    return true;
}
bool ExReadFlashPage_Bulk_SPI_ST1802(int Addr,unsigned char *Data)
{
    int ret=0;
    BYTE EBuffer[BulkLen]={0},RBuffer[BulkLen]={0},OutPutData[PageSize1K *2]={0};
    //Read Flash Data
    /*
    EBuffer[0] = Bridge_SPI_AvdRead_Command_Packet;	//SPI Read
    EBuffer[1] = 12;	//Length L
    EBuffer[2] = 0;		//Length H

    EBuffer[3] = 0;		//Delay1
    EBuffer[4] = 0;		//Delay2
    EBuffer[5] = 0x80;	//Signature Number
    EBuffer[6] = 0;		//Transfer Method
    EBuffer[7] = 0;		//Read Length L
    EBuffer[8] = 4;		//Read Length H
    EBuffer[9] = 0;		//Reserved
    EBuffer[10] = 0;	//Reserved

    EBuffer[11] = 0x03;	//Read Data
    EBuffer[12] = (unsigned char)(Addr>>16);	//A23-A16
    EBuffer[13] = (unsigned char)(Addr>>8);	//A15-A8
    EBuffer[14] = (unsigned char)(Addr);		//A7-A0
    */

    EBuffer[0] = Bridge_Read_SPI_AvdRead_Command_Packet;	//SPI Read
    EBuffer[1] = 12;	//Length L
    EBuffer[2] = 0;		//Length H

    EBuffer[3] = 0;		//Delay1 Td 0
    EBuffer[4] = 0;		//Delay2 Tr 1
    EBuffer[5] = 0x80;	//Signature 2
    EBuffer[6] = 0x00;		//Transfer Method 3
    EBuffer[7] = 0;		//Len L 4 
    EBuffer[8] = 4;		//Len H 5
    EBuffer[9] = 0;		//Tr 6
    EBuffer[10] = 0;	// 7

    EBuffer[11] = 0x03;	//Read Data 8
    EBuffer[12] = (unsigned char)(Addr>>16);	//A23-A16
    EBuffer[13] = (unsigned char)(Addr>>8);	//A15-A8
    EBuffer[14] = (unsigned char)(Addr);		//A7-A0

    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    int nIndex=0;
    for(int i=0;i<17;i++){
        ret = USBComm_ReadFromBulkEx(RBuffer,64);
        if(ret<0){
            return false;
        }
        if(i==0){
            for(int j=8;j<64;j++){
                OutPutData[nIndex++]=RBuffer[j];
            }
        }else{
            for(int j=1;j<64;j++){
                OutPutData[nIndex++]=RBuffer[j];
            }
        }        
    }

    memcpy(Data,OutPutData,PageSize1K);
    return true;
}
bool ExReadFlashPage_Bulk_SPI_ST1801(unsigned short Addr,unsigned char *Data)
{
    int ret=0;
    BYTE EBuffer[BulkLen]={0},RBuffer[BulkLen]={0},OutPutData[PageSize1K*2]={0};
    //Read Flash Data
    EBuffer[0] = Bridge_Read_SPI_AvdRead_Command_Packet;//Bridge_SPI_AvdRead_Command_Packet;	//SPI Read
    EBuffer[1] = 12;	//Length L
    EBuffer[2] = 0;		//Length H

    EBuffer[3] = 0;		//Delay1
    EBuffer[4] = 0;		//Delay2
    EBuffer[5] = 0x80;	//Signature Number
    EBuffer[6] = 0;		//Transfer Method
    EBuffer[7] = 0;		//Read Length L
    EBuffer[8] = 4;		//Read Length H
    EBuffer[9] = 0;		//Reserved
    EBuffer[10] = 0;	//Reserved

    EBuffer[11] = 0x03;	//Read Data
    EBuffer[12] = (unsigned char)(Addr>>16);	//A23-A16
    EBuffer[13] = (unsigned char)(Addr>>8);	//A15-A8
    EBuffer[14] = (unsigned char)(Addr);		//A7-A0

    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    int nIndex=0;
    for(int i=0;i<17;i++){
        ret = USBComm_ReadFromBulkEx(RBuffer,64);
        if(ret<0){
            return false;
        }
        if(i==0){
            for(int j=8;j<64;j++){
                OutPutData[nIndex++]=RBuffer[j];
            }
        }else{
            for(int j=1;j<64;j++){
                OutPutData[nIndex++]=RBuffer[j];
            }
        }        
    }
    memcpy(Data,OutPutData,PageSize1K);
    return true;
}
bool ExReadFlashPage_Bulk(unsigned short Addr,unsigned char *Data)
{

    int ret;
    char StrShow[100];
    unsigned char RBuffer[BulkLen]={0};
    short Retry=RetryTimes;
    unsigned short curPageSize=0; 
#if IC_Module==IC_Module_A8008
    curPageSize = PageSize512;
#elif IC_Module==IC_Module_A8010
    curPageSize = PageSize1K;
#elif IC_Module==IC_Module_A8015
    curPageSize = PageSize512;
#elif (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    curPageSize = PageSize1K;
    return ExReadFlashPage_Bulk_SPI_ST1801(Addr,Data);
#endif
    if( (Addr&0xFF)!=0 )
    {
        //MsgAdd("Wrong Start Address");
        return false;
    }

    RBuffer[0] = Bridge_T_HWISP;
    RBuffer[1] = 6; //Length L
    RBuffer[2] = 0; //Length H 
    RBuffer[3] = HWISP_Read;
#if IC_Module==IC_Module_A8008
    RBuffer[4] = Addr >> 7;
    RBuffer[5] = Addr & 0x7F;
    if(Addr >= Flash16k)
        RBuffer[6] = 0x80;		
    else
        RBuffer[6] = 0;
#elif IC_Module==IC_Module_A8010
    RBuffer[4] = Addr >> 8;
    RBuffer[5] = Addr & 0x8F;
    if(Addr >= Flash63k)
        RBuffer[6] = 0x80;		
    else
        RBuffer[6] = 0;
#elif IC_Module==IC_Module_A8015
    RBuffer[4] = Addr >> 7;
    RBuffer[5] = Addr & 0x7F;
    if(Addr >= Flash32k) {
        RBuffer[4] = (Addr-Flash32k) >> 7;
        RBuffer[5] = (Addr-Flash32k) & 0x7F;
        RBuffer[6] = 0x80;		        
    }else
        RBuffer[6] = 0;
#endif
    RBuffer[7] = (curPageSize & 0xFF);; // 256byte
    RBuffer[8] = (curPageSize>>8);
    while(Retry--)
    {
        ret = USBComm_WriteToBulkEx(RBuffer,BulkLen);
        if(ret<0)
        {
            //ShowMessage("USB_Write_Fail");
            return false;
        }
        if(ret!=BulkLen)
        {
            //ShowMessage("Non-complete");
            return false;
        }    

        ret = Read_Packet_Bulk(Data,curPageSize);
        if(((ret<0)&&(ret!=ERRORMSG_BRIDGE_STATUS_NAK))) //When status is NAK, it retries several times.
        {
            return false;
        }
        else
            return true;
    }    
    if(Retry==0)
    {
        return false;    
    }

    return true;
}

//-------------------------------------------------------------------------------
//|0x10|0x68|0x00|0x01|AddressL|AddressH|16K|0x00|0x00|Data0|~|Data56|-----57bytes
//|0x7F|Data57 |~|Data63|--------------------------------------------------7bytes
//Address + 64
//|0x10|0x68|0x00|0x01|AddressL|AddressH|16K|0x00|0x00|Data64|~|Data120|---57bytes
//|0x7F|Data121|~|Data127|-------------------------------------------------7bytes
//Address + 64
//|0x10|0x68|0x00|0x01|AddressL|AddressH|16K|0x00|0x00|Data128|~|Data184|---57bytes
//|0x7F|Data185|~|Data191|-------------------------------------------------7bytes
//Address + 64
//|0x10|0x68|0x00|0x01|AddressL|AddressH|16K|0x00|0x00|Data192|~|Data248|---57bytes
//|0x7F|Data249|~|Data255|-------------------------------------------------7bytes
//-------------------------------------------------------------------------------
bool ExWriteFlashPage_Bulk_ST1801(unsigned short Addr,unsigned char *Data)
{
    BYTE EBuffer[BulkLen]={0};
    int ret=0,times=0,i=0,Windex=0;;
    //Set Write Enable
    EBuffer[0] = 0x75;	//SPI Write
    EBuffer[1] = 3;		//Length L
    EBuffer[2] = 0;		//Length H

    EBuffer[3] = 0;		//Delay1
    EBuffer[4] = 0;		//Delay2

    EBuffer[5] = 0x06;	//Write Enable

    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    if(pFlashWritePageSize==0x20){
        //Page Program Command
        EBuffer[0] = 0x7A;	//SPI Write
        EBuffer[1] = 0x28;		//Length L 6
        EBuffer[2] = 0x00;//1;		//Length H 1
        EBuffer[3] = 0;		//Delay1 Td
        EBuffer[4] = 0;		//Delay2 Tw
        EBuffer[5] = 0;		//Delay2 Tp
        EBuffer[7] = 0x02;	//Page Program
        EBuffer[8] = (unsigned char)(Addr>>16);	//A23-A16
        EBuffer[9] = (unsigned char)(Addr>>8);	//A15-A8
        EBuffer[0x0a] = (unsigned char)(Addr);		//A7-A0
        for(i=0x0b ; i<32+0x0b ; i++,Windex++){
            EBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        if(ret<0)    {
            return false;
        }
    }else{
        //Page Program Command
        EBuffer[0] = 0x7A;	//SPI Write
        EBuffer[1] = 0x08;		//Length L 6
        EBuffer[2] = 0x01;//1;		//Length H 1
        EBuffer[3] = 0;		//Delay1 Td
        EBuffer[4] = 0;		//Delay2 Tw
        EBuffer[5] = 0;		//Delay2 Tp
        EBuffer[7] = 0x02;	//Page Program
        EBuffer[8] = (unsigned char)(Addr>>16);	//A23-A16
        EBuffer[9] = (unsigned char)(Addr>>8);	//A15-A8
        EBuffer[0x0a] = (unsigned char)(Addr);		//A7-A0
        for(i=0x0b ; i<BulkLen ; i++,Windex++){
            EBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        if(ret<0)    {
            return false;
        }
        memset(EBuffer,0x00,BulkLen);
        EBuffer[0] = Bridge_T_Continuous;
        do {
            for(i=1 ; i<BulkLen ; i++,Windex++){
                EBuffer[i] = Data[Windex];
            }
            ret = USBComm_WriteToBulkEx(EBuffer,BulkLen);
            if(ret<0){
                return false;
            }
            if(ret!=BulkLen){
                return false;
            }
        } while (Windex<256);
    }

    if(CheckST1801_SPI_FLASH_Busy()==false) return false;
    return true;
}
bool ExWriteFlashPage_Bulk_ST1802(int Addr,unsigned char *Data)
{
    BYTE EBuffer[BulkLen*6]={0};
    int ret=0,times=0,i=0,Windex=0;;
    //Set Write Enable
    if(fSPIMode){
        EBuffer[0] = 0x75;	//SPI Write
        EBuffer[1] = 3;		//Length L
        EBuffer[2] = 0;		//Length H
        EBuffer[3] = 0x00;		//Delay1
        EBuffer[4] = 0x00;		//Delay2
        EBuffer[5] = 0x06;	//Write Enable
    }else{
        EBuffer[0] = 0x14;
        EBuffer[1] = 0x02;	
        EBuffer[2] = 0x00;	
        EBuffer[3] = 0x05;		
        EBuffer[4] = 0x06;	//Write Enable
    }
   
    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
       
    if(fSPIMode){
         memset(EBuffer,0x00,BulkLen);
        if(pFlashWritePageSize==0x20){
            EBuffer[0] = 0x7A;	//SPI Write
            EBuffer[1] = 0x28;		//Length L 6
            EBuffer[2] = 0x00;//1;		//Length H 1
            EBuffer[3] = 0;		//Delay1 Td
            EBuffer[4] = 0;		//Delay2 Tw
            EBuffer[5] = 0;		//Delay2 Tp
            EBuffer[7] = 0x02;	//Page Program
            EBuffer[8] = (unsigned char)(Addr>>16);	//A23-A16
            EBuffer[9] = (unsigned char)(Addr>>8);	//A15-A8
            EBuffer[0x0a] = (unsigned char)(Addr);		//A7-A0
            for(i=0x0b ; i<32+0x0b ; i++,Windex++){
                EBuffer[i] = Data[Windex];
            }
            ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
            if(ret<0)    {
                return false;
            }
            if(CheckST1801_SPI_FLASH_Busy()==false){
                return false;
            }
        }else{
            EBuffer[0] = 0x7A;	//SPI Write
            EBuffer[1] = 0x08;		//Length L 6
            EBuffer[2] = 0x01;//1;		//Length H 1
            EBuffer[3] = 0;		//Delay1 Td
            EBuffer[4] = 0;		//Delay2 Tw
            EBuffer[5] = 0;		//Delay2 Tp
            EBuffer[7] = 0x02;	//Page Program
            EBuffer[8] = (unsigned char)(Addr>>16);	//A23-A16
            EBuffer[9] = (unsigned char)(Addr>>8);	//A15-A8
            EBuffer[0x0a] = (unsigned char)(Addr);		//A7-A0
            for(i=0x0b ; i<BulkLen ; i++,Windex++){
                EBuffer[i] = Data[Windex];
            }
            ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);           
            EBuffer[0] = Bridge_T_Continuous;
            do {
                for(i=1 ; i<BulkLen ; i++,Windex++){
                    EBuffer[i] = Data[Windex];
                }
                ret = USBComm_WriteToBulkEx(EBuffer,BulkLen);
                if(ret<0){
                    return false;
                }
                if(ret!=BulkLen){
                    return false;
                }
            } while (Windex<256);
        }
    }
    if(pFlashWritePageSize==0x20){
        //Page Program Command
        if(fSPIMode){
            //SPI
            EBuffer[0] = 0x7A;	//SPI Write
            EBuffer[1] = 0x28;		//Length L 6
            EBuffer[2] = 0x00;//1;		//Length H 1
            EBuffer[3] = 0;		//Delay1 Td
            EBuffer[4] = 0;		//Delay2 Tw
            EBuffer[5] = 0;		//Delay2 Tp
            EBuffer[7] = 0x02;	//Page Program
            EBuffer[8] = (unsigned char)(Addr>>16);	//A23-A16
            EBuffer[9] = (unsigned char)(Addr>>8);	//A15-A8
            EBuffer[0x0a] = (unsigned char)(Addr);		//A7-A0
            for(i=0x0b ; i<32+0x0b ; i++,Windex++){
                EBuffer[i] = Data[Windex];
            }
            ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
            if(ret<0)    {
                return false;
            }
            if(CheckST1801_SPI_FLASH_Busy()==false){
                return false;
            }
        }else{   
            //I2C
            EBuffer[0] = 0x14;	//I2C Write
            EBuffer[1] = 0x25;	//Length L 
            EBuffer[2] = 0x00;	//Length H 

            EBuffer[3] = 0x05;	
            EBuffer[4] = 0x02;
            EBuffer[5] = (unsigned char)(Addr>>16);	//A23-A16
            EBuffer[6] = (unsigned char)(Addr>>8);	//A15-A8
            EBuffer[7] = (unsigned char)(Addr);		//A7-A0
            for(i=8 ; i<32+8 ; i++,Windex++){
                EBuffer[i] = Data[Windex];
            }            
            ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);                
            if(ret<0)    {
                return false;
            }
            SleepInProgram(nSleepTime);
            if(CheckST1801_SPI_FLASH_Busy()==false){
                return false;
            }
        }
    }else{
        //Page Program Command
        if(fSPIMode){
            EBuffer[0] = 0x7A;	//SPI Write
            EBuffer[1] = 0x08;		//Length L 6
            EBuffer[2] = 0x01;//1;		//Length H 1
            EBuffer[3] = 0;		//Delay1 Td
            EBuffer[4] = 0;		//Delay2 Tw
            EBuffer[5] = 0;		//Delay2 Tp
            EBuffer[7] = 0x02;	//Page Program
            EBuffer[8] = (unsigned char)(Addr>>16);	//A23-A16
            EBuffer[9] = (unsigned char)(Addr>>8);	//A15-A8
            EBuffer[0x0a] = (unsigned char)(Addr);		//A7-A0
            for(i=0x0b ; i<BulkLen ; i++,Windex++){
                EBuffer[i] = Data[Windex];
            }
            ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        }else{
            EBuffer[0] = 0x14;	//SPI Write
            EBuffer[1] = 0x05;		//Length L
            EBuffer[2] = 0x01;		//Length H

            EBuffer[3] = 0x05;		//Delay1
            EBuffer[4] = 0x02;		//Delay2
            EBuffer[5] = (unsigned char)(Addr>>16);	//A23-A16
            EBuffer[6] = (unsigned char)(Addr>>8);	//A15-A8
            EBuffer[7] = (unsigned char)(Addr);		//A7-A0
            for(i=8 ; i<BulkLen ; i++,Windex++){
                EBuffer[i] = Data[Windex];
            }
            ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        }
        if(ret<0){
            return false;
        }
        memset(EBuffer,0x00,BulkLen);
        EBuffer[0] = Bridge_T_Continuous;
        do {
            for(i=1 ; i<BulkLen ; i++,Windex++){
                EBuffer[i] = Data[Windex];
            }
            ret = USBComm_WriteToBulkEx(EBuffer,BulkLen);
            if(ret<0){
                return false;
            }
            if(ret!=BulkLen){
                return false;
            }
        } while (Windex<256);
        if(CheckST1801_SPI_FLASH_Busy()==false){
            return false;
        }
    }
    // if(CheckST1801_SPI_FLASH_Busy()==false) return false;
    return true;
}
bool ExWriteFlashPage_Bulk_A8010(unsigned short Addr,unsigned char *Data)
{
    unsigned char RPage[PageSize1K]={0};
    unsigned char WBuffer[BulkLen]={0};
    BOOL fFirstRun=true;
Retry:
    unsigned char ChecksumR=0,ChecksumW=0;
    char StrShow[100];
    unsigned short tempAddr=Addr;
    unsigned short	curPageSize=1024;
    unsigned short	curTimes = 0;
    int ret,times=0,i=0,Windex=0;
    curTimes =8;
    if( (Addr&0xFF)!=0 )
    {
        return false;
    }

    //while(times<curTimes){     
    for(int j=0;j<8;j++){
        //=========================
        WBuffer[0] = Bridge_T_HWISP;
        WBuffer[1] = 0x86; //Length L 
        WBuffer[2] = 0x00; //Length H        
        WBuffer[3] = HWISP_WriteBySection;      
#if IC_Module== IC_Module_A8010
        WBuffer[4] = ((Addr+(j<<7))>>8);
        WBuffer[5] = (Addr+(j<<7));
#endif
        WBuffer[6] = 0;
        if((Addr) >= Flash63k){
            WBuffer[6] = 0x80;
        }
#if IC_Module== IC_Module_A8010
        WBuffer[7] = 4;
        WBuffer[8] = 0x20;
#endif
        for(i=9 ; i<BulkLen ; i++,Windex++){
            WBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        if(ret<0){
            return false;
        }
        if(ret!=BulkLen){
            return false;
        }	
        memset(WBuffer,0x00,BulkLen);
        WBuffer[0] = Bridge_T_Continuous;
        for(i=1 ; i<BulkLen ; i++,Windex++){
            WBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        memset(WBuffer,0x00,BulkLen);
        WBuffer[0] = Bridge_T_Continuous;
        for(i=1 ; i<=10 ; i++,Windex++){
            WBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        if(ret<0)
        {
            return false;
        }
        if(ret!=BulkLen)
        {
            return false;
        }
        tempAddr+=(BulkLen*2);
        times++;
    }
    fFirstRun=false;
    //=================================*/
    return true;
}
bool ExWriteFlashPage_Bulk_WriteBySection(unsigned short Addr,unsigned char *Data)
{   //A8008 SPI ICP Write
    unsigned char RPage[PageSize1K]={0};
    unsigned char WBuffer[BulkLen]={0};
    BOOL fFirstRun=true;
Retry:
    unsigned char ChecksumR=0,ChecksumW=0;
    char StrShow[100];
    unsigned short tempAddr=Addr;
    unsigned short	curPageSize=0;
    unsigned short	curTimes = 0;
    int ret,times=0,i=0,Windex=0;
    curPageSize = 1024;
    curTimes =8;
    if( (Addr&0xFF)!=0 )
    {
        //MsgAdd("Wrong Start Address");
        return false;
    }
    while(times<curTimes)
    {        
        //=========================
        WBuffer[0] = Bridge_T_HWISP;
        WBuffer[1] = 0x86; //Length L 
        WBuffer[2] = 0x00; //Length H        
        WBuffer[3] = HWISP_WriteBySection;      
        WBuffer[4] = (tempAddr) >> 7;
        WBuffer[5] = (tempAddr) & 0x7F;
        WBuffer[6] = 0;
        WBuffer[7] = 0x02;
        WBuffer[8] = 64;
#if IC_Module==IC_Module_A8008
        if(tempAddr >= Flash16k){
            WBuffer[4] = (tempAddr-Flash16k) >> 7;
            WBuffer[5] = (tempAddr-Flash16k) & 0x7F;
            WBuffer[6] = 0x80;
        }else
#elif IC_Module==IC_Module_A8015
        if(tempAddr >= Flash32k){
            WBuffer[4] = (tempAddr-Flash32k) >> 7;
            WBuffer[5] = (tempAddr-Flash32k) & 0x7F;
            WBuffer[6] = 0x80;
        }else
#endif
            WBuffer[6] = 0;
        for(i=9 ; i<BulkLen ; i++,Windex++){
            WBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        if(ret<0){
            return false;
        }
        if(ret!=BulkLen){
            return false;
        }
        memset(WBuffer,0x00,BulkLen);
        WBuffer[0] = Bridge_T_Continuous;
        for(i=1 ; i<BulkLen ; i++,Windex++){
            WBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        memset(WBuffer,0x00,BulkLen);
        WBuffer[0] = Bridge_T_Continuous;
        for(i=1 ; i<=10 ; i++,Windex++){
            WBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        if(ret<0)
        {
            return false;
        }
        if(ret!=BulkLen)
        {
            return false;
        }
        tempAddr+=(BulkLen*2);
        times++;
    }
    fFirstRun=false;
    //=================================*/
    return true;
}
bool ExWriteFlashPage_Bulk(unsigned short Addr,unsigned char *Data)
{
#if IC_Module== IC_Module_A8010
    return ExWriteFlashPage_Bulk_A8010(Addr,Data);
#endif // _I
#if (IC_Module== IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    return ExWriteFlashPage_Bulk_ST1801(Addr,Data);
#endif // _I
#if (IC_Module== IC_Module_A8008) || (IC_Module== IC_Module_A8015)
    if(fSPIMode)
        return ExWriteFlashPage_Bulk_WriteBySection(Addr,Data);
#endif // _I
    unsigned char RPage[PageSize1K]={0};
    unsigned char WBuffer[BulkLen]={0};
    BOOL fFirstRun=true;
Retry:
    unsigned char ChecksumR=0,ChecksumW=0;
    char StrShow[100];
    unsigned short tempAddr=Addr;
    unsigned short	curPageSize=0;
    unsigned short	curTimes = 0;
    int ret,times=0,i=0,Windex=0;
    curPageSize = 1024;
    curTimes =8;
    if( (Addr&0xFF)!=0 )
    {
        //MsgAdd("Wrong Start Address");
        return false;
    }

    while(times<curTimes)
    {        
        //=========================
        WBuffer[0] = Bridge_T_HWISP;
        WBuffer[1] = 0x86; //Length L 
        WBuffer[2] = 0x00; //Length H        
        WBuffer[3] = HWISP_Write;      
        WBuffer[4] = (tempAddr) >> 7;
        WBuffer[5] = (tempAddr) & 0x7F;
#if IC_Module==IC_Module_A8008
        if(tempAddr >= Flash16k){
            WBuffer[4] = (tempAddr-Flash16k) >> 7;
            WBuffer[5] = (tempAddr-Flash16k) & 0x7F;
            WBuffer[6] = 0x80;
        }else{
            WBuffer[6] = 0;
        }
#elif IC_Module==IC_Module_A8015
        if(tempAddr >= Flash32k){
            WBuffer[4] = (tempAddr-Flash32k) >> 7;
            WBuffer[5] = (tempAddr-Flash32k) & 0x7F;
            WBuffer[6] = 0x80;
        }else{
            WBuffer[6] = 0;
        }
#endif

        for(i=9 ; i<BulkLen ; i++,Windex++){
            WBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        if(ret<0){
            return false;
        }
        if(ret!=BulkLen){
            return false;
        }
        memset(WBuffer,0x00,BulkLen);
        WBuffer[0] = Bridge_T_Continuous;
        for(i=1 ; i<BulkLen ; i++,Windex++){
            WBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        memset(WBuffer,0x00,BulkLen);
        WBuffer[0] = Bridge_T_Continuous;
        for(i=1 ; i<=10 ; i++,Windex++){
            WBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        if(ret<0)
        {
            return false;
        }
        if(ret!=BulkLen)
        {
            return false;
        }
        tempAddr+=(BulkLen*2);
        times++;
    }
    fFirstRun=false;
    //=================================*/
    return true;

}
bool ExWriteFlashPage_NoVerify_Bulk(unsigned short Addr,unsigned char *Data)
{
    int ret,times=0,i=0,Windex=0;

    unsigned char ChecksumR=0,ChecksumW=0;
#if  IC_Module==IC_Module_A8008 ||  IC_Module==IC_Module_A8010 || IC_Module == IC_Module_A8015	||IC_Module==IC_Module_ST1801 || (IC_Module==IC_Module_ST1802) || (IC_Module_A8018)
    unsigned char RPage[PageSize1K]={0};
    unsigned char WBuffer[BulkLen]={0};
#endif

    char StrShow[100];
    unsigned short tempAddr=Addr;
    unsigned short	curPageSize=0;
    unsigned short	curTimes = 0;

#if IC_Module==IC_Module_A8008 || IC_Module==IC_Module_A8010 || IC_Module==IC_Module_A8015	||IC_Module==IC_Module_ST1801 || (IC_Module==IC_Module_ST1802)
    curPageSize = 1024;
    curTimes =8;
#endif

    if( (Addr&0xFF)!=0 )
    {
        //MsgAdd("Wrong Start Address");
        return false;
    }

    while(times<curTimes)
    {        
        if(tempAddr>=(0x4000)){
            int a=0;
        }
        //=========================
        WBuffer[0] = Bridge_T_HWISP;
#if IC_Module==IC_Module_A8008 || IC_Module==IC_Module_A8010 || IC_Module==IC_Module_A8015	||IC_Module==IC_Module_ST1801 || (IC_Module==IC_Module_ST1802)
        WBuffer[1] = 0x86; //Length L 
#endif
        WBuffer[2] = 0x00; //Length H        
        WBuffer[3] = HWISP_Write;    
        WBuffer[4] = (tempAddr) >> 7;
        WBuffer[5] = (tempAddr) & 0x7F;
#if IC_Module==IC_Module_A8008
        if(tempAddr >= Flash16k){
            WBuffer[4] = (tempAddr-Flash16k) >> 7;
            WBuffer[5] = (tempAddr-Flash16k) & 0x7F;
            WBuffer[6] = 0x80;	
        }else{
            WBuffer[6] = 0x00;		
        }
#elif IC_Module==IC_Module_A8015
        if(tempAddr >= Flash32k){
            WBuffer[4] = (tempAddr-Flash32k) >> 7;
            WBuffer[5] = (tempAddr-Flash32k) & 0x7F;
            WBuffer[6] = 0x80;		
        }else{
            WBuffer[6] = 0x00;		
        }

#elif  IC_Module==IC_Module_A8010	
        WBuffer[4] = (tempAddr) >> 7;
        WBuffer[5] = (tempAddr) & 0x7F;
        if((tempAddr) >= Flash16k){
            WBuffer[4] = (tempAddr-Flash16k) >> 8;
            WBuffer[5] = (tempAddr-Flash16k) & 0xFF;
            WBuffer[6] = 0x80;
        }else{
            WBuffer[6] = 0;
        }
#endif
        for(i=9 ; i<BulkLen ; i++,Windex++){
            WBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        if(ret<0){
            return false;
        }
        if(ret!=BulkLen){
            return false;
        }
        memset(WBuffer,0x00,BulkLen);
        WBuffer[0] = Bridge_T_Continuous;
        for(i=1 ; i<BulkLen ; i++,Windex++){
            WBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        memset(WBuffer,0x00,BulkLen);
        WBuffer[0] = Bridge_T_Continuous;
        for(i=1 ; i<=10 ; i++,Windex++){
            WBuffer[i] = Data[Windex];
        }
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        if(ret<0)
        {
            return false;
        }
        if(ret!=BulkLen)
        {
            return false;
        }
        tempAddr+=(BulkLen*2);
        times++;
    }
    return true;
}

//----------------------------------------------------------
bool Jump_SWISP(void)
{
    int ret,i=0,Repeat=2;
    unsigned char state[9];
    ret = ReadI2CReg(state, 0x01, 0x08);
    if(ret<0)
    {
        return false;
    }

    if(state[0]==0x06)
    {
        return true;
    }

    unsigned char pattern[9] = { "STX_FWUP" };

    //====check status====end*/
    Repeat=2;
    while(Repeat--)
    {    
        for(i=0;i<8;i++)
        {
            //ret = WriteI2CReg_Bulk(0x00,JumpSWISPString[i]);
            ret = WriteI2CReg( &pattern[i], 0x00, 1 );
            if(ret==false)
            {
                return false;
            }
        }

        SleepInProgram(300);
        //====check status====
        ret = ReadI2CReg(state, 0x01, 0x08);
        if(ret<0)
        {
            return false;
        }
        if(state[0]==0x06)
        {
            return true;
        }
        //====check status====end
        SleepInProgram(nSleepTime);
        //====check status====
        ret = ReadI2CReg(state, 0x01, 0x08);
        if(ret<0)
        {
            return false;
        }
        if(state[0]==0x06)
        {
            return true;
        }
    }
    return false;
}


//------------------------------------------------------------------
bool GetResponse_SW(unsigned short *Checksum)
{
    int ret;
    char StrShow[100];
    unsigned char GBuffer[BulkLen]={0};

    ret = ReadCmd(GBuffer, SWISPLength);
    if(ret<0)
    {
        return false;
    }
    if(GBuffer[0]!=0x8F)
    {
        return false;
    }else{
        if(GBuffer[2]&0x10)
        {
            return false;
        }        
        else{
            if(GBuffer[2]&0x02)
            {
                return false;
            }else{
                if(GBuffer[2]&0x01)
                {
                    //MsgAdd("Flash Error");
                    return false;
                }            
                else
                {
                    //MsgAdd("Flash Correct");
                }
            }
        }
    }
    *Checksum = GBuffer[4] + (GBuffer[5]<<8);
    return true;
}
//------------------------------------------------------------------
//|0x72|0x09|0x00|0x80|0x00|BlockNum|0x00|0x00|0x00|0x00|0x00|
//------------------------------------------------------------------
bool   EraseFlashBlock_SW(unsigned int Addr)
{
    int ret;
    char StrShow[100];
    unsigned char EBuffer[BulkLen]={0};
    unsigned short CK;
    if( (Addr&0x1FF)!=0 )
    {
        return false;
    }

    EBuffer[0] = SWISP_Erase;
    EBuffer[1] = 0;
    EBuffer[2] = (Addr >> 9) & 0xFF;
    EBuffer[3] = 0;
    EBuffer[4] = 0;
    EBuffer[5] = 0;
    EBuffer[6] = 0;    
    EBuffer[7] = 0;
    ret = WriteCmd(EBuffer, SWISPLength);
    if(ret<0)
    {
        return false;
    }
    SleepInProgram(60);
    ret = GetResponse_SW(&CK);
    if(ret==false)
    {
        return false;
    }

    return true; 

}

//ST1536------------------------------------------------------------
//|0x72|0x09|0x00|0x80|0x00|PageNum|0x00|0x00|0x00|0x00|0x00|
//------------------------------------------------------------------
extern int A8018_CheckFlashStatusCRCSW( unsigned char nFlashStatus, unsigned int nCheckCRC, unsigned char nDelayMS );
extern UINT32 RemainderCal( UINT32 poly, UINT32 dat, UINT8 polysize, UINT8 datasize );
extern UINT32 Crc24Cal( UINT32 poly, UINT8 * dat, UINT32 dat_len );
bool EraseFlashPage_SW(unsigned int Addr)
{
    int ret;
    char StrShow[100];
    unsigned char EBuffer[BulkLen]={0};
    unsigned short CK;
    if( (Addr&0xFF)!=0 )
        return false;

    unsigned char nWriteSize = SWISPLength;
#if IC_Module==IC_Module_A8008 ||  IC_Module==IC_Module_A8010	||  IC_Module==IC_Module_A8015
    EBuffer[0] = SWISP_UNLOCK;
    EBuffer[1] = 0;
    EBuffer[2] = 0;
    EBuffer[3] = 0;
    EBuffer[4] = 0;
    EBuffer[5] = 0;
    EBuffer[6] = 0;    
    EBuffer[7] = 0;
    ret = WriteCmd(EBuffer, SWISPLength);
#endif

#if IC_Module == IC_Module_A8018
    EBuffer[0] = 0x72;  // CMD Erase
    EBuffer[1] = Addr >= Flash60k ? 0xA5 : 0x5A;  // Erase Pass Code
    nWriteSize = 3;
#else
    EBuffer[0] = SWISP_Erase;
    EBuffer[1] = 0;
#endif

#if IC_Module==IC_Module_A8008 ||  IC_Module==IC_Module_A8010 || IC_Module==IC_Module_A8015 || IC_Module == IC_Module_A8018
    EBuffer[2] = (Addr >> 10) & 0xFF;
#endif
    EBuffer[3] = 0;
    EBuffer[4] = 0;
    EBuffer[5] = 0;
    EBuffer[6] = 0;    
    EBuffer[7] = 0;
#if ( IC_Module==IC_Module_A8008 ) || (IC_Module==IC_Module_A8015)

#elif IC_Module==IC_Module_A8010	
    if(Addr>=0xC000&&Addr<0xFC00){
        return true;
    }
#endif	
    ret = WriteCmd(EBuffer, nWriteSize);
    if(ret<0)
        return false;

#if IC_Module == IC_Module_A8018
    Sleep( 30 ); // for non-clock stretch
    if ( A8018_CheckFlashStatusCRCSW( 2, g_CRC24_PAGE_1K_FF, 0 ) <= 0 )
        return false;
#else
    ret = GetResponse_SW(&CK);
    if(ret==false)
        return false;
#endif

    return true;

}

//------------------------------------------------------------------
//|0x72|0x09|0x00|0x00|0x82|0x00|BlockNum|0x00|ChecksumL|ChecksumH|0x00|0x00|
//
//|0x72|0x09|0x00|0x00|0x81|Data0 |Data1 |Data2 |Data3 |Data4 |Data5 |Data6 |
//|0x72|0x09|0x00|0x00|0x81|Data7 |Data8 |Data9 |Data10|Data11|Data12|Data13|
//...
//|0x72|0x09|0x00|0x00|0x81|Data511 |0x00|0x00|0x00|0x00|0x00|0x00
//------------------------------------------------------------------
bool WriteFlashBlock_SW(unsigned int Addr,unsigned char *Data)
{
    unsigned short Checksum=0,ChecksumR=0;
    unsigned char WBuffer[BulkLen]={0};
    int i,ret,iData=0;

    if( (Addr&0x1FF)!=0 )
    {
        return false;
    }

    for(i=0 ; i<BlockSize ; i++)
    {
        Checksum += (unsigned short)Data[i];
    }

    WBuffer[0] = SWISP_Write_Flash;
    WBuffer[1] = 0;
    WBuffer[2] = (Addr >> 9) & 0xFF;
    WBuffer[3] = 0;
    WBuffer[4] = Checksum & 0xFF;
    WBuffer[5] = Checksum >> 8;
    WBuffer[6] = 0;    
    WBuffer[7] = 0;

    ret = WriteCmd(WBuffer, SWISPLength);
    if(ret<0)
    {
        return false;
    }

    WBuffer[0] = SWISP_Write_Data;
    while(iData<BlockSize)
    {
        for(i=1; (i<SWISPLength) && (iData<BlockSize) ; i++,iData++)
        {
            WBuffer[i] = Data[iData];
        }
        ret = WriteCmd(WBuffer, SWISPLength);
        if(ret<0)
        {
            return false;
        }
    }

    ret = GetResponse_SW(&ChecksumR);
    if(ret==false)
    {
        return false;
    }
    return true;
}
//ST1536------------------------------------------------------------------
//|0x72|0x09|0x00|0x00|0x82|0x00|PageNum|0x00|ChecksumL|ChecksumH|0x00|0x00|
//
//|0x72|0x09|0x00|0x00|0x81|Data0 |Data1 |Data2 |Data3 |Data4 |Data5 |Data6 |
//|0x72|0x09|0x00|0x00|0x81|Data7 |Data8 |Data9 |Data10|Data11|Data12|Data13|
//...
//|0x72|0x09|0x00|0x00|0x81|Data252 |Data253|Data254|Data255|0x00|0x00|0x00
//------------------------------------------------------------------

bool   Write_A8010_SPI_ISP(unsigned int Addr,unsigned char *Data)
{
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0,DataCheckSum=0;    
    BYTE pStatus[8]={0};
    pCheckSum=0;
    //UnLock
    RBuffer[0] = ST1801_SWISP_Unlock_Flash;
    RBuffer[1] = 0x01; //Valid Data Size  
    ChecksumCalculation(&pCheckSum,RBuffer,2);
    RBuffer[2] = pCheckSum;	//CheckSum   
#ifdef _DEBUG
    CString strOut;
    strOut.Format("Write Page Unlock Addr:0x%04X Unlock. 0x%02X  0x%02X  0x%02X\r\n",Addr,RBuffer[2],RBuffer[3],RBuffer[4]);
    OutputDebugString(strOut);
#endif // _DEBUG     
    WriteI2CReg_ProtocolA(RBuffer,0xD0,3);
    pStatus[0]=0x01;
    WriteI2CReg_ProtocolA(pStatus,0xF8,1);    
    SleepInProgram(nSleepTime);
    pStatus[0]=0x00;
    ReadI2CReg_ProtocolA(pStatus,0xF8,1);
    if(pStatus[0]!=0x00){
        return false;
    }

    //Program
    pCheckSum=0;
    RBuffer[0] = ST1801_SWISP_Page_Program;
    RBuffer[1] = 0x03; //Valid Data Size 
    RBuffer[2] = Addr/1024;	    //Page Num    
    ChecksumCalculation(&DataCheckSum,Data,0x400);
    RBuffer[3] = DataCheckSum;	//CheckSum   
    ChecksumCalculation(&pCheckSum,RBuffer,0x04);
    RBuffer[4] = pCheckSum;	//CheckSum   
    WriteI2CReg_ProtocolA(RBuffer,0xD0,5);
    WriteI2CReg_ProtocolA(Data,0x200,0x400);
    pStatus[0]=0x01;
    WriteI2CReg_ProtocolA(pStatus,0xF8,1);
    SleepInProgram(10);
    ReadI2CReg_ProtocolA(pStatus,0xF8,1);
#ifdef _DEBUG
    strOut.Format("Write Page Addr:0x%04X Unlock. 0x%02X  0x%02X  0x%02X\r\n",Addr,RBuffer[2],RBuffer[3],RBuffer[4]);
    OutputDebugString(strOut);
#endif // _DEBUG     
    if(pStatus[0]==0x00 ){
    }else if(pStatus[0]!=0x01){
        int nCount = 10;
        do 
        {
#ifdef _DEBUG
            OutputDebugString("Delay 10ms\r\n");
#endif // _DEBUG
            SleepInProgram(10);
            ReadI2CReg_ProtocolA(pStatus,0xF8,1);
            nCount--;
#ifdef _DEBUG
            OutputDebugString("Write Waiting..\r\n");
#endif // _DEBUG
        } while (nCount>0);
        if(pStatus[0]!=0x00){
            return false;
        }
    }else{
#ifdef _DEBUG
        CString strOut;
        strOut.Format("Write Flash Error 0x%02X\r\n",pStatus[0]);
        OutputDebugString(strOut);
#endif // _DEBUG
    }
    return true;
}
bool   Write_A8008_SPI_ISP(unsigned int Addr,unsigned char *Data)
{
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0,DataCheckSum=0;    
    BYTE pStatus[8]={0};
    pCheckSum=0;
    //UnLock
    RBuffer[0] = ST1801_SWISP_Unlock_Flash;
    RBuffer[1] = 0x01; //Valid Data Size  
    ChecksumCalculation(&pCheckSum,RBuffer,2);
    RBuffer[2] = pCheckSum;	//CheckSum   
#ifdef _DEBUG
    CString strOut;
    strOut.Format("Write Page Unlock Addr:0x%04X Unlock. 0x%02X  0x%02X  0x%02X\r\n",Addr,RBuffer[2],RBuffer[3],RBuffer[4]);
    OutputDebugString(strOut);
#endif // _DEBUG     
    WriteI2CReg_ProtocolA(RBuffer,0xD0,3);
    pStatus[0]=0x01;
    WriteI2CReg_ProtocolA(pStatus,0xF8,1);    
    SleepInProgram(nSleepTime);
    pStatus[0]=0x00;
    ReadI2CReg_ProtocolA(pStatus,0xF8,1);
    if(pStatus[0]!=0x00){
        return false;
    }

    //Program
    pCheckSum=0;
    RBuffer[0] = ST1801_SWISP_Page_Program;
    RBuffer[1] = 0x03; //Valid Data Size 
    RBuffer[2] = Addr/1024;	    //Page Num    
    ChecksumCalculation(&DataCheckSum,Data,0x400);
    RBuffer[3] = DataCheckSum;	//CheckSum   
    ChecksumCalculation(&pCheckSum,RBuffer,0x04);
    RBuffer[4] = pCheckSum;	//CheckSum   
    WriteI2CReg_ProtocolA(RBuffer,0xD0,5);
    WriteI2CReg_ProtocolA(Data,0x200,0x400);
    pStatus[0]=0x01;
    WriteI2CReg_ProtocolA(pStatus,0xF8,1);
    SleepInProgram(10);
    ReadI2CReg_ProtocolA(pStatus,0xF8,1);
#ifdef _DEBUG
    strOut.Format("Write Page Addr:0x%04X Unlock. 0x%02X  0x%02X  0x%02X\r\n",Addr,RBuffer[2],RBuffer[3],RBuffer[4]);
    OutputDebugString(strOut);
#endif // _DEBUG     
    if(pStatus[0]==0x00 ){
    }else if(pStatus[0]!=0x01){
        int nCount = 10;
        do 
        {
#ifdef _DEBUG
            OutputDebugString("Delay 10ms\r\n");
#endif // _DEBUG
            SleepInProgram(10);
            ReadI2CReg_ProtocolA(pStatus,0xF8,1);
            nCount--;
#ifdef _DEBUG
            OutputDebugString("Write Waiting..\r\n");
#endif // _DEBUG
        } while (nCount>0);
        if(pStatus[0]!=0x00){
            return false;
        }
    }else{
#ifdef _DEBUG
        CString strOut;
        strOut.Format("Write Flash Error 0x%02X\r\n",pStatus[0]);
        OutputDebugString(strOut);
#endif // _DEBUG
    }
    return true;
}

int   WriteFlashPage_SW(unsigned int Addr,unsigned char *Data)
{
#if IC_Module==(IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    return Write_ST1801_SW_ISP(Addr,Data);
#endif // IC_Module==IC_Module_A1801
#if IC_Module==IC_Module_A8010
    if(fSPIMode){       
        return Write_A8010_SPI_ISP(Addr,Data);
    }
#elif (IC_Module==IC_Module_A8008 ) || (IC_Module==IC_Module_A8015)
    if(fSPIMode){       
        return Write_A8008_SPI_ISP(Addr,Data);
    }
    if ( g_emCommunicationMode != CommunicationMode::CommunicationMode_TouchLink_Bulk )
        return Write_ST1801_SW_ISP( Addr, Data );
#endif
    //OutputDebugStringA("TTKCoomm WriteFlashPage_SW in\r\n");
    unsigned short Checksum=0,ChecksumR=0;
    const int WBufferSize = PageSize1K+64;
    unsigned char WBuffer[WBufferSize]={0}; // PageSize=256, Page Write 55+63+63+63+12 => 64+64+64+64+64 = 256+64 = 320
    unsigned char RBuffer[BulkLen]={0};
    int i,ret,iData=0;
    int WriteL=0;
    if( (Addr&0xFF)!=0 )
        return false;
#if ( IC_Module==IC_Module_A8008)  ||( IC_Module==IC_Module_A8015)
    unsigned char LowByteChecksum=0;
    for(i=0 ; i<PageSize1K ; i++){
        Checksum += (unsigned short)Data[i];
        LowByteChecksum = (unsigned char)(Checksum&0xFF);
        LowByteChecksum=(LowByteChecksum)>>7|(LowByteChecksum)<<1;
        Checksum = (Checksum&0xFF00)|LowByteChecksum;        
    }
    Checksum = ((Checksum&0xFF00)>>8)+((Checksum&0x00FF)<<8);
#elif IC_Module==IC_Module_A8010	
    Get_CRC16_Calc(&Checksum,Data,PageSize1K);
#endif	
    WBuffer[0] = SWISP_Flash_Advance;
    WBuffer[1] = 0x02; //Valid Data Size L
    WBuffer[2] = 0x00; //Valid Data Size H
    WBuffer[3] = 0x05; // Erase  Unlock
    WBuffer[4] = (Addr >> 10)&0xFF;	    //PageNum 1
    ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);

    WBuffer[0] = SWISP_Flash_Advance;
    WBuffer[1] = 0x06; //Valid Data Size L
    WBuffer[2] = 0x04; //Valid Data Size H
    WBuffer[3] = 0x06; //  Write with Unlock
    WBuffer[4] = (Addr >> 10)&0xFF;	    //PageNum 1
    WBuffer[5] = (Checksum&0xFF);	//Write Checksum L   2
    WBuffer[6] = (Checksum>>8)&0xFF;	//Write Checksum H 3
    WBuffer[7] = 0;  //reserved   4
    WBuffer[8] = 0;  //reserved   5    
    for(i=9; i<64 ; i++,iData++)
    {
        WBuffer[i] = Data[iData];
    }
#if  IC_Module==IC_Module_A8008 

#elif IC_Module==IC_Module_A8010	
    if(Addr>=0xC000&&Addr<0xFC00){
        return true;
    }
#endif	
    ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
    WriteL = iData;
    while(WriteL<PageSize1K)
    {
        WBuffer[0]=Bridge_T_Continuous;
        for(i=1 ; (i<BulkLen)&&(WriteL<PageSize1K) ; i++,WriteL++)
        {
            WBuffer[i] = Data[WriteL];
        }

        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        if(ret<0)
            return ERRORMSG_WRITE_BLUK_FAIL;

        if(ret!=BulkLen)
            return ERRORMSG_NON_COMPLETE_TRANSFER;

    }
    ret = Read_Packet_Bulk(RBuffer, 8);
    if(ret!=1)
        return false;
    if(RBuffer[0]!=0x8F)
        return false;
#if  IC_Module==IC_Module_A8008 
    if( (RBuffer[4]!= (Checksum&0xFF))|| (RBuffer[5]!=((Checksum>>8)&0xFF))  )
    {
        return false;
    }
#elif IC_Module==IC_Module_A8010	
    if( (RBuffer[5]!= (Checksum&0xFF))|| (RBuffer[4]!=((Checksum>>8)&0xFF))  )
    {
        return false;
    }
#endif

    return true;
}


//------------------------------------------------------------------
//ST1232: |0x83|0x00|BlockNum|0x00|0x00|0x00|0x00|0x00|
//ST1572: |0x83|0x00|BlockNum|0x00|0x00|0x00|0x57|0x08|
//
//|Data0 |Data1 |Data2 |Data3 |Data4 |Data5 |Data6 |Data7 |
//|Data8 |Data9 |Data10|Data11|Data12|Data13|Data14|Data15|
//...
//|Data504|Data505|Data506|Data507|Data508|Data509|Data510|Data511|
//------------------------------------------------------------------
#define Password 0x5708
bool ReadFlashBlock_SW(unsigned int Addr,unsigned char *Data)
{
    unsigned char RBuffer[BulkLen]={0};
    int i,ret,iData=0;
    RBuffer[0] = SWISP_Read_Flash;
    RBuffer[1] = 0;
    RBuffer[2] = (Addr >> 9) & 0xFF;
    RBuffer[3] = 0;
    RBuffer[4] = 0;
    RBuffer[5] = 0;
    RBuffer[6] = Password >> 8;
    RBuffer[7] = Password & 0xFF;

    ret = WriteCmd(RBuffer, SWISPLength);
    if(ret<0)    {
        return false;
    }
    while(iData<BlockSize)    {
        ret = ReadCmd(RBuffer, SWISPLength);
        if(ret<0)        {
            return false;
        }
        for(i=0; (i<SWISPLength) && (iData<BlockSize) ; i++,iData++)        {
            Data[iData] = RBuffer[i];
        }
    }    
    return true;
}



//------------------------------------------------------------------
//ST1536: |0x83|0x00|PageNum|0x00|0x00|0x00|0x00|0x00|
//
//|Data0 |Data1 |Data2 |Data3 |Data4 |Data5 |Data6 |Data7 |
//|Data8 |Data9 |Data10|Data11|Data12|Data13|Data14|Data15|
//...
//|Data248|Data249|Data250|Data251|Data252|Data253|Data254|Data255|
//------------------------------------------------------------------
bool   ReadFlashPage_SW(unsigned int Addr,unsigned char *Data)
{ 
    return ReadFlashPage_SW_Advance(Addr,Data);
}

//------------------------------------------------------------------
//ST1536: |0x12|0x00|PageNum|0x00|0x00|0x00|0x00|0x00|
//
//|Data0 |Data1 |Data2 |Data3 |Data4 |Data5 |Data6 |Data7 |
//|Data8 |Data9 |Data10|Data11|Data12|Data13|Data14|Data15|
//...
//|Data248|Data249|Data250|Data251|Data252|Data253|Data254|Data255|
//For A8008 Only
//------------------------------------------------------------------
BOOL SwitchI2C_16Bit_8Bit_ST1801(BOOL f16Bit)
{
    BOOL fRet = false;
    BYTE pData[8]={0};
    int nCount=10;
    if(fSPIMode)
        ReadI2CReg_ProtocolA(pData,0xF1,1);
    else
        ReadI2CReg(pData,0xF1,1);
    do{
        BOOL fNowIs16Bit=false;
        if(f16Bit){
            if ( CommunicationMode::CommunicationMode_TouchLink_Bulk == g_emCommunicationMode )
                ReadI2CReg_2Bytes_ST1801( pData, 0xF1, 1 );
            else if ( CommunicationMode::CommunicationMode_Phone_ADB == g_emCommunicationMode )
                ReadI2CReg_ProtocolA_ADB( pData, 0xF1, 1 );
            else
                ReadI2CReg_ProtocolA_HID( pData, 0xF1, 1 );
            if(pData[0]==0x20){
                fRet = TRUE;
                break;
            }else{
                pData[0]=0x20;
                fNowIs16Bit=false;
            }
        }else{
            if(pData[0]==0x00){
                fRet=true;
                break;
            }else{
                pData[0]=0x00;
                fNowIs16Bit=true;
            }
        }
        if(fNowIs16Bit){
            pData[0]=0x00;
            if(fSPIMode){
                WriteI2CReg_ProtocolA(pData,0xF1,1);
                SleepInProgram(nSleepTime);
                ReadI2CReg_ProtocolA(pData,0xF1,1);
            }else{
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
                if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_TouchLink_Bulk ) {
                    WriteI2CData_2Bytes_ST1801( pData, 0xF1, 1 );
                    SleepInProgram( nSleepTime );
                    ReadI2CReg_2Bytes_ST1801( pData, 0xF1, 1 );
                } else if ( CommunicationMode::CommunicationMode_Phone_ADB == g_emCommunicationMode ) {
                    WriteI2CReg_ProtocolA_ADB( pData, 0xF1, 1 );
                    SleepInProgram( nSleepTime );
                    ReadI2CReg_ProtocolA_ADB( pData, 0xF1, 1 );
                } else {
                    WriteI2CReg_ProtocolA_HID( pData, 0xF1, 1 );
                    SleepInProgram( nSleepTime );
                    ReadI2CReg_ProtocolA_HID( pData, 0xF1, 1 );
                }
#endif // IC_MODULE==IC_ST1801
            }
            pData[0]=pData[1];
        }else{
            pData[0]=0x20;
            WriteI2CReg(pData,0xF1,1);
            ReadI2CReg(pData,0xF1,1);
        }
        nCount--;
    } while (nCount >0);
    return fRet;
}
int ChecksumCalculation(unsigned short *pChecksum,unsigned char *pInData,unsigned long nLen)
{
    unsigned short nChecksum = 0;
    unsigned char lowByteChecksum = 0;   
    for ( unsigned long i = 0; i < nLen; ++i ) {
        nChecksum += pInData[i];
        lowByteChecksum = nChecksum & 0xFF;
        lowByteChecksum = (lowByteChecksum>>7) | (lowByteChecksum << 1);
        nChecksum = (nChecksum & 0xFF00) | lowByteChecksum;
    }

    if ( NULL != pChecksum )
        *pChecksum = nChecksum;

    return nChecksum;
}

bool   UnLock_Flash_ST1801_SW_ISP(BOOL fUse16Bit=false)
{
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0;    
    BYTE pStatus[8]={0};
   
    if(fUse16Bit==false){
        if(SwitchI2C_16Bit_8Bit_ST1801(TRUE)==false){
            return false;
        }
        if(fSPIMode==false){
            fUse16Bit=true;
        }
    }
   
    pCheckSum=0;
    RBuffer[0] = ST1801_SWISP_Unlock_Flash;
    RBuffer[1] = 0x09; //Valid Data Size L    
    RBuffer[2] = 0x55;
    RBuffer[3] = 0xAA;
    RBuffer[4] = 0x55;
    RBuffer[5] = 0x6E;
    RBuffer[6] = 0x4C;
    RBuffer[7] = 0x7F;
    RBuffer[8] = 0x83;
    RBuffer[9] = 0x9B;
    ChecksumCalculation(&pCheckSum,RBuffer,10);
    RBuffer[10] = pCheckSum;	//CheckSum       
    if(fUse16Bit){
        WriteI2CData_2Bytes_ST1801(RBuffer,0xD0,11);
    }else{
        WriteI2CReg(RBuffer,0xD0,11);
    }    
    
    pStatus[0]=0x01;
    if(fUse16Bit){
        WriteI2CData_2Bytes_ST1801(pStatus,0xF8,1);    
    }else{
        WriteI2CReg(pStatus,0xF8,1);    
    }
    SleepInProgram(nSleepTime);
    pStatus[0]=0x00;
    if(fUse16Bit){
        ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
    }else{
        ReadI2CReg(pStatus,0xF8,1);
    }   
    if(pStatus[0]==0x00){
        return true;
    }else if(pStatus[0]==0x01){
        int nCount=10;
        do 
        {
            SleepInProgram(nSleepTime);
            if(fUse16Bit){
                ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
            }else{
                ReadI2CReg(pStatus,0xF8,1);
            }
            if(pStatus[0]==0x00){
                return true;
            }
            nCount--;
            if(nCount==0) break;
        } while (1);     
        if(pStatus[0]==0x00){
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}
bool UnlockST1801BootLoader(BOOL fUse16Bit)
{
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0;    
    BYTE pStatus[8]={0};
    pCheckSum=0;
    RBuffer[0] = ST1801_SWISP_Unlock_Boot_Loader;
    RBuffer[1] = 0x09; //Valid Data Size L
    RBuffer[2] = 0x38;
    RBuffer[3] = 0xB6;
    RBuffer[4] = 0xD2;
    RBuffer[5] = 0x7C;
    RBuffer[6] = 0x49;
    RBuffer[7] = 0xAA;
    RBuffer[8] = 0x55;
    RBuffer[9] = 0xAA;
    ChecksumCalculation(&pCheckSum,RBuffer,10);
    RBuffer[10] = pCheckSum;	//CheckSum   
    if(fUse16Bit){
        WriteI2CData_2Bytes_ST1801(RBuffer,0xD0,11);
    }else{
        WriteI2CReg(RBuffer,0xD0,11);
    }    
    pStatus[0]=0x01;
    if(fUse16Bit){
        WriteI2CData_2Bytes_ST1801(pStatus,0xF8,1);    
    }else{
        WriteI2CReg(pStatus,0xF8,1);    
    }
    SleepInProgram(nSleepTime);
    pStatus[0]=0x00;
    if(fUse16Bit){
        ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
    }else{
        ReadI2CReg(pStatus,0xF8,1);
        if(pStatus[0]==0x00){
            return true;
        }else if(pStatus[0]==0x01){
            int nCount=10;
            do 
            {
                SleepInProgram(nSleepTime);
                if(fUse16Bit){
                    ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
                }else{
                    ReadI2CReg(pStatus,0xF8,1);
                }
                if(pStatus[0]==0x00){
                    return true;
                }
                nCount--;
                if(nCount==0) break;
            } while (1);
            if(pStatus[0]==0x00){
                return true;
            }else{
                return false;
            }
        }else{
            return false;
        }
    }
    return true;
}
bool   Lock_Flash_ST1801_SW_ISP(BOOL fUse16Bit=false)
{
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0;    
    BYTE pStatus[8]={0};
    if(fUse16Bit==false){
        if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
            return false;
        }
    }
    pCheckSum=0;
    RBuffer[0] = ST1801_SWISP_lock_Flash;
    RBuffer[1] = 0x01; //Valid Data Size L
    ChecksumCalculation(&pCheckSum,RBuffer,2);
    RBuffer[3] = pCheckSum;	//CheckSum   
    if(fUse16Bit){
        WriteI2CData_2Bytes_ST1801(RBuffer,0xD0,4);
    }else{
        WriteI2CReg(RBuffer,0xD0,4);
    }    
    pStatus[0]=0x01;
    if(fUse16Bit){
        WriteI2CData_2Bytes_ST1801(pStatus,0xF8,1);    
    }else{
        WriteI2CReg(pStatus,0xF8,1);    
    }
    SleepInProgram(nSleepTime);
    pStatus[0]=0x00;
    if(fUse16Bit){
        ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
    }else{
        ReadI2CReg(pStatus,0xF8,1);
    }
    /*
    pCheckSum=0;
    RBuffer[0] = ST1801_SWISP_Unlock_Boot_Loader;
    RBuffer[1] = 0x09; //Valid Data Size L
    RBuffer[2] = 0x38;
    RBuffer[3] = 0xB6;
    RBuffer[4] = 0xD2;
    RBuffer[5] = 0x7C;
    RBuffer[6] = 0x49;
    RBuffer[7] = 0xAA;
    RBuffer[8] = 0x55;
    RBuffer[9] = 0xAA;
    ChecksumCalculation(&pCheckSum,RBuffer,10);
    RBuffer[10] = pCheckSum;	//CheckSum   
    if(fUse16Bit){
    WriteI2CData_2Bytes_ST1801(RBuffer,0xD0,11);
    }else{
    WriteI2CReg(RBuffer,0xD0,11);
    }    
    pStatus[0]=0x01;
    if(fUse16Bit){
    WriteI2CData_2Bytes_ST1801(pStatus,0xF8,1);    
    }else{
    WriteI2CReg(pStatus,0xF8,1);    
    }
    SleepInProgram(100);
    pStatus[0]=0x00;
    if(fUse16Bit){
    ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
    }else{
    ReadI2CReg(pStatus,0xF8,1);
    }
    */
    if(pStatus[0]==0x00){
        return true;
    }else if(pStatus[0]==0x01){
        int nCount=10;
        do 
        {
            SleepInProgram(10);
            if(fUse16Bit){
                ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
            }else{
                ReadI2CReg(pStatus,0xF8,1);
            }
            if(pStatus[0]==0x00){
                return true;
            }
            nCount--;
            if(nCount==0) break;
        } while (1);
        if(pStatus[0]==0x00){
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}

bool   Chip_Erase_ST1801_SW_ISP()
{
    DWORD dwStart = GetTickCount();
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0;    
    BYTE pStatus[8]={0};
    BYTE pData[8]={0};
    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
        return false;
    }   
    UnLock_Flash_ST1801_SW_ISP();
    pCheckSum=0;
    pCheckSum=0;
    RBuffer[0] = ST1801_SWISP_32K_Erase;
    RBuffer[1] = 0x04; //Valid Data Size L    
    RBuffer[2] = 0; //Addr H
    RBuffer[3] = 0; //Addr M
    RBuffer[4] = 0; //Addr L
    ChecksumCalculation(&pCheckSum,RBuffer,5);
    RBuffer[5] = pCheckSum;	//CheckSum   
    WriteI2CReg(RBuffer,0xD0,8);
    pStatus[0]=0x01;
    WriteI2CReg(pStatus,0xF8,1);    
    SleepInProgram(10);
    pStatus[0]=0x00;
    ReadI2CReg(pStatus,0xF8,1);
    for(int i=0;i<100;i++){
        if(pStatus[0]==0x00){
            break;
        }  
        SleepInProgram(10);
        ReadI2CReg(pStatus,0xF8,1);
    }
    if(pStatus[0]!=0x00)   return false;
    Sector_Erase_ST1801_SW_ISP(31*1024,5*1024);
#ifdef _DEBUG
    CString strTmp;
    strTmp.Format("%s Chip Erase Cost %d ms\r\n",__FUNCTION__,GetTickCount()-dwStart);
    OutputDebugString(strTmp);
#endif // _DEBUG
    return true;
}
bool   Sector_Erase_ST1801_SW_ISP(int Addr,int nLen)
{

    DWORD dwStart = GetTickCount();
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    CString strOut;
    unsigned short pCheckSum=0;    
    BYTE pStatus[8]={0};
    BOOL fRet =true;
    int nPageSize = PageSize4K;
#if (IC_Module_A8015 == IC_Module)
    if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
        nPageSize = PageSize1K;
#endif
    if(SwitchI2C_16Bit_8Bit_ST1801(FALSE)==FALSE){
        return false;
    }
    UnLock_Flash_ST1801_SW_ISP();
    if(SwitchI2C_16Bit_8Bit_ST1801(FALSE)==FALSE){
        return false;
    }
#if (IC_Module==IC_Module_ST1801)
    if((Addr+nLen)>=0xC000){
        UnlockST1801BootLoader(FALSE);
    }
#endif     
    if(fSPIMode==false){
        if(SwitchI2C_16Bit_8Bit_ST1801(FALSE)==FALSE){
            return false;
        }
    }
   
    for(int i=Addr;i<(Addr+nLen);i+=nPageSize){        
        {
            CString strTmp;
            strTmp.Format("Erase Addr 0x%04X\r\n",i);
            OutputDebugString(strTmp);
        }
#if (IC_Module==IC_Module_ST1802)
		if(Addr>=0x1F000){
			return true;
		}
		if((Addr+nLen)>=0x1E000){
			UnlockST1801BootLoader(FALSE);
		}
#endif
        pCheckSum=0;
        RBuffer[0] = ST1801_SWISP_Sector_Erase;
        RBuffer[1] = 0x04; //Valid Data Size L    
        RBuffer[2] = i>>16; //Addr H
        RBuffer[3] = i>>8; //Addr M
        RBuffer[4] = i&0xFF; //Addr L
        ChecksumCalculation(&pCheckSum,RBuffer,5);
        RBuffer[5] = pCheckSum;	//CheckSum   
        WriteI2CReg(RBuffer,0xD0,8);
        pStatus[0]=0x01;
        WriteI2CReg(pStatus,0xF8,1);    
        int nCount=100;
        SleepInProgram(nSleepTime);
        do{
            SleepInProgram(nSleepTime);
            ReadI2CReg(pStatus,0xF8,1);
            if(pStatus[0]==0x00){
                fRet = true;;
                break;
            }
            nCount--;
            if(nCount==0){
                fRet = false;;
                break;
            }
        } while (1);
        pStatus[0]=0x00;
        ReadI2CReg(pStatus,0xF8,1);
        if(pStatus[0]!=0x00){
            fRet =  false;
        }
        // i+=nPageSize;
    }
    
    return fRet;
}

bool   Write_ST1801_SW_ISP_SPI(unsigned int Addr,unsigned char *Data)
{
    DWORD dwStart=GetTickCount();
    BOOL fRet = true;
    CString strTmp;
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0,nPageSize=PageSize;
    unsigned short pCheckSum=0,DataCheckSum=0;    
    BYTE pStatus[8]={0};
    BYTE pTestData[0x100]={0};
    BYTE pTestReadData[0x100]={0xFF};
    pCheckSum=0;
   
    RBuffer[0] = ST1801_SWISP_Page_Program;
    RBuffer[1] = 0x07; //Valid Data Size L
    RBuffer[2] = Addr>>16;	    //Address High
    RBuffer[3] = Addr>>8;	    //Address High
    RBuffer[4] = Addr&0xFF;	    //Address Low
    RBuffer[5] = 0x01;	    //Length for Read High
    RBuffer[6] = 0x00;	//Length for Read Low    
    ChecksumCalculation(&DataCheckSum,Data,nPageSize);
    RBuffer[7] = DataCheckSum;	//CheckSum   
    ChecksumCalculation(&pCheckSum,RBuffer,0x08);
    RBuffer[8] = pCheckSum;	//CheckSum   
    WriteI2CReg_2Bytes_ST1801(RBuffer,0xD0,9);

    if(fSPIMode==true){
        WriteI2CReg_2Bytes_ST1801(Data,0x200,nPageSize);
    }else{
        WriteI2CData_2Bytes_ST1801(Data,0x200,nPageSize);
    }

    pStatus[0]=0x01;
    WriteI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
    SleepInProgram(nSleepTime);
    
    ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
    if(pStatus[0]==0x00 ){
        fRet=true;
    }else if(pStatus[0]!=0x01){
        int nCount = 10;
        
        do 
        {            
            SleepInProgram(nSleepTime);
            ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
            nCount--;
            OutputDebugString("...Write Waiting..\r\n");
        } while (nCount>0);
        if(pStatus[0]!=0x00){
            return false;
        }else{
            return true;
        }
    }else if(pStatus[0]==0x01){
        int nCount = 10;
        
        fRet=false;
        do 
        {
            SleepInProgram(nSleepTime);
            ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
            if(pStatus[0]==0x00){
                fRet=true;
                break;
            }
            nCount--;
#ifdef _DEBUG
            OutputDebugString("...Cmd Process Waiting..\r\n");
#endif // _DEBUG
        } while (nCount>0);
    }else{
        CString strOut;
#ifdef _DEBUG
        strOut.Format("Write Flash Error 0x%02X\r\n",pStatus[0]);
        OutputDebugString(strOut);      
#endif // _DEBUG
        fRet=false;
    }
   
    return fRet;
}
bool   Write_ST1801_SW_ISP(unsigned int Addr,unsigned char *Data)
{
    if(fSPIMode)  return Write_ST1801_SW_ISP_SPI(Addr,Data);
    DWORD dwStart=GetTickCount();
    BOOL fRet = true;
    CString strTmp;
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0,DataCheckSum=0;    
    BYTE pStatus[8]={0};
    BYTE pTestData[0x400]={0};
    BYTE pTestReadData[0x400]={0xFF};
#if  (IC_Module==IC_Module_ST1802)
    if((Addr>=0x1F000)){
        return true;
    }
#endif
    int nPageSize = PageSize;
#if ( IC_Module == IC_Module_A8015 )
    nPageSize = PageSize1K;
#endif

    pCheckSum=0;
    RBuffer[0] = ST1801_SWISP_Page_Program;
    RBuffer[1] = 0x07; //Valid Data Size L
    RBuffer[2] = Addr>>16;	    //Address High
    RBuffer[3] = Addr>>8;	    //Address High
    RBuffer[4] = Addr&0xFF;	    //Address Low
    RBuffer[5] = nPageSize>>8;	//Length for Read High
    RBuffer[6] = 0x00;	//Length for Read Low    
    ChecksumCalculation(&DataCheckSum,Data,nPageSize);
    RBuffer[7] = DataCheckSum;	//CheckSum   
    ChecksumCalculation(&pCheckSum,RBuffer,0x08);
    RBuffer[8] = pCheckSum;	//CheckSum   
    WriteI2CReg_2Bytes_ST1801(RBuffer,0xD0,9);
    WriteI2CData_2Bytes_ST1801(Data,0x200,nPageSize);
    
    pStatus[0]=0x01;
    WriteI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
    SleepInProgram(nSleepTime);
   
    ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
    if(pStatus[0]==0x00 ){
        fRet=true;
#ifdef _DEBUG
        OutputDebugString("Write Success\r\n");
#endif // _DEBUG
    }else if(pStatus[0]==0x81){
        fRet=false;
        OutputDebugStringA("CheckSum Error\r\n");
    }else if(pStatus[0]!=0x01){
        int nCount = 4000;
        do 
        {            
            SleepInProgram(nSleepTime);
            ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
            nCount--;
        } while (nCount>0);
        if(pStatus[0]!=0x00){
            fRet=false;
        }else{
            fRet=true;
#ifdef _DEBUG
            strTmp.Format("Write Success retry %d times\r\n",4000-nCount);
            OutputDebugStringA(strTmp);
#endif // _DEBUG
        }
    }else if(pStatus[0]==0x01){
        int nCount = 4000;
        fRet=false;
        do 
        {
            SleepInProgram(nSleepTime);
            ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
            if(pStatus[0]==0x00){
                fRet=true;
#ifdef _DEBUG
                    strTmp.Format("Write Success retry %d times\r\n",4000-nCount);
                OutputDebugStringA(strTmp);
#endif // _DEBUG
                break;
            }
            nCount--;
        } while (nCount>0);
    }else{
        switch (pStatus[0])
        {
        case  0x80:    OutputDebugString("ISP Status: Unknown command ID.\r\n");       break;
        case  0x81:    OutputDebugString("ISP Status: Command packet checksum error.\r\n");       break;
        case  0x82:    OutputDebugString("ISP Status: Programming data checksum error.\r\n");       break;
        case  0x83:    OutputDebugString("ISP Status: Command time out because flash is busy.\r\n");       break;
        case  0x84:    OutputDebugString("ISP Status: Flash Unlock Key error\r\n");       break;
        case  0x85:    OutputDebugString("ISP Status: Boot Loader Unlock Key error\r\n");       break;
        case  0x86:    OutputDebugString("ISP Status: Flash Not Found\r\n");       break;
        case  0x87:    OutputDebugString("ISP Status: Flash Not Support\r\n");       break;
        }
        fRet=false;
    }
   
    return fRet;
}
bool   Read_ST1801_SW_ISP(unsigned int Addr,unsigned char *Data)
{
    unsigned char RBuffer[PageSize1K]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0;    
    CString strOut;
    BYTE pStatus[8]={0},pVerify[8]={0};
    pCheckSum=0;
    
    RBuffer[0] = ST1801_SWISP_Read_Flash;
    RBuffer[1] = 0x06; //Valid Data Size L
    RBuffer[2] = Addr>>16;	    //Address High
    RBuffer[3] = Addr>>8;	    //Address High
    RBuffer[4] = Addr&0xFF;	    //Address Low
    if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C ) {
		RBuffer[5] = 0x00;
		RBuffer[6] = PageSize_HID;
    } else {
		RBuffer[5] = 0x01;	    //Length for Read High
		RBuffer[6] = 0x00;	//Length for Read Low
    }
#if (IC_Module == IC_Module_A8015)
    if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C ) {
		RBuffer[5] = 0x04;
		RBuffer[6] = 0x00;
    }
#endif
    if(fSPIMode==false || g_emCommunicationMode != CommunicationMode::CommunicationMode_TouchLink_Bulk){
        return ST1801_ReadCommandIO(RBuffer,7,Data,true);
    }
    ChecksumCalculation(&pCheckSum,RBuffer,7);
    RBuffer[7] = pCheckSum;	//CheckSum   
    WriteI2CReg_ProtocolA(RBuffer,0xD0,8);
    
    pStatus[0]=0x01;
    WriteI2CReg_ProtocolA(pStatus,0xF8,1);
    SleepInProgram(nSleepTime);
    ReadI2CReg_ProtocolA(pStatus,0xF8,1);
    if(pStatus[0]==0x00){
        ReadI2CReg_ProtocolA(Data,0x200,PageSize);
        return true;
    }else if(pStatus[0]==0x01){
        int nCount=10;
        do{
            SleepInProgram(nSleepTime);
            pStatus[0]=0xFF;
            if(ReadI2CReg_ProtocolA(pStatus,0xF8,1)<=0){
                return false;
            }
            nCount--;
        } while (pStatus[0]!=0x00 || nCount>=0);      
        if(pStatus[0]==0x00){
            ReadI2CReg_ProtocolA(Data,0x200,PageSize);
            return true;
        }else{
            return false;
        }
    }else{
        switch (pStatus[0])
        {
        case  0x80:    OutputDebugString("ISP Status: Unknown command ID.\r\n");       break;
        case  0x81:    OutputDebugString("ISP Status: Command packet checksum error.\r\n");       break;
        case  0x82:    OutputDebugString("ISP Status: Programming data checksum error.\r\n");       break;
        case  0x83:    OutputDebugString("ISP Status: Command time out because flash is busy.\r\n");       break;
        case  0x84:    OutputDebugString("ISP Status: Flash Unlock Key error\r\n");       break;
        case  0x85:    OutputDebugString("ISP Status: Boot Loader Unlock Key error\r\n");       break;
        case  0x86:    OutputDebugString("ISP Status: Flash Not Found\r\n");       break;
        case  0x87:    OutputDebugString("ISP Status: Flash Not Support\r\n");       break;
        }

        return false;
    }
    return true;
    // return ST1801_ReadCommandIO(RBuffer,7,Data,true);
}
bool   Read_STA8010_SPI(unsigned int Addr,unsigned char *Data)
{
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0;    
    CString strOut;
    BYTE pStatus[8]={0},pVerify[8]={0};
    pCheckSum=0;
    RBuffer[0] = ST1801_SWISP_Read_Flash;
    RBuffer[1] = 0x02; //Valid Data Size L
    RBuffer[2] = Addr/1024;	    //Page Num
#ifdef _DEBUG
    strOut.Format("Page Addr:0x%02X \r\n",Addr,RBuffer[3]);
    OutputDebugString(strOut);
#endif // _DEBUG
    ChecksumCalculation(&pCheckSum,RBuffer,3);
    RBuffer[3] = pCheckSum;	//CheckSum   
    WriteI2CReg_ProtocolA(RBuffer,0xD0,4);
    pStatus[0]=0x01;
    WriteI2CReg_ProtocolA(pStatus,0xF8,1);
    SleepInProgram(nSleepTime);
    ReadI2CReg_ProtocolA(pStatus,0xF8,1);
    if(pStatus[0]==0x00){
        ReadI2CReg_ProtocolA(Data,0x200,0x400);
        return true;
    }else if(pStatus[0]==0x01){
        int nCount=10;
        do{
#ifdef _DEBUG
            OutputDebugString("Flash Read Waiting\r\n");
#endif // _DEBUG
            SleepInProgram(nSleepTime);
            pStatus[0]=0xFF;
            if(ReadI2CReg_ProtocolA(pStatus,0xF8,1)<=0){
                return false;
            }
            nCount--;
        } while (pStatus[0]!=0x00 || nCount>=0);      
        if(pStatus[0]==0x00){
            ReadI2CReg_ProtocolA(Data,0x200,0x400);   
            return true;
        }else{
            return false;
        }
    }else{
        char chErr[255]={0};
        sprintf(&chErr[0],"Read A8010 SW ISP Fail, Ret = 0x%02X\r\n",pStatus[0]);
#ifdef _DEBUG
        OutputDebugString(chErr);
#endif // _DEBUG
        return false;
    }
    return true;
}
bool   Read_STA8008_SPI(unsigned int Addr,unsigned char *Data)
{
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0;    
    CString strOut;
    BYTE pStatus[8]={0},pVerify[8]={0};
    pCheckSum=0;
    RBuffer[0] = ST1801_SWISP_Read_Flash;
    RBuffer[1] = 0x02; //Valid Data Size L
    RBuffer[2] = Addr/1024;	    //Page Num
#ifdef _DEBUG
    strOut.Format("Page Addr:0x%02X \r\n",Addr,RBuffer[3]);
    OutputDebugString(strOut);
#endif // _DEBUG
    ChecksumCalculation(&pCheckSum,RBuffer,3);
    RBuffer[3] = pCheckSum;	//CheckSum   
    WriteI2CReg_ProtocolA(RBuffer,0xD0,4);
    pStatus[0]=0x01;
    WriteI2CReg_ProtocolA(pStatus,0xF8,1);
    SleepInProgram(nSleepTime);
    ReadI2CReg_ProtocolA(pStatus,0xF8,1);
    if(pStatus[0]==0x00){
        ReadI2CReg_ProtocolA(Data,0x200,0x400);
        return true;
    }else if(pStatus[0]==0x01){
        int nCount=10;
        do{
#ifdef _DEBUG
            OutputDebugString("Flash Read Waiting\r\n");
#endif // _DEBUG
            SleepInProgram(nSleepTime);
            pStatus[0]=0xFF;
            if(ReadI2CReg_ProtocolA(pStatus,0xF8,1)<=0){
                return false;
            }
            nCount--;
        } while (pStatus[0]!=0x00 || nCount>=0);      
        if(pStatus[0]==0x00){
            ReadI2CReg_ProtocolA(Data,0x200,0x400);   
            return true;
        }else{
            return false;
        }
    }else{
        char chErr[255]={0};
        sprintf(&chErr[0],"Read A8010 SW ISP Fail, Ret = 0x%02X\r\n",pStatus[0]);
#ifdef _DEBUG
        OutputDebugString(chErr);
#endif // _DEBUG
        return false;
    }
    return true;
}

bool   ReadFlashPage_SW_Advance(unsigned int Addr,unsigned char *Data)
{
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    return Read_ST1801_SW_ISP(Addr,Data);
#endif
#if ( IC_Module == IC_Module_A8015 )
    if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
        return Read_ST1801_SW_ISP( Addr, Data );
#endif
    if(fSPIMode){        
#if (IC_Module==IC_Module_A8008 )|| (IC_Module==IC_Module_A8015)
        return Read_STA8008_SPI(Addr,Data);
#endif
#if IC_Module==IC_Module_A8010
        if(fSPIMode){     
            return Read_STA8010_SPI(Addr,Data);
        }
#endif
    } 
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;

    RBuffer[0] = SWISP_Flash_Advance;
    RBuffer[1] = 0x06; //Valid Data Size L
    RBuffer[2] = 0x00; //Valid Data Size H
    RBuffer[3] = 0x02;	    //Read
#if  IC_Module==IC_Module_A8008 
    RBuffer[3] = 0x02;	    //Read
#elif IC_Module==IC_Module_A8015
    RBuffer[3] = 0x09;	    //Unlock + Read
#endif
    RBuffer[4] = (Addr >> 10)&0xFF;	    //PageNum
    RBuffer[5] = 0x0;	//Reserved
    RBuffer[6] = 0x0;	//Reserved
    RBuffer[7] = (unsigned char)(PageSize1K); //Read L//4
    RBuffer[8] = (unsigned char)(PageSize1K>>8); //Read H//5
#if  IC_Module==IC_Module_A8008 

#elif IC_Module==IC_Module_A8010	
    if(Addr>=0xC000&&Addr<0xFC00){
        memset(Data,0xFF,PageSize1K);
        return true;
    }
#endif	
    ret = WriteCmd_Advancd(RBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    BOOL fFirstPage=true;
    int nIndex=0;
    BYTE pContiunuePackCount=0;
    while(nIndex<(PageSize1K))
    {
        unsigned int nReadNum=0;
        ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
        if(ret<0)        {
            return false;
        }
        BYTE ResponseID = RBuffer[0];
        unsigned short ResponseLength = RBuffer[1] + (RBuffer[2]<<8);
        BYTE ResponseParam = RBuffer[3];
        if(nIndex<PageSize1K){
            if(fFirstPage){
                for (int cnt=0;cnt<60;cnt++)
                    if(nIndex<PageSize1K){
                        Data[nIndex++] = RBuffer[cnt+4];
                    }
                    if(ResponseID!=0x82){
                        return false;
                    }
                    if (ResponseLength!=PageSize1K) {
                        return FALSE;
                    }
                    if (ResponseParam!=0) {
                        return false;
                    }
            }else{
                if(ResponseID!=0xFF){
                    return false;
                }
                for (int cnt=0;cnt<63;cnt++)
                    if(nIndex<PageSize1K){
                        Data[nIndex++] = RBuffer[cnt+1];
                    }
            }
        }

        fFirstPage=false;
    }    
    return true;
}
//------------------------------------------------------------------
//ST1572: |0x8D|0x00|BlockNum|0x00|0x00|0x00|0x00|0x00|
//
//|Data0 |Data1 |Data2 |Data3 |Data4 |Data5 |Data6 |Data7 |
//|Data8 |Data9 |Data10|Data11|Data12|Data13|Data14|Data15|
//...
//|Data504|Data505|Data506|Data507|Data508|Data509|Data510|Data511|
//------------------------------------------------------------------
bool ReadDataFlashBlock_SW(unsigned int Addr,unsigned char *Data)
{
    unsigned char RBuffer[BulkLen]={0};
    int i,ret,iData=0;

    RBuffer[0] = SWISP_Read_DataFlash;
    RBuffer[1] = 0;
    RBuffer[2] = (Addr >> 9) & 0xFF;
    RBuffer[3] = 0;
    RBuffer[4] = 0;
    RBuffer[5] = 0;
    RBuffer[6] = 0;
    RBuffer[7] = 0;

    ret = WriteCmd(RBuffer, SWISPLength);
    if(ret<0)    {
        return false;
    }
    while(iData<BlockSize)
    {
        ret = ReadCmd(RBuffer, SWISPLength);
        if(ret<0)        {
            return false;
        }

        for(i=0; (i<SWISPLength) && (iData<BlockSize) ; i++,iData++)        {
            Data[iData] = RBuffer[i];
        }
    }
    return true;
}

//------------------------------------------------------------------
//ST1572: |0x88|0x00|0x00|0x00|0x00|0x00|0x00|0x00|
//
//|VerIntegral[3]|VerIntegral[2]|VerIntegral[1]|VerIntegral[0]|VerDecimal[3] |VerDecimal[2] |VerDecimal[1] |VerDecimal[0]|
//------------------------------------------------------------------
bool ReadFWVersion_SW(float *Ver)
{
    unsigned char VBuffer[BulkLen]={0};
    int ret;

    VBuffer[0] = SWISP_Get_FWVer;
    VBuffer[1] = 0;
    VBuffer[2] = 0;
    VBuffer[3] = 0;
    VBuffer[4] = 0;
    VBuffer[5] = 0;
    VBuffer[6] = 0;
    VBuffer[7] = 0;

    ret = WriteCmd(VBuffer, SWISPLength);
    if(ret<0)    {
        return false;
    }
    ret = ReadCmd(VBuffer, SWISPLength);
    if(ret<0)    {
        return false;
    }
    *Ver = (float)VBuffer[7]*1000.0 + (float)VBuffer[6]*100.0 + (float)VBuffer[5]*10.0 + (float)VBuffer[4]*1.0 + \
        (float)VBuffer[3]*0.1 + (float)VBuffer[2]*0.01 + (float)VBuffer[1]*0.001 + (float)VBuffer[0]*0.0001 ;
    return true;

}

//------------------------------------------------------------------
//ST1572: |0x8B|0x00|0x00|0x00|0x00|0x00|0x00|0x00|
//
//|ChecksumL|ChecksumH|0x00|0x00|0x00|0x00|0x00|0x00|
//------------------------------------------------------------------
bool ReadChecksum_SW(unsigned int Length,unsigned int *Checksum)
{
    unsigned char CBuffer[BulkLen]={0};
    int ret;

    CBuffer[0] = SWISP_Read_Checksum;
    CBuffer[1] = Length & 0xFF;
    CBuffer[2] = (Length>>8) & 0xFF;
    CBuffer[3] = (Length>>16) & 0xFF;
    CBuffer[4] = (Length>>24) & 0xFF;
    CBuffer[5] = 0;
    CBuffer[6] = 0;
    CBuffer[7] = 0;

    ret = WriteCmd(CBuffer,SWISPLength);
    if(ret<0)    {
        return false;
    }

    ret = ReadCmd(CBuffer,SWISPLength);
    if(ret<0)    {
        return false;
    }
    *Checksum = CBuffer[0] + (CBuffer[1]<<8) ;
    return true;
}



//------------------------------------------------------------------
//ST1572: |0x89|0x00|0x00|0x00|0x00|0x00|0x00|0x00|
//------------------------------------------------------------------
bool JumpBack_SW(void)
{
    unsigned char JBuffer[BulkLen]={0};
    int ret;

    JBuffer[0] = SWISP_Run_APROM;
    JBuffer[1] = 0;
    JBuffer[2] = 0;
    JBuffer[3] = 0;
    JBuffer[4] = 0;
    JBuffer[5] = 0;
    JBuffer[6] = 0;
    JBuffer[7] = 0;

    ret = WriteCmd(JBuffer, SWISPLength);
    if(ret<0)    {
        return false;
    }
    return true;
}

//------------------------------------------------------------------
//|0x10|0x04|0x00|0x00|AddressH|AddressL|16K|
//------------------------------------------------------------------
bool ExEraseFlashPage_Bulk(unsigned short Addr)
{
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    return EraseFlashHW_ST1801(Addr);
#endif
#if IC_Module==IC_Module_A8010
    if(Addr>=0xC000 && Addr <0xFC00){
        return true;
    }
#endif
    int ret;
    char StrShow[100];
    unsigned char EBuffer[BulkLen]={0};    

    if( (Addr&0xFF)!=0 )    {
        return false;
    }

    EBuffer[0] = Bridge_T_HWISP;
    EBuffer[1] = 4; //Length L
    EBuffer[2] = 0; //Length H
    EBuffer[3] = HWISP_Erase;
    EBuffer[4] = Addr >> 7;
    EBuffer[5] = Addr & 0x7F;
#if IC_Module==IC_Module_A8008
    if(Addr >= Flash16k){
        EBuffer[4] = (Addr-Flash16k) >> 7;
        EBuffer[5] = (Addr-Flash16k) & 0x7F;
        EBuffer[6] = 0x80;		
    }else{
        EBuffer[6] = 0x00;		
    }
#elif IC_Module==IC_Module_A8015
    if(Addr >= Flash32k){
        EBuffer[4] = (Addr-Flash32k) >> 7;
        EBuffer[5] = (Addr-Flash32k) & 0x7F;
        EBuffer[6] = 0x80;		
    }else{
        EBuffer[6] = 0x00;		
    }
#elif  IC_Module==IC_Module_A8010	
    EBuffer[4] = Addr >> 8;
    EBuffer[5] = Addr & 0x8F;
    if(Addr >= Flash63k){
        EBuffer[4] = Addr >> 8;
        EBuffer[5] = Addr & 0xFF;
        EBuffer[6] = 0x80; //禬筁16k, 璶硄Touch-Link
    }else{
        EBuffer[6] = 0;
    }
#endif
    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)   {
        return false;
    }
    if(ret!=BulkLen)   {
        return false;
    }
    return true;
}

//------------------------------------------------------------------
//|0x10|0x04|0x00|0x03|
//------------------------------------------------------------------
bool MassEraseFlash_Bulk( void )
{
    int ret;
    unsigned char EBuffer[BulkLen] = { 0 };

    EBuffer[0] = Bridge_T_HWISP;
    EBuffer[1] = 4; //Length L
    EBuffer[2] = 0; //Length H
    EBuffer[3] = HWISP_Mass_Erase;
    EBuffer[6] = 0x00; //All erase
    ret = USBComm_WriteToBulkEx( EBuffer, BulkLen );
    if ( ret < 0 ) {
        return false;
    }
    SleepInProgram( 40 );
    return true;
}

//------------------------------------------------------------------
int   ST1801_SPI_FLASH_LOCK(BOOL fLock)
{
    BYTE EBuffer[BulkLen]={0};
    int ret=0;
    if(fSPIMode){
        EBuffer[0] = 0x75;	//SPI Write
        EBuffer[1] = 3;		//Length L
        EBuffer[2] = 0;		//Length H
        EBuffer[3] = 0;		//Delay1
        EBuffer[4] = 0;		//Delay2
        EBuffer[5] = 0x06;	//Write Enable
    }else{
        EBuffer[0] = 0x14;	
        EBuffer[1] = 2;		
        EBuffer[2] = 0;		

        EBuffer[3] = 0x05;		
        EBuffer[4] = 0x06;	
    }
    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    if(fSPIMode){
        EBuffer[0] = 0x75;	//SPI Write
        EBuffer[1] = 4;		//Length L
        EBuffer[2] = 0;		//Length H
        EBuffer[3] = 0;		//Delay1
        EBuffer[4] = 0;		//Delay2
        EBuffer[5] = 0x01;	//Write Status Register
    }else{
        EBuffer[0] = 0x14;
        EBuffer[1] = 3;		
        EBuffer[2] = 0;		

        EBuffer[3] = 0x05;		        
        EBuffer[4] = 0x01;		        
    }
    if(fSPIMode){
        if(!fLock)
            EBuffer[6] = 0x00;	//Data
        else
            EBuffer[6] = 0x8C;	//Data
    }else{
        if(!fLock)
            EBuffer[5] = 0x00;	//Data
        else
            EBuffer[5] = 0x8C;	//Data
    }
    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    if(CheckST1801_SPI_FLASH_Busy()==false){
        return false;
    }
    return true;
}
int   EraseFlashHW_ST1801_ChipErase()
{
    BYTE EBuffer[BulkLen]={0};
    int ret=0;

    EBuffer[0] = 0x75;	//SPI Write
    EBuffer[1] = 3;		//Length L
    EBuffer[2] = 0;		//Length H

    EBuffer[3] = 0;		//Delay1
    EBuffer[4] = 0;		//Delay2

    EBuffer[5] = 0x06;	//Write Enable

    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    if(pFlashWritePageSize!=0x20){
        //Erase Sector Command
        EBuffer[0] = 0x75;	//SPI Write
        EBuffer[1] = 6;		//Length L
        EBuffer[2] = 0;		//Length H
        EBuffer[3] = 0;		//Delay1
        EBuffer[4] = 0;		//Delay2
        EBuffer[5] = ST1801_SPI_ERASE_32K;	//Chip Erase    
        EBuffer[6] = 0;	//A23-A16
        EBuffer[7] = 0;	//A15-A8
        EBuffer[8] = 0;		//A7-A0
        ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        if(ret<0)    {
            return false;
        }    
#ifdef _DEBUG
        OutputDebugString("32K Erase Start\r\n");
#endif // _DEBUG
        if(CheckST1801_SPI_FLASH_Busy()==false){
            return false;
        }
#ifdef _DEBUG
        OutputDebugString("32K Erase Finish\r\n");
#endif // _DEBUG
    }
#ifdef _DEBUG
    OutputDebugString("Page Erase Start\r\n");
#endif // _DEBUG
    if(pFlashWritePageSize==0x20){
        int Addr=0*1024; //32*1024
        for(int i=0;i<12;i++){       
            EraseFlashHW_ST1801(Addr);
            Addr = Addr+((i+1)*4096);
            if(CheckST1801_SPI_FLASH_Busy()==false){
                return false;
            }
        }
    }else{
        int Addr=32*1024;
        for(int i=0;i<4;i++){     
            EraseFlashHW_ST1801(Addr);
            Addr = Addr+((i+1)*4096);
            if(CheckST1801_SPI_FLASH_Busy()==false){
                return false;
            }
        }
    }
#ifdef _DEBUG
    OutputDebugString("Page Erase End\r\n");
#endif // _DEBUG
    if(CheckST1801_SPI_FLASH_Busy()==false){
        return false;
    }
    return true;
}
int   EraseFlashHW_ST1802(unsigned int Addr)
{
    BYTE EBuffer[BulkLen]={0};
    int ret=0;
   
	ret = GetTouchLinkVersion( &g_TouchLinkVersion );
	if ( ret <= 0 ) return ret;

    EBuffer[0] = 0x14;	//SPI Write
    EBuffer[1] = 0x02;		//Length L
    EBuffer[2] = 0x00;		//Length H

    EBuffer[3] = 0x05;		//Delay1
    EBuffer[4] = 0x06;		//Delay2
    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }

    //Erase Sector Command
    EBuffer[0] = 0x14;	//SPI Read
    EBuffer[1] = 0x05;	//Length L
    EBuffer[2] = 0x00;		//Length H

    EBuffer[3] = 0x05;		//Delay1
    EBuffer[4] = 0x20;		//Delay2
    EBuffer[5] = (unsigned char)(Addr>>16);	//A23-A16
    EBuffer[6] = (unsigned char)(Addr>>8);	//A15-A8
    EBuffer[7] = (unsigned char)(Addr);		//A7-A0

    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    SleepInProgram(1);
    if(CheckST1802_FlashEraseBusyWait()==false){
        return false;
    }
    return true;
}

int   EraseFlashHW_ST1801(unsigned int Addr)
{
    BYTE EBuffer[BulkLen]={0};
    int ret=0;
    
	ret = GetTouchLinkVersion( &g_TouchLinkVersion );
	if ( ret <= 0 ) return ret;

    EBuffer[0] = 0x75;	//SPI Write
    EBuffer[1] = 3;		//Length L
    EBuffer[2] = 0;		//Length H

    EBuffer[3] = 0;		//Delay1
    EBuffer[4] = 0;		//Delay2

    EBuffer[5] = 0x06;	//Write Enable

    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    //Erase Sector Command
    EBuffer[0] = 0x75;	//SPI Write
    EBuffer[1] = 6;		//Length L
    EBuffer[2] = 0;		//Length H

    EBuffer[3] = 0;		//Delay1
    EBuffer[4] = 0;		//Delay2

    EBuffer[5] = 0x20;	//Sector Erase
    EBuffer[6] = (unsigned char)(Addr>>16);	//A23-A16
    EBuffer[7] = (unsigned char)(Addr>>8);	//A15-A8
    EBuffer[8] = (unsigned char)(Addr);		//A7-A0

    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    if(CheckST1801_SPI_FLASH_Busy()==false){
        return false;
    }
    return true;
}
int   ST1802EraseFlashHW(unsigned int Addr , unsigned int Len)
{
    int ret,i=0;
    unsigned int Eindex=0,curPos,rtLen;
    unsigned char PBuffer[PageSize1K*4]; // 1024 bytes
    int PAddr=Addr;
    unsigned char ReadFlag = true;
    unsigned short curPageSize=0;
    unsigned short validMask=0; 

    curPageSize = PageSize4K;
    validMask = 0x1FF;
    if(ST1801FlashWakeUp()==false){
        if(ST1801ResetTouchLinkPullHight()==false){
            return false;
        }
        return false;
    }    
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false) {
        if(ST1801ResetTouchLinkPullHight()==false){
            return false;
        }
        return false;
    }
    
    while(Eindex < Len)
    {                      
        if(fSPIMode){
            ret =  EraseFlashHW_ST1801(PAddr);
        }else{
            ret =  EraseFlashHW_ST1802(PAddr);
        }
        if(ret == false)
        {
            //ShowMessage("EraseFlashHW:EraseFlashPage_Bulk");
            if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
            return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
        }
        if(ReadFlag == true)
        {
            if(ret == false)
            {
                //ShowMessage("EraseFlashHW:WriteFlashPage_Bulk");
                return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
            }
        }
        PAddr+=curPageSize;
        Eindex = PAddr;
    }
    if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
    if(ST1801ResetTouchLinkPullHight()==false){
        return false;
    }
    
    return true;
}
int   EraseFlashHW(unsigned int Addr , unsigned int Len)
{
#if  (IC_Module==IC_Module_ST1802)
    return  ST1802EraseFlashHW( Addr ,  Len);
#endif
#if ( IC_Module == IC_Module_A8018)
    return EraseFlashHW_A8018( Addr, Len, true );
#endif
    int ret,i=0;
    unsigned int Eindex=0,curPos,rtLen;
#if IC_Module==IC_Module_A8008 
    unsigned char PBuffer[PageSize512]; // 1024 bytes
#elif  IC_Module==IC_Module_A8010
    unsigned char PBuffer[PageSize1K]; // 1024 bytes
#elif IC_Module == IC_Module_A8015
    unsigned char PBuffer[PageSize512];
#elif (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    unsigned char PBuffer[PageSize1K*4]; // 1024 bytes
#elif (IC_Module==IC_Module_A8018)
    unsigned char PBuffer[PageSize1K];
#endif
    unsigned short PAddr=(unsigned short)Addr;
    unsigned char ReadFlag = false;
    unsigned short curPageSize=0;
    unsigned short validMask=0; 

#if  IC_Module==IC_Module_A8008
    curPageSize = 512;
    validMask = 0x1FF;
#elif  IC_Module==IC_Module_A8010
    curPageSize = 1024;
    validMask = 0x1FF;
#elif IC_Module == IC_Module_A8015
    curPageSize = 512;
    validMask = 0x1FF;
#elif  (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    curPageSize = 4096;
    validMask = 0x1FF;
    if(ST1801FlashWakeUp()==false){
        if(ST1801ResetTouchLinkPullHight()==false){
            return false;
        }
        return false;
    }
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false) {
        if(ST1801ResetTouchLinkPullHight()==false){
            return false;
        }
        return false;
    }
#endif  
    while(Eindex < Len)
    {        
        ReadFlag = false;
        if(Eindex==0)
        {
            //===============read back===========
            if((PAddr&validMask) || (((Addr+Len)-PAddr)<curPageSize && (Addr+Len)&validMask))
            {
                ret = ExReadFlashPage_Bulk(PAddr&0xFF00,PBuffer);
                if(ret == false)	
                {
                    //ShowMessage("EraseFlashHW:ReadFlashPage_Bulk");
#if  (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
                    if(ST1801ResetTouchLinkPullHight()==false){
                        return false;
                    }
                    if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
#endif
                    return ERRORMSG_ATOM_READ_FLASH_FAIL;
                }
                ReadFlag = true;
            }                 

            for(i=PAddr&validMask ; (i<curPageSize) && (Eindex<Len) ; i++,Eindex++)
            {
                PBuffer[i] = 0xFF;
            }          
        }		else		{
            //===============read back===========
            if(((Addr+Len)-(PAddr&0xFF00))<PageSize && ((Addr+Len)&0xFF))				{
                ret = ExReadFlashPage_Bulk(PAddr&0xFF00,PBuffer);
                if(ret == false)					{
#if  (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
                    if(ST1801ResetTouchLinkPullHight()==false){
                        return false;
                    }
                    if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
#endif
                    return ERRORMSG_ATOM_READ_FLASH_FAIL;
                }
                ReadFlag = true;
            }
            for(i=0 ; (i<curPageSize) && (Eindex<Len) ; i++,Eindex++)
            {
                PBuffer[i] = 0xFF;
            }
        }
        ret = ExEraseFlashPage_Bulk(PAddr&0xFF00);
        if(ret == false)
        {
            //ShowMessage("EraseFlashHW:EraseFlashPage_Bulk");
#if  (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
            if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
#endif
            return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
        }
        if(ReadFlag == true)
        {
            if(ret == false)
            {
                //ShowMessage("EraseFlashHW:WriteFlashPage_Bulk");
                return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
            }
        }
        PAddr+=curPageSize;
    }
#if  (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
    if(ST1801ResetTouchLinkPullHight()==false){
        return false;
    }
#endif
    return true;
}

//------------------------------------------------------------------
int ST1802ReadFlashHW(unsigned char *pReadData, unsigned int nAddr, unsigned int nReadSize)
{
    if ( !pReadData || (nReadSize <= 0) || (nAddr & 0xFF) || (nReadSize & 0xFF) )
        return ERRORMSG_DATA_NULL;

    int ret = GetTouchLinkVersion( &g_TouchLinkVersion );
    if ( ret <= 0 ) return ret;

	if ( !ST1801FlashWakeUp() ) {
		ST1801ResetTouchLinkPullHight();
		return ERRORMSG_FLASH_UNLOCK;
	}
    
    constexpr unsigned int ST1802_MAX_FLASH_SIZE = 0x20000; // 128K
    unsigned int nCurrentReadSize = 0;

    while ( nCurrentReadSize < nReadSize )
    {
        if ( fSPIMode )
            ret = ExReadFlashPage_Bulk_SPI_ST1802( nAddr, &pReadData[nCurrentReadSize] );
        else
            ret = ExReadFlashPage_Bulk_I2C_ST1802( nAddr, &pReadData[nCurrentReadSize] );
        
        if ( !ret ) {
            ST1801ResetTouchLinkPullHight();
            return ERRORMSG_ATOM_READ_FLASH_FAIL;
        }

        nAddr += PageSize1K;
        nCurrentReadSize += PageSize1K;
    }

    if ( !ST1801ResetTouchLinkPullHight() )
        return ERRORMSG_FLASH_LOCK;

    return 1;
}

int ReadFlashHW(unsigned char *Data, unsigned int Addr , unsigned int Len)
{
#if (IC_Module==IC_Module_ST1802) || (IC_Module == IC_Module_ST1801)
    return ST1802ReadFlashHW(Data,Addr,Len);
#elif IC_Module == IC_Module_A8018
    return ReadFlashHW_A8018( Data, Addr, Len );
#endif

    int ret,i=0;
    unsigned int Rindex=0,rtLen,readCnt=0,tmpCnt,dataLen;
    unsigned int startPos,endPos,offset;
    unsigned short curPageSize=0;     

    if(!Data)
        return ERRORMSG_DATA_NULL;

#if (IC_Module==IC_Module_A8008) || (IC_Module==IC_Module_A8015)
    unsigned char PBuffer[PageSize512]={0}; // 1024 bytes
    if(fSPI_ISPMode){
        if(ST1801FlashWakeUp()==false){
            return false;
        }
    }
#elif (IC_Module==IC_Module_A8010 ) || (IC_Module == IC_Module_ST1802) || (IC_Module == IC_Module_ST1801) || (IC_Module == IC_Module_A8018)
    unsigned char PBuffer[PageSize1K]={0}; // 1024 bytes
    if(ST1801FlashWakeUp()==false){
        return false;
    }
#endif

    unsigned short PAddr=(unsigned short)Addr;
    while(Rindex < Len)
    {
#if (IC_Module==IC_Module_A8008) || (IC_Module==IC_Module_A8015)
        curPageSize = PageSize512;
#endif
#if IC_Module==IC_Module_A8010 ||IC_Module==IC_Module_ST1801|| IC_Module==IC_Module_ST1802
        curPageSize = PageSize1K;
#endif
        ret = ExReadFlashPage_Bulk((PAddr&0xFF00),PBuffer);
        if(ret == false)        {
            return ERRORMSG_ATOM_READ_FLASH_FAIL;
        }
        if(Rindex==0)        {   
            for(i=(PAddr&(curPageSize-1)) ; (i<curPageSize) && (Rindex<Len) ; i++,Rindex++)            {
                Data[Rindex] = PBuffer[i];
            }
        }        else        {
            for(i=0 ; (i<curPageSize) && (Rindex<Len) ; i++,Rindex++)            {
                Data[Rindex] = PBuffer[i];
            }
        }
        PAddr+=curPageSize;
    }
#if (IC_Module==IC_Module_ST1801)
    if(ST1801ResetTouchLinkPullHight()==false){
        return false;
    }
#endif
    return true;
}

//------------------------------------------------------------------
int   ExWriteFlashHW(unsigned char *Data, unsigned int Addr ,unsigned int Len)
{
    int ret,i=0;
    unsigned int Windex=0;
    unsigned char PBuffer[PageSize1K]={0};
    unsigned short PAddr=(unsigned short)Addr;
    unsigned short curPageSize=0; 
    unsigned short validMask=0; 
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
#if IC_Module==IC_Module_A8008 ||  IC_Module==IC_Module_A8010	||  IC_Module==IC_Module_A8015
    curPageSize = 256;
    validMask = 0xFF;
#endif  
    while(Windex < Len)
    {
        if(Windex==0)
        {

            //===============read back===========
            if((PAddr&validMask) || (((Addr+Len)-PAddr)<curPageSize && (Addr+Len)&validMask))
            {            
                ret = ExReadFlashPage_Bulk(PAddr&(0xFFFF^validMask), PBuffer);

                if(ret == false)
                {
                    return ERRORMSG_ATOM_READ_FLASH_FAIL;
                }
            } 

            for(i=PAddr&validMask ; (i<curPageSize) && (Windex<Len) ; i++,Windex++)
            {
                PBuffer[i] = Data[Windex];
            }
        }
        else
        {
            //===============read back===========
            if(((Addr+Len)-(PAddr&(0xFFFF^validMask)))<curPageSize && (Addr+Len)&validMask)
            {            
                ret = ExReadFlashPage_Bulk(PAddr&(0xFFFF^validMask), PBuffer);

                if(ret == false)                {
                    return ERRORMSG_ATOM_READ_FLASH_FAIL;
                }
            }

            for(i=0 ; (i<curPageSize) && (Windex<Len) ; i++,Windex++)
            {
                PBuffer[i] = Data[Windex];
            }
        }

        ret = ExEraseFlashPage_Bulk(PAddr&(0xFFFF^validMask));

        if(ret == false)        {
            return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
        }
        ret = ExWriteFlashPage_Bulk(PAddr&(0xFFFF^validMask), PBuffer);
        if(ret == false)        {
            return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
        }

        PAddr+=curPageSize;
    }
    return true;
}
bool ExReadFlashPage_Bulk_1K(unsigned short Addr,unsigned char *Data)
{
    int ret;
    char StrShow[100];
    unsigned char RBuffer[BulkLen]={0};
    short Retry=RetryTimes;
    unsigned short curPageSize=0; 
    curPageSize = PageSize1K;
    if( (Addr&0xFF)!=0 )
    {
        return false;
    }
    RBuffer[0] = Bridge_T_HWISP;
    RBuffer[1] = 6; //Length L
    RBuffer[2] = 0; //Length H 
    RBuffer[3] = HWISP_Read;
    RBuffer[4] = Addr >> 7;
    RBuffer[5] = Addr & 0x7F;
#if IC_Module==IC_Module_A8008
    if(Addr >= Flash16k){
        RBuffer[4] = (Addr-Flash16k) >> 7;
        RBuffer[5] = (Addr-Flash16k) & 0x7F;
        RBuffer[6] = 0x80;		
    }else{
        RBuffer[6] = 0x00;
    }
#elif IC_Module==IC_Module_A8015
    if(Addr >= Flash32k){
        RBuffer[4] = (Addr-Flash32k) >> 7;
        RBuffer[5] = (Addr-Flash32k) & 0x7F;
        RBuffer[6] = 0x80;		
    }else{
        RBuffer[6] = 0x00;
    }
#endif
    RBuffer[7] = (curPageSize & 0xFF);; // 256byte
    RBuffer[8] = (curPageSize>>8);
    while(Retry--)
    {
        ret = USBComm_WriteToBulkEx(RBuffer,BulkLen);
        if(ret<0)
        {
            return false;
        }
        if(ret!=BulkLen)
        {
            return false;
        }    
        ret = Read_Packet_Bulk(Data,curPageSize);
        if(((ret<0)&&(ret!=ERRORMSG_BRIDGE_STATUS_NAK))) //When status is NAK, it retries several times.
        {
            return false;
        }
        else
            return true;
        if(Retry==0)
        {
            return false;    
        }
    }    
    if(Retry==0)
    {
        return false;    
    }
    return true;
}
//------------------------------------------------------------------
int WriteFlashHW_ST1802_SPI_NoReadCheck(unsigned char *Data, unsigned int Addr ,unsigned int Len)
{
    int ret,i=0;
    unsigned int Windex=0,rtLen,curPos,curWriteFlashPos;
    unsigned char flashTypeIndex;
    unsigned short retryCMP = 2,cntCMP;
    unsigned int startPos,endPos,offset,tmpCnt,writeCnt,dataCnt=0;
    unsigned short curPageSize=0; 
    unsigned short validMask=0; 
    DWORD dwStart=GetTickCount();
    CString strTmp;
    curPageSize = 4096;
    validMask = 0x3FF;
   
    unsigned char PBuffer[PageSize1K]={0};
    int PAddr=Addr;
    if(ST1801FlashWakeUp()==false) return false;
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
    while(Windex < Len)
    {
        if(fSPIMode)
            ret = EraseFlashHW_ST1801(PAddr);
        else
            ret = EraseFlashHW_ST1802(PAddr);
        if(ret == false){
            if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
            return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
        }     
        PAddr+=curPageSize;
        Windex+=curPageSize;
    }
    if(ST1801FlashWakeUp()==false) return false;
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    Windex=0;
    PAddr=Addr;
    if(pFlashWritePageSize==0x20){
        if(pFlashWritePageSize==32){
            curPageSize = 32; //MXIC one Page 32Bytes
            validMask = 32;
        }else if(pFlashWritePageSize==0xFF){
            curPageSize = 256;
            validMask = 0xFF;
        }
    }else{
        curPageSize = 256;
        validMask = 0xFF;
    }
    dwStart=GetTickCount();
    while(Windex < Len)
    {    
        memset(PBuffer,0xFF,curPageSize);
        if(Windex==0)
        {
            for(i=PAddr&validMask ; (i<curPageSize) && (Windex<Len) ; i++,Windex++){
                PBuffer[i] = Data[Windex];
            }
        }
        else
        {
            //===============read back===========
            for(i=0 ; (i<curPageSize) && (Windex<Len) ; i++,Windex++){
                PBuffer[i] = Data[Windex];
            }
        }       
        ret = ExWriteFlashPage_Bulk_ST1802(PAddr, PBuffer);        
        if(ret == false)
        {
            if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
            return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
        }
        PAddr+=curPageSize;
    }
    BOOL fRet = true;
    if(ST1801_SPI_FLASH_LOCK(TRUE)==false)  
        return false;    
    if(ST1801ResetTouchLinkPullHight()==false){
        return false;
    }
    return fRet;
}
int WriteFlashHW_ST1802_SPI(unsigned char *Data, unsigned int Addr ,unsigned int Len)
{
    int ret=1,i=0;
    unsigned int Windex=0,rtLen,curPos,curWriteFlashPos;
    unsigned char flashTypeIndex;
    unsigned short retryCMP = 2,cntCMP;
    unsigned int startPos,endPos,offset,tmpCnt,writeCnt,dataCnt=0;
    unsigned short curPageSize=0; 
    unsigned short validMask=0; 

    ret = GetTouchLinkVersion( &g_TouchLinkVersion );
    if ( ret <= 0 ) return ret;

    /////////////////////////////////Test Param/////////////////////////////////////////
    int nCount =0;


    //////////////////////////////////////////////////////////////////////////
    DWORD dwStart=GetTickCount();
    CString strTmp;
    curPageSize = 4096;
    validMask = 0x3FF;
    
    unsigned char PBuffer[PageSize1K]={0};
    int PAddr=Addr;
    if(ST1801FlashWakeUp()==false) return false;
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    
    strTmp.Format("------- LIB ---------   %s, FlashWakeUp + Unlock cost %d ms\r\n",__FUNCTION__,GetTickCount()-dwStart);
    OutputDebugString(strTmp);
    dwStart = GetTickCount();
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
   
    while(Windex < Len)
    {
        if(fSPIMode)
            ret = EraseFlashHW_ST1801(PAddr);
        else
            ret = EraseFlashHW_ST1802(PAddr);
        if(ret == false){
            if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
            return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
        }     
        PAddr+=curPageSize;
        Windex+=curPageSize;
    }
    
    strTmp.Format("------- LIB ---------   %s, Erase cost %d ms\r\n",__FUNCTION__,GetTickCount()-dwStart);
    OutputDebugString(strTmp);
    dwStart = GetTickCount();
    if(ST1801FlashWakeUp()==false) return false;
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
     strTmp.Format("------- LIB ---------   %s, FlashWakeUp + Unlock cost %d ms\r\n",__FUNCTION__,GetTickCount()-dwStart);
    OutputDebugString(strTmp);
    dwStart = GetTickCount();
    Windex=0;
    PAddr=Addr;
    if(pFlashWritePageSize==0x20){
        if(pFlashWritePageSize==32){
            curPageSize = 32; //MXIC one Page 32Bytes
            validMask = 32;
        }else if(pFlashWritePageSize==0xFF){
            curPageSize = 256;
            validMask = 0xFF;
        }
    }else{
        curPageSize = 256;
        validMask = 0xFF;
    }

    while(Windex < Len)
    {    
        memset(PBuffer,0xFF,curPageSize);
        
        if(Windex==0)
        {                
            for(i=0 ; (i<curPageSize) && (Windex<Len) ; i++,Windex++){
                PBuffer[i] = Data[Windex];
            }
        }
        else
        {
            //===============read back===========
            for(i=0 ; (i<curPageSize) && (Windex<Len) ; i++,Windex++){
                PBuffer[i] = Data[Windex];
            }
        }       
        
        ret = ExWriteFlashPage_Bulk_ST1802(PAddr, PBuffer);        
        if(ret == false)
        {
            if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
            return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
        }
        PAddr+=curPageSize;
        strTmp.Format("------- LIB ---------   %s, Write %d cost %d ms\r\n",__FUNCTION__,nCount,GetTickCount()-dwStart);
        OutputDebugString(strTmp);
        nCount++;
        dwStart = GetTickCount();
    }

    BOOL fRet = true;
    if(fWriteflashNoRead==false){
        BYTE pReadBuffer[0x20000]={0};    	 
        if(ST1801_SPI_FLASH_LOCK(TRUE)==false)  
            return false;
        dwStart=GetTickCount();
        Sleep(10);
        ReadFlashHW(pReadBuffer,Addr,Len);    
        for(int i=Addr;i<Len;i++){
            if(Data[i]!=pReadBuffer[i]){
                fRet=false;
            }
        }
    }
    strTmp.Format("------- LIB ---------   %s, Read Comare cost %d ms\r\n",__FUNCTION__,GetTickCount()-dwStart);
    OutputDebugString(strTmp);
    nCount++;
    if(ST1801ResetTouchLinkPullHight()==false){
        return false;
    }
    return fRet;
}

int WriteFlashHW_ST1801_SPI_MassErase(unsigned char *Data, unsigned int Addr ,unsigned int Len)
{
    int ret,i=0;
    unsigned int Windex=0,rtLen,curPos,curWriteFlashPos;
    unsigned char flashTypeIndex;
    unsigned short retryCMP = 2,cntCMP;
    unsigned int startPos,endPos,offset,tmpCnt,writeCnt,dataCnt=0;
    unsigned short curPageSize=0; 
    unsigned short validMask=0; 
    CString strTmp;
    DWORD dwStart=GetTickCount();
    curPageSize = 4096;
    validMask = 0x3FF;
    unsigned char PBuffer[PageSize1K]={0};
    unsigned short PAddr=(unsigned short)Addr;
    if(ST1801FlashWakeUp()==false) return false;
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    if(Data==NULL)   return ERRORMSG_DATA_NULL;
    ret = EraseFlashHW_ST1801_ChipErase();  
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    Windex=0;
    PAddr=Addr;
    if(pFlashWritePageSize==0x20){
        curPageSize = 32;//256;   MXIC one Page 32Byts
        validMask = 32;
    }else{
        curPageSize = 256;
        validMask = 0xFF;
    }
    dwStart=GetTickCount();
    while(Windex < Len)
    {    
        if(Windex==0)
        {
            for(i=PAddr&validMask ; (i<curPageSize) && (Windex<Len) ; i++,Windex++){
                PBuffer[i] = Data[Windex];
            }
        }
        else
        {
            //===============read back===========
            for(i=0 ; (i<curPageSize) && (Windex<Len) ; i++,Windex++){
                PBuffer[i] = Data[Windex];
            }
        }
        if(pFlashWritePageSize==0x20){
            ret = ExWriteFlashPage_Bulk(PAddr, PBuffer);
        }else{
            ret = ExWriteFlashPage_Bulk(PAddr&(0xFFFF^validMask), PBuffer);
        }
        if(ret == false)
        {
            if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
            return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
        }
        PAddr+=curPageSize;
    }

    BOOL fRet = true;
    BYTE pReadBuffer[0x20000]={0};    	 
    if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
    dwStart=GetTickCount();
    /*
    ReadFlashHW(pReadBuffer,Addr,Len);
    for(int i=Addr;i<Len;i++){
    if(Data[i]!=pReadBuffer[i]){
    fRet=false;
    }
    }
    */
    if(ST1801ResetTouchLinkPullHight()==false){
        return false;
    }
    return fRet;
}

int WriteFlashHW(unsigned char *Data, unsigned int Addr ,unsigned int Len)
{
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    return WriteFlashHW_ST1802_SPI( Data, Addr, Len );
#endif
#if (IC_Module == IC_Module_A8018)
    return WriteFlashHW_A8018( Data, Addr, Len );
#endif
    int ret,i=0;
    unsigned int Windex=0,rtLen,curPos,curWriteFlashPos;
    unsigned char flashTypeIndex;
    unsigned short retryCMP = 2,cntCMP;
    unsigned int startPos,endPos,offset,tmpCnt,writeCnt,dataCnt=0;
    unsigned short curPageSize=0; 
    unsigned short validMask=0; 

    curPageSize = 1024;
    validMask = 0x3FF;
    unsigned char PBuffer[PageSize1K]={0};
    unsigned short PAddr=(unsigned short)Addr;

    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
    while(Windex < Len)
    {
        if(Windex==0)
        {
            //===============read back===========
            if((PAddr&validMask) || (((Addr+Len)-PAddr)<curPageSize && (Addr+Len)&validMask))
            {            
                ret = ExReadFlashPage_Bulk(PAddr&(0xFFFF^validMask), PBuffer);

                if(ret == false)
                {
                    return ERRORMSG_ATOM_READ_FLASH_FAIL;
                }
            } 

            for(i=PAddr&validMask ; (i<curPageSize) && (Windex<Len) ; i++,Windex++)
            {
                PBuffer[i] = Data[Windex];
            }
        }
        else
        {
            //===============read back===========
            if(((Addr+Len)-(PAddr&(0xFFFF^validMask)))<curPageSize && (Addr+Len)&validMask)
            {            
                ret = ExReadFlashPage_Bulk(PAddr&(0xFFFF^validMask), PBuffer);
                if(ret == false)
                {
                    return ERRORMSG_ATOM_READ_FLASH_FAIL;
                }
            }

            for(i=0 ; (i<curPageSize) && (Windex<Len) ; i++,Windex++)
            {
                PBuffer[i] = Data[Windex];
            }
        }
        ret = ExEraseFlashPage_Bulk(PAddr&(0xFFFF^validMask));
        if(ret == false)
        {
            return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
        }

        ret = ExWriteFlashPage_Bulk(PAddr&(0xFFFF^validMask), PBuffer);

        if(ret == false)
        {
            return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
        }
        PAddr+=curPageSize;
    }
    BOOL fRet = true;


    BYTE pReadBuffer[0x10000]={0};  
    int nCount=0;
    ReadFlashHW(pReadBuffer,Addr,Len);
    for(int i=Addr;i<Len;i++){
        if(Data[i]!=pReadBuffer[i]){
#if ( IC_Module==IC_Module_A8008) || (IC_Module==IC_Module_A8015)
            fRet=false;
#elif IC_Module==IC_Module_A8010	
            if(i<0xC000 || i>0xFC00){
                fRet=false;
            }
#endif	
        }
    }

    return fRet;
}

int WriteFlashHW_NoErase(unsigned char *Data, unsigned int Addr ,unsigned int Len)
{
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    int ret,i=0;
    unsigned int Windex=0,rtLen,curPos,curWriteFlashPos;
    unsigned char flashTypeIndex;
    unsigned short retryCMP = 2,cntCMP;
    unsigned int startPos,endPos,offset,tmpCnt,writeCnt,dataCnt=0;
    unsigned short curPageSize=0; 
    unsigned short validMask=0; 
    DWORD dwStart=GetTickCount();
    CString strTmp;
    curPageSize = 4096;
    validMask = 0x3FF;
    unsigned char PBuffer[PageSize1K]={0};
    int PAddr=Addr;
    if(ST1801FlashWakeUp()==false) return false;
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
    Windex=0;
    PAddr=Addr;
    if(pFlashWritePageSize==0x20){
        if(pFlashWritePageSize==32){
            curPageSize = 32; //MXIC one Page 32Bytes
            validMask = 32;
        }else if(pFlashWritePageSize==0xFF){
            curPageSize = 256;
            validMask = 0xFF;
        }
    }else{
        curPageSize = 256;
        validMask = 0xFF;
    }
    dwStart=GetTickCount();
    while(Windex < Len)
    {    
        memset(PBuffer,0xFF,curPageSize);
        if(Windex==0)
        {
            for(i=PAddr&validMask ; (i<curPageSize) && (Windex<Len) ; i++,Windex++){
                PBuffer[i] = Data[Windex];
            }
        }
        else
        {
            //===============read back===========
            for(i=0 ; (i<curPageSize) && (Windex<Len) ; i++,Windex++){
                PBuffer[i] = Data[Windex];
            }
        }       
        ret = ExWriteFlashPage_Bulk_ST1802(PAddr, PBuffer);        
        if(ret == false)
        {
            if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
            return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
        }
        PAddr+=curPageSize;
    }
    BOOL fRet = true;
    BYTE pReadBuffer[0x20000]={0};    	 
    Sleep(100);
    if(ST1801_SPI_FLASH_LOCK(TRUE)==false)  
        return false;
    dwStart=GetTickCount();

    ReadFlashHW(pReadBuffer,Addr,Len);    
    for(int i=Addr;i<Len;i++){
        if(Data[i]!=pReadBuffer[i]){
            fRet=false;
        }
    }
    if(ST1801ResetTouchLinkPullHight()==false){
        return false;
    }
    return fRet;
#else

    //for A8008 page = 1024Bytes
    int ret,i=0;
    unsigned int Windex=0,rtLen,curPos,curWriteFlashPos;
    unsigned char flashTypeIndex;
    unsigned short retryCMP = 2,cntCMP;
    unsigned int startPos,endPos,offset,tmpCnt,writeCnt,dataCnt=0;
    unsigned short curPageSize=0; 
    unsigned short validMask=0; 
    curPageSize = 1024;
    validMask = 0x3FF;
    unsigned char PBuffer[PageSize1K]={0};
    unsigned short PAddr=(unsigned short)Addr;
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
    while(Windex < Len)
    {
        if(Windex==0)
        {
            //===============read back===========
            if((PAddr&validMask) || (((Addr+Len)-PAddr)<curPageSize && (Addr+Len)&validMask))
            {            
                ret = ExReadFlashPage_Bulk(PAddr&(0xFFFF^validMask), PBuffer);
                if(ret == false)
                {
                    return ERRORMSG_ATOM_READ_FLASH_FAIL;
                }
            } 
            for(i=PAddr&validMask ; (i<curPageSize) && (Windex<Len) ; i++,Windex++)
            {
                PBuffer[i] = Data[Windex];
            }
        }
        else
        {
            //===============read back===========
            if(((Addr+Len)-(PAddr&(0xFFFF^validMask)))<curPageSize && (Addr+Len)&validMask)
            {            
                ret = ExReadFlashPage_Bulk(PAddr&(0xFFFF^validMask), PBuffer);
                if(ret == false)
                {
                    return ERRORMSG_ATOM_READ_FLASH_FAIL;
                }
            }

            for(i=0 ; (i<curPageSize) && (Windex<Len) ; i++,Windex++)
            {
                PBuffer[i] = Data[Windex];
            }
        }
        ret = ExWriteFlashPage_NoVerify_Bulk(PAddr&(0xFFFF^validMask), PBuffer);
        if(ret == false)
        {
            return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
        }
        PAddr+=curPageSize;
    }
    return true;
#endif
}

int WriteFlashSW_NoErase(unsigned char *Data, unsigned int Addr ,unsigned int Len)
{
    //only for ST1802 ST1801
    if(fSPIMode){                    
        int ret,i=0;
        unsigned int Windex=0;
        unsigned char BBuffer[PageSize1K]={0};
        unsigned int BAddr = Addr;
        char strBuf[0x100],*pstrBuf=strBuf;
        CString strTmp;
        if(fSPI_ISPMode==false)
            ST1801FlashWakeUp();
        if(SwitchI2C_16Bit_8Bit_ST1801(FALSE)==FALSE){
            return false;
        }
        UnLock_Flash_ST1801_SW_ISP();
        if(fSPI_ISPMode==true){
            if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
                return false;
            }
        }
        if(fSPI_ISPMode==true){
            if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
                return false;
            }
        }
        if(Data==NULL)
            return ERRORMSG_DATA_NULL;
        unsigned int BAddrMask=0;
        unsigned int BAddrMaskComplete=0;
        int	BankSize=0;

        BAddrMask = 0xFF;
        BAddrMaskComplete = 0xFFFFFF00;
        BankSize = PageSize1K;
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
        BankSize = PageSize;
#endif
        DWORD dwStart=GetTickCount();
        while((Windex < Len) ){
            if(Windex==0){
                //===============read back===========         
                if((BAddr&BAddrMask) || (((Addr+Len)-BAddr)<BankSize && (Addr+Len)&BAddrMask)){
                    ret = ReadFlashPage_SW(BAddr&BAddrMaskComplete, BBuffer);
                    if(ret == false)
                        return ERRORMSG_ATOM_READ_FLASH_FAIL;
                } 
                for(i=BAddr&BAddrMask ; (i<BankSize) && (Windex<Len) ; i++,Windex++){
                    BBuffer[i] = Data[Windex];
                }
            }else{
                //===============read back===========
                if(((Addr+Len)-(BAddr&BAddrMaskComplete))<BankSize && (Addr+Len)&BAddrMask){
                    ret = ReadFlashPage_SW(BAddr&BAddrMaskComplete, BBuffer);
                    if(ret == false){
                        if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                            return false;
                        }
                        return ERRORMSG_ATOM_READ_FLASH_FAIL;
                    }
                }
                for(i=0 ; (i<BankSize) && (Windex<Len) ; i++,Windex++){
                    BBuffer[i] = Data[Windex];
                }
            }
            DWORD dwWriteStart=GetTickCount();
#if (IC_Module==IC_Module_ST1802)
            if(BAddr<0x1E000)
#else
#endif
            ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer);    
            if(ret == false)
            {
                SleepInProgram(50);
#if (IC_Module==IC_Module_ST1802)
                if(BAddr<0x1E000)
#else
#endif
                ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer);    
                if(ret == false){
                    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                        return false;
                    }
                    return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
                }
            }else if (ret == ERRORMSG_CheckSum_Error){
                ret = EraseFlashPage_SW(BAddr&BAddrMaskComplete);
                if(ret == false){
                    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                        return false;
                    }
                    return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
                }
#if (IC_Module==IC_Module_ST1802)
                if(BAddr<0x1E000)
#else
#endif
                ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer); 
                if(ret==false){
                    SleepInProgram(50);
#if (IC_Module==IC_Module_ST1802)
                    if(BAddr<0x1E000)
#else
#endif
                    ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer); 
                    if(ret==false){
                        if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                            return false;
                        }
                        return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
                    }
                }else if(ret == ERRORMSG_CheckSum_Error){
                    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                        return false;
                    }
                    return ERRORMSG_CheckSum_Error;
                }
            }
            BAddr+=BankSize;       
        }
        Lock_Flash_ST1801_SW_ISP(false);
        return true;
    }else{
        //I2C
        int ret,i=0;
        unsigned int Windex=0;
        unsigned char BBuffer[PageSize1K]={0};
        unsigned int BAddr = Addr;
        char strBuf[0x100],*pstrBuf=strBuf;
        CString strTmp;
        if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
            return false;
        }
        if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
            return false;
        }
        if(Data==NULL)
            return ERRORMSG_DATA_NULL;
        unsigned int BAddrMask=0;
        unsigned int BAddrMaskComplete=0;
        int	BankSize=0;

        BAddrMask = 0xFF;
        BAddrMaskComplete = 0xFFFFFF00;
        BankSize = PageSize;
        DWORD dwStart=GetTickCount();
        while((Windex < Len) ){
            if(Windex==0){
                //===============read back===========         
                if((BAddr&BAddrMask) || (((Addr+Len)-BAddr)<BankSize && (Addr+Len)&BAddrMask)){
                    ret = ReadFlashPage_SW(BAddr&BAddrMaskComplete, BBuffer);
                    if(ret == false)
                        return ERRORMSG_ATOM_READ_FLASH_FAIL;
                } 
                for(i=BAddr&BAddrMask ; (i<BankSize) && (Windex<Len) ; i++,Windex++){
                    BBuffer[i] = Data[Windex];
                }
            }else{
                //===============read back===========
                if(((Addr+Len)-(BAddr&BAddrMaskComplete))<BankSize && (Addr+Len)&BAddrMask){
                    ret = ReadFlashPage_SW(BAddr&BAddrMaskComplete, BBuffer);
                    if(ret == false){
                        if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                            return false;
                        }
                        return ERRORMSG_ATOM_READ_FLASH_FAIL;
                    }
                }
                for(i=0 ; (i<BankSize) && (Windex<Len) ; i++,Windex++){
                    BBuffer[i] = Data[Windex];
                }
            }
            ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer);    
            if(ret == false){
                SleepInProgram(50);
                ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer);    
                if(ret == false){
                    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                        return false;
                    }
                    return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
                }
            }else if (ret == ERRORMSG_CheckSum_Error){
                ret = EraseFlashPage_SW(BAddr&BAddrMaskComplete);
                if(ret == false){
                    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                        return false;
                    }
                    return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
                }
                ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer); 
                if(ret==false){
                    SleepInProgram(50);
                    ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer); 
                    if(ret==false){
                        if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                            return false;
                        }
                        return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
                    }
                }else if(ret == ERRORMSG_CheckSum_Error){
                    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                        return false;
                    }
                    return ERRORMSG_CheckSum_Error;
                }
            }

            BAddr+=BankSize;       
        }
        Lock_Flash_ST1801_SW_ISP(false);
        return true;
    }
}


//------------------------------------------------------------------
bool   Sector_Erase_A8010_SPI_ISP(int Addr,int nLen)
{
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    CString strOut;
    unsigned short pCheckSum=0;    
    BYTE pStatus[8]={0};
    for(int i=0;i<nLen;i+=1024){        
        pCheckSum=0;
        //UnLock
        RBuffer[0] = ST1801_SWISP_Unlock_Flash;
        RBuffer[1] = 0x01; //Valid Data Size  
        ChecksumCalculation(&pCheckSum,RBuffer,2);
        RBuffer[2] = pCheckSum;	//CheckSum   
#ifdef _DEBUG
        strOut.Format("Page Addr:0x%04X Unlock. 0x%02X  0x%02X  0x%02X\r\n",Addr,RBuffer[2],RBuffer[3],RBuffer[4]);
        OutputDebugString(strOut);
#endif // _DEBUG     
        WriteI2CReg_ProtocolA(RBuffer,0xD0,3);
        pStatus[0]=0x01;
        WriteI2CReg_ProtocolA(pStatus,0xF8,1);    
        //SleepInProgram(10);
        pStatus[0]=0x00;
        ReadI2CReg_ProtocolA(pStatus,0xF8,1);
        if(pStatus[0]!=0x00){
            return false;
        }
        //Erase
        pCheckSum=0;
        //UnLock
        RBuffer[0] = ST1801_SWISP_Sector_Erase;
        RBuffer[1] = 0x02; //Valid Data Size
        RBuffer[2] = Addr/1024; //Page Number
        ChecksumCalculation(&pCheckSum,RBuffer,3);
        RBuffer[3] = pCheckSum;	//CheckSum   
#ifdef _DEBUG
        strOut.Format("Page Erase Addr:0x%04X Erase. 0x%02X  0x%02X  0x%02X\r\n",Addr,RBuffer[2],RBuffer[3],RBuffer[4]);
        OutputDebugString(strOut);
#endif // _DEBUG     
        WriteI2CReg_ProtocolA(RBuffer,0xD0,4);
        pStatus[0]=0x01;
        WriteI2CReg_ProtocolA(pStatus,0xF8,1);    
        //SleepInProgram(50);
        pStatus[0]=0x00;
        ReadI2CReg_ProtocolA(pStatus,0xF8,1);
        if(pStatus[0]!=0x00){
            return false;
        }
        Addr+=1024;
    }
    return true;
}
bool   Sector_Erase_A8008_SPI_ISP(int Addr,int nLen)
{
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    CString strOut;
    unsigned short pCheckSum=0;    
    BYTE pStatus[8]={0};
    for(int i=0;i<nLen;i+=1024){        
        pCheckSum=0;
        //UnLock
        RBuffer[0] = ST1801_SWISP_Unlock_Flash;
        RBuffer[1] = 0x01; //Valid Data Size  
        ChecksumCalculation(&pCheckSum,RBuffer,2);
        RBuffer[2] = pCheckSum;	//CheckSum   
#ifdef _DEBUG
        strOut.Format("Page Addr:0x%04X Unlock. 0x%02X  0x%02X  0x%02X\r\n",Addr,RBuffer[2],RBuffer[3],RBuffer[4]);
        OutputDebugString(strOut);
#endif // _DEBUG     
        WriteI2CReg_ProtocolA(RBuffer,0xD0,3);
        pStatus[0]=0x01;
        WriteI2CReg_ProtocolA(pStatus,0xF8,1);    
        //SleepInProgram(10);
        pStatus[0]=0x00;
        ReadI2CReg_ProtocolA(pStatus,0xF8,1);
        if(pStatus[0]!=0x00){
            return false;
        }
        //Erase
        pCheckSum=0;
        //UnLock
        RBuffer[0] = ST1801_SWISP_Sector_Erase;
        RBuffer[1] = 0x02; //Valid Data Size
        RBuffer[2] = Addr/1024; //Page Number
        ChecksumCalculation(&pCheckSum,RBuffer,3);
        RBuffer[3] = pCheckSum;	//CheckSum   
#ifdef _DEBUG
        strOut.Format("Page Erase Addr:0x%04X Erase. 0x%02X  0x%02X  0x%02X\r\n",Addr,RBuffer[2],RBuffer[3],RBuffer[4]);
        OutputDebugString(strOut);
#endif // _DEBUG     
        WriteI2CReg_ProtocolA(RBuffer,0xD0,4);
        pStatus[0]=0x01;
        WriteI2CReg_ProtocolA(pStatus,0xF8,1);    
        SleepInProgram(50);
        pStatus[0]=0x00;        
        ReadI2CReg_ProtocolA(pStatus,0xF8,1);
        if(pStatus[0]!=0x00){
            return false;
        }
        Addr+=1024;
    }
    return true;
}

int EraseFlashSW(unsigned int Addr , unsigned int Len)
{
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    return Sector_Erase_ST1801_SW_ISP(Addr,Len);
#endif // IC_Module==IC_Module_ST1801
#if ( IC_Module == IC_Module_A8015 )
    // A8015 Touch Pad 按 A2152 Protocol 删除
    if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
        return Sector_Erase_ST1801_SW_ISP( Addr, Len );
#endif
#if IC_Module==IC_Module_A8010
    if(fSPIMode){
        return Sector_Erase_A8010_SPI_ISP(Addr,Len);
    }
#endif

#if IC_Module==IC_Module_A8010
    if(fSPIMode){
        return Sector_Erase_A8010_SPI_ISP(Addr,Len);
    }
#endif

#if (IC_Module==IC_Module_A8008) || (IC_Module==IC_Module_A8015)
    if(fSPIMode){
        return Sector_Erase_A8008_SPI_ISP(Addr,Len);
    }
#endif

#if IC_Module == IC_Module_A8018
	if ( (Addr % PageSize1K) || (Len % PageSize1K) || (Addr >= Flash61k) )
		return ERRORMSG_DATA_NULL;
    if ( 0x00 == Addr && Len >= Flash48k ) {
        int res = EraseFlashSW_A8018_Mass();
        if ( res <= 0 || Len <= Flash60k ) return res;
        Addr += Flash60k;
    }
    
#endif

    unsigned int Eindex = 0;
    do 
    {
#if IC_Module == IC_Module_A8018
        if ( Addr >= Flash48k ) {   // skip empty area
            Addr = Flash60k;
            Eindex += (Flash60k - Flash48k);
        }
#endif
        if ( !EraseFlashPage_SW( Addr ) )
            return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
        
        Addr += PageSize1K;
        Eindex += PageSize1K;
    } while ( Eindex < Len );
    return true;
}

int ReadFlashSW(unsigned char *Data, unsigned int Addr , unsigned int Len)
{
    if (g_emCommunicationMode == CommunicationMode::CommunicationMode_Phone_ADB) {
        return ReadFlashSW_ADB(Data, Addr, Len);
    }
#if IC_Module == IC_Module_A8018
	return ReadFlashSW_A8018( Data, Addr, Len );
#endif
     int ret,i=0;

   
    unsigned int Rindex=0;
    unsigned char BBuffer[PageSize1K]={0};
    unsigned int BAddr = Addr;
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
    int istart=0;
    int BankSize=0;

    istart=(BAddr&0xFF); 
    BankSize=PageSize1K;

    unsigned int mask = 0xFFFFFF00;
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
        return false;
    }
    BankSize = PageSize;
   
    if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C ) {
        istart = (BAddr & PageSize_HID);
        BankSize = PageSize_HID;
        mask = 0xFFFFFFFF;
    }
#elif ( IC_Module == IC_Module_A8015 )
    if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C ) {
		if ( SwitchI2C_16Bit_8Bit_ST1801( true ) == false ) {
			return false;
		}
    }
#else
    if(fSPIMode){
        if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
            return false;
        }
    }
#endif
    DWORD dwStart=GetTickCount();
    g_pro = 0;
    while(Rindex < Len)
    {
        //ret = ReadFlashPage_SW((BAddr&0xFFFFFF00), BBuffer);                   
        ret = ReadFlashPage_SW( (BAddr & mask), BBuffer );
        if(ret == false){
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
            if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                OutputDebugStringA("Change to 8bit error!");
                return false;
            }
#endif
            return ERRORMSG_ATOM_READ_FLASH_FAIL;
        }
        if(Rindex==0)
        { 
            for(i=istart ; (i<BankSize) && (Rindex<Len) ; i++,Rindex++)
            {
                Data[Rindex] = BBuffer[i];
            }
        }
        else
        {     
            for(i=0 ; (i<BankSize) && (Rindex<Len) ; i++,Rindex++)          
            {
                Data[Rindex] = BBuffer[i];
            }
        }
        BAddr+=BankSize;      
        g_pro = (int)((double)Rindex / (double)Len * 100.0);
    }
    
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    //OutputDebugStringA("Change to 8 bit\r\n");
    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
       // OutputDebugStringA("Change to 8bit error!");
        return false;
    }
#endif

#if ( IC_Module == IC_Module_A8015 )
    if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C ) {
		if ( SwitchI2C_16Bit_8Bit_ST1801( false ) == false )
			return false;
	}
#endif
    return true;
}

//------------------------------------------------------------------
int WriteFlashSW_SPI(unsigned char *Data, unsigned int Addr ,unsigned int Len)
{
    int ret,i=0;
    unsigned int Windex=0;
    unsigned char BBuffer[PageSize1K*2]={0};
    unsigned int BAddr = Addr;
    char strBuf[PageSize1K*2],*pstrBuf=strBuf;
    CString strTmp;
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    if(fSPI_ISPMode==false)
        ST1801FlashWakeUp();
    if(fSPI_ISPMode==true){
        if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
            return false;
        }
    }
   
    Sector_Erase_ST1801_SW_ISP(Addr,Len);
    if(fSPI_ISPMode==false){
        if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
            return false;
        }
    }
#endif

#if IC_Module==IC_Module_A8010
    if(fSPIMode)
        Sector_Erase_A8010_SPI_ISP(Addr,Len);
#endif
#if (IC_Module==IC_Module_A8008) || (IC_Module== IC_Module_A8015)
    if(fSPIMode){
        Sector_Erase_A8008_SPI_ISP(Addr,Len);
        if(fSPI_ISPMode==false){
            if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
                return false;
            }
        }
    }

    // A8015 TouchPad 读写 Flash 模拟 A2152 方式
    if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C ) {
        ST1801FlashWakeUp();
		Sector_Erase_ST1801_SW_ISP( Addr, Len );
		if ( SwitchI2C_16Bit_8Bit_ST1801( true ) == false )
			return false;
    }

#endif
    if(fSPI_ISPMode==true){
        if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
            return false;
        }
    }
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;

    unsigned int BAddrMask=0;
    unsigned int BAddrMaskComplete=0;
    int	BankSize=0;

    BAddrMask = 0xFF;
    BAddrMaskComplete = 0xFFFFFF00;
    BankSize = PageSize1K;
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    BankSize = PageSize;
#endif
   
    DWORD dwStart=GetTickCount();
    //#ifdef _DEBUG
    while((Windex < Len) )
        // #else
        //     while((Windex < Len) && ((GetTickCount()-dwStart)<60000))
        // #endif // _DEBUG
    {
        if(Windex==0)
        {
            //===============read back===========     
            // 第一次写入，先将地址之前的信息读出来，将数据拷贝到真正位置，防止覆盖之前的数据
            if((BAddr&BAddrMask) || (((Addr+Len)-BAddr)<BankSize && (Addr+Len)&BAddrMask))
            {
                ret = ReadFlashPage_SW(BAddr&BAddrMaskComplete, BBuffer);

                if(ret == false)
                    return ERRORMSG_ATOM_READ_FLASH_FAIL;
            } 
            for(i=BAddr&BAddrMask ; (i<BankSize) && (Windex<Len) ; i++,Windex++)       
            {
                BBuffer[i] = Data[Windex];
            }
        }
        else
        {
            //===============read back===========
            // 最后一次写入，也需要将地址后面的数据读出来，防止覆盖
            if(((Addr+Len)-(BAddr&BAddrMaskComplete))<BankSize && (Addr+Len)&BAddrMask)         
            {
                ret = ReadFlashPage_SW(BAddr&BAddrMaskComplete, BBuffer);

                if(ret == false)
                {
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
                    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                        return false;
                    }
#endif
                    return ERRORMSG_ATOM_READ_FLASH_FAIL;
                }
            }
            for(i=0 ; (i<BankSize) && (Windex<Len) ; i++,Windex++)           
            {
                BBuffer[i] = Data[Windex];
            }
        }
        DWORD dwWriteStart=GetTickCount();
#if (IC_Module==IC_Module_ST1802)
        if(BAddr<0x1E000)
#else
#endif
        ret = WriteFlashPage_SW(BAddr,BBuffer);    
        if(ret == false)
        {
            SleepInProgram(50);
#if (IC_Module==IC_Module_ST1802)
            if(BAddr<0x1E000)
#else
#endif
            ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer);    
            if(ret == false){
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
                if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                    return false;
                }
#endif
                return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
            }
        }else if (ret == ERRORMSG_CheckSum_Error){
            ret = EraseFlashPage_SW(BAddr&BAddrMaskComplete);
            if(ret == false){
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
                if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                    return false;
                }
#endif
                return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
            }
#if (IC_Module==IC_Module_ST1802)
            if(BAddr<0x1E000)
#else
#endif
            ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer); 
            if(ret==false){
                SleepInProgram(50);
#if (IC_Module==IC_Module_ST1802)
                if(BAddr<0x1E000)
#else
#endif
                ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer); 
                if(ret==false){
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
                    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                        return false;
                    }
#endif
                    return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
                }
            }else if(ret == ERRORMSG_CheckSum_Error){
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
                if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                    return false;
                }
#endif
                return ERRORMSG_CheckSum_Error;
            }
        }
        BAddr+=BankSize;       
    }
#ifdef _DEBUG
    strTmp.Format("%s Write Cost %d ms\r\n",__FUNCTION__,GetTickCount()-dwStart);
    OutputDebugString(strTmp);
#endif // _DEBUG
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    Lock_Flash_ST1801_SW_ISP(false);
#endif
    return true;
}
int WriteFlashSW(unsigned char *Data, unsigned int Addr ,unsigned int Len)
{
    if (g_emCommunicationMode == CommunicationMode::CommunicationMode_Phone_ADB)
    {
        return WriteFlash_ADB(Data, Addr, Len);
    }

#if IC_Module == IC_Module_A8018
    return WriteFlashSW_A8018( Data, Addr, Len );
#endif

    if(fSPIMode){                    
        return WriteFlashSW_SPI(Data,Addr,Len);
    }
    int ret,i=0;
    unsigned int Windex=0;
    unsigned char BBuffer[PageSize1K*2]={0};
    unsigned int BAddr = Addr;
    char strBuf[0x100],*pstrBuf=strBuf;
    CString strTmp;
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
        if ( (Addr & 0xFF) || (Len & 0xFF) )
            return false;

    if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
        return false;
    }
    UnLock_Flash_ST1801_SW_ISP();
    if(Len>=0x1E000){
        Sector_Erase_ST1801_SW_ISP(Addr,0x1E000);
    }else{
        Sector_Erase_ST1801_SW_ISP(Addr,Len);
    }
    if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
        return false;
    }
#endif
#if ( IC_Module == IC_Module_A8015 )
    if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C ) {
		if ( SwitchI2C_16Bit_8Bit_ST1801( true ) == false )
			return false;

		UnLock_Flash_ST1801_SW_ISP();
		Sector_Erase_ST1801_SW_ISP( Addr, Len );
		
		if ( SwitchI2C_16Bit_8Bit_ST1801( true ) == false )
			return false;
    }
#endif
#if IC_Module==IC_Module_A8010
    if(fSPIMode)
        Sector_Erase_A8010_SPI_ISP(Addr,Len);
#endif
#if (IC_Module==IC_Module_A8008) || (IC_Module==IC_Module_A8015)
    if(fSPIMode){
        Sector_Erase_A8008_SPI_ISP(Addr,Len);
    }
#endif
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;

    unsigned int BAddrMask=0;
    unsigned int BAddrMaskComplete=0;
    int	BankSize=0;

    BAddrMask = 0xFF;
    BAddrMaskComplete = 0xFFFFFF00;
    BankSize = PageSize1K;
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    BankSize = PageSize;    
    if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
        return false;
    }
	/*if ( g_emCommunicationMode != CommunicationMode::CommunicationMode_TouchLink_Bulk ) {
		BAddrMaskComplete = 0xFFFFFFFF;
		BankSize = PageSize_HID;
		BAddrMask = 0xFFFFFFFF;
	}*/
    UnLock_Flash_ST1801_SW_ISP();
#endif
    DWORD dwStart=GetTickCount();
    g_pro = 0;
    while((Windex < Len) )
    {
		if ( Windex == 0 )
		{
			//===============read back===========         
			if ( (BAddr & BAddrMask) || (((Addr + Len) - BAddr) < BankSize && (Addr + Len) & BAddrMask) )
			{
				ret = ReadFlashPage_SW( BAddr & BAddrMaskComplete, BBuffer );

				if ( ret == false )
					return ERRORMSG_ATOM_READ_FLASH_FAIL;
			}
			for ( i = BAddr & BAddrMask; (i < BankSize) && (Windex < Len); i++, Windex++ )
			{
				BBuffer[i] = Data[Windex];
			}
		} else
		{
			//===============read back===========
			if ( ((Addr + Len) - (BAddr & BAddrMaskComplete)) < BankSize && (Addr + Len) & BAddrMask )
			{
				ret = ReadFlashPage_SW( BAddr & BAddrMaskComplete, BBuffer );
				if ( ret == false )
				{
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
					if ( SwitchI2C_16Bit_8Bit_ST1801( false ) == false ) {
						return false;
					}
#endif
					return ERRORMSG_ATOM_READ_FLASH_FAIL;
				}
			}
			for ( i = 0; (i < BankSize) && (Windex < Len); i++, Windex++ )
			{
				BBuffer[i] = Data[Windex];
			}
		}

        ret = WriteFlashPage_SW(BAddr,BBuffer);    

        if(ret == false)
        {
            SleepInProgram(50);
            ret = WriteFlashPage_SW(BAddr,BBuffer);    
            if(ret == false){
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
                if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                    return false;
                }
#endif
#if ( IC_Module == IC_Module_A8015 )
                if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
                    if ( !SwitchI2C_16Bit_8Bit_ST1801( false ) )
                        return -1;
#endif
                return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
            }
        }else if (ret == ERRORMSG_CheckSum_Error){
            ret = EraseFlashPage_SW(BAddr&BAddrMaskComplete);
            if(ret == false){
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
                if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                    return false;
                }
#endif
#if ( IC_Module == IC_Module_A8015 )
				if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
					if ( !SwitchI2C_16Bit_8Bit_ST1801( false ) )
						return -1;
#endif
                return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
            }
            ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer); 
            if(ret==false){
                SleepInProgram(50);
                ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer); 
                if(ret==false){
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
                    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                        return false;
                    }
#endif
#if ( IC_Module == IC_Module_A8015 )
					if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
						if ( !SwitchI2C_16Bit_8Bit_ST1801( false ) )
							return -1;
#endif
                    return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
                }
            }else if(ret == ERRORMSG_CheckSum_Error){
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
                if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                    return false;
                }
#endif
#if ( IC_Module == IC_Module_A8015 )
				if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
					if ( !SwitchI2C_16Bit_8Bit_ST1801( false ) )
						return -1;
#endif
                return ERRORMSG_CheckSum_Error;
            }
        }
        BAddr+=BankSize;       
        g_pro = (int)((double)Windex / (double)Len * 100.0);
    }
#ifdef _DEBUG
    strTmp.Format("%s Write Cost %d ms\r\n",__FUNCTION__,GetTickCount()-dwStart);
    OutputDebugString(strTmp);
#endif // _DEBUG
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    //     if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
    //         return false;
    //     }
    Lock_Flash_ST1801_SW_ISP(false);
#endif
#if ( IC_Module == IC_Module_A8015 )
	if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
        Lock_Flash_ST1801_SW_ISP( false );
#endif
    return true;

}
int WriteFlashSW_SPI_ST1801_MassErase(unsigned char *Data, unsigned int Addr ,unsigned int Len)
{
    int ret,i=0;
    unsigned int Windex=0;
    unsigned char BBuffer[PageSize1K]={0};
    unsigned int BAddr = Addr;
    char strBuf[0x100],*pstrBuf=strBuf;
    if(fSPI_ISPMode==false)
        ST1801FlashWakeUp();
    if(fSPI_ISPMode==true){
        if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
            return false;
        }
        UnLock_Flash_ST1801_SW_ISP(true);
    }
    //Chip_Erase_ST1801_SW_ISP();
    EraseFlashSW(0,0xC000);
    Sleep(50);
    if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
        return false;
    }
    /*
    UnLock_Flash_ST1801_SW_ISP(true);
    if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
    return false;
    }
    */
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;

    unsigned int BAddrMask=0;
    unsigned int BAddrMaskComplete=0;
    int	BankSize=0;
 CString strTmp;
    BAddrMask = 0xFF;
    BAddrMaskComplete = 0xFFFFFF00;
    BankSize = PageSize;
    DWORD dwStart = GetTickCount();
    while(Windex < Len)
    {
        if(Windex==0)
        {

            for(i=BAddr&BAddrMask ; (i<BankSize) && (Windex<Len) ; i++,Windex++)       
            {
                BBuffer[i] = Data[Windex];
            }
        }
        else
        {
            for(i=0 ; (i<BankSize) && (Windex<Len) ; i++,Windex++)           
            {
                BBuffer[i] = Data[Windex];
            }
        }
        ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer);    

        if(ret == false)
        {
            SleepInProgram(50);
            ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer);    
            if(ret == false){
                if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                    return false;
                }
                return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
            }
        }else if (ret == ERRORMSG_CheckSum_Error){
            ret = EraseFlashPage_SW(BAddr&BAddrMaskComplete);
            if(ret == false){
                if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                    return false;
                }
                return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
            }
            ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer); 
            if(ret==false){
                SleepInProgram(50);
                ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer); 
                if(ret==false){
                    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                        return false;
                    }
                    return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
                }
            }else if(ret == ERRORMSG_CheckSum_Error){
                if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
                    return false;
                }
                return ERRORMSG_CheckSum_Error;
            }
        }
        BAddr+=BankSize;       
    }
#ifdef _DEBUG
   
    strTmp.Format("%s Wrtite Cost %d ms\r\n",__FUNCTION__,GetTickCount()-dwStart);
    OutputDebugString(strTmp);
#endif // _DEBUG
    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
        return false;
    }
    Lock_Flash_ST1801_SW_ISP(false);
    return true;

}
//------------------------------------------------
int ReadUsb(unsigned char *data, unsigned int ilen, unsigned int &rtlen)
{
    int rlen;
    rlen = USBComm_ReadFromBulkEx(data,ilen);
    rtlen = rlen;
    if(rlen<0)
    {
        rtlen = 0;
        return ERRORMSG_READ_BLUK_FAIL;
    }
    if(rlen!=ilen)
    {
        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }
    return true;
}

//------------------------------------------------
int WriteUsb(unsigned char *data, unsigned int ilen, unsigned int &rtlen)
{

    int rlen;
    rlen = USBComm_WriteToBulkEx(data,ilen);
    rtlen = rlen;
    if(rlen<0)
    {
        rtlen = 0;
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    if(rlen!=ilen)
    {
        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }    
    return true;
}

//------------------------------------------------------------------
//|0x7E|0x04|0x00|INT MODE|Address|Length|other|
//------------------------------------------------------------------
int ReadI2CRegBeginINT(unsigned char Addr,unsigned char length)
{
    unsigned char IBuffer[BulkLen]={0};    
    int ret=1;
    BOOL rt;
    int verTL=0;
    int verHW=0;
    UCHAR   buf[8]={0};
    int     *ptr = NULL;
    rt = GetI2CStatus_Bulk(buf);
    fUseNewINTProtocol=false;
    if(rt==true){
        verTL = (buf[2]&0x1F)*10+buf[3];      
        verHW = (buf[2]>>5);  
        if( (verHW==02 && verTL>=36) ||(verHW==01 && verTL>=36) ||verHW>=03 ){
            // fUseNewINTProtocol=true;
        }
    }
    IBuffer[0] = Bridge_T_Mode_SEL;
    if(fUseNewINTProtocol){     
        IBuffer[1] = 6; //Length L
        IBuffer[2] = 0; //Length H    
        IBuffer[3] = 5; //INT mode 2   
        IBuffer[4] = 0x01; 
        IBuffer[5] = Addr; 
        IBuffer[6] = length;   //length L
        IBuffer[7] = 0x00;   //length H
        IBuffer[8] = 0x01;  //100 ms == 0x01, 1Sec = 0x0A
    }else{
        IBuffer[1] = 4; //Length L
        IBuffer[2] = 0; //Length H    
        IBuffer[3] = 1; //INT mode    
        IBuffer[4] = Addr; 
        IBuffer[5] = length; 
    }

    ret = USBComm_WriteToBulkEx(IBuffer, BulkLen);
    if(ret<0)
    {
        //ShowMessage("USB_Write_Fail");
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    if(fUseNewINTProtocol){     
        ret = 2;
    }else{
        ret = 1;
    }   
    return ret;
}

int   ReadI2CRegDataINT(unsigned char *Data,unsigned char length)
{
    return Read_Packet_INT_Bulk(Data,length);    
}


//------------------------------------------------------------------
//|0x7E|0x04|0x00|Normal MODE|Address|Length|other|
//------------------------------------------------------------------
int ReadI2CRegEndINT(void)
{
    unsigned char IBuffer[BulkLen]={0};    
    int ret;   
    IBuffer[0] = Bridge_T_Mode_SEL;
    IBuffer[1] = 4; //Length L
    IBuffer[2] = 0; //Length H    
    IBuffer[3] = 0; //Normal mode    
    IBuffer[4] = 0; 
    IBuffer[5] = 0; 

    ret = USBComm_WriteToBulkEx(IBuffer, BulkLen);
    if(ret<0)
    {
        //ShowMessage("USB_Write_Fail");
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    return 1;
}


//------------------------------------------------------------------
//|0x60|
//------------------------------------------------------------------
int GetI2CStatus_Bulk(unsigned char *SBuff)
{
    unsigned char GBuffer[BulkLen]={0};
    unsigned char SBuffer[BulkLen]={0};
    int ret;
    GBuffer[0] = Bridge_T_Get_Status;
    GBuffer[1] = 0x0;
    GBuffer[2] = 0x0;

    ret = USBComm_WriteToBulkEx(GBuffer, BulkLen);
    if(ret<0)
    {
        //ShowMessage("USB_Write_Fail");
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    SleepInProgram(10);

    ret = USBComm_ReadFromBulkEx(SBuffer,BulkLen);
    if(ret<0)
    {
        //ShowMessage("Read Stauts timeout");
        return ERRORMSG_READ_BLUK_FAIL;
    }

    if(SBuffer[0]!=0xF0)
    {
        return ERRORMSG_READ_PACKET_ERROR;
    }
    /*
    Bridge_Status_Struct.CmdId=SBuffer[0];
    Bridge_Status_Struct.Len[0]=SBuffer[1];
    Bridge_Status_Struct.Len[1]=SBuffer[2];
    Bridge_Status_Struct.Bridge_Status=SBuffer[3];
    Bridge_Status_Struct.I2C_Interface_Status=SBuffer[4];
    Bridge_Status_Struct.Touch_link_version[0]=SBuffer[5];
    Bridge_Status_Struct.Touch_link_version[1]=SBuffer[6];
    Bridge_Status_Struct.Error_Code=SBuffer[7];
    Bridge_Status_Struct.ICP_Interface_Switch=SBuffer[10];
    */
    for(int i=0 ; i<SBuffer[1] ; i++)
    {
        SBuff[i]=SBuffer[4+i];
    }
    int nTouchLinkSWVersion = (SBuff[2]&0x1F)*100+SBuff[3];      
    int nTouchLinkHWVersion =(SBuff[2]>>5);       
    return 1;
}

int GetTouchLinkVersion( TouchLinkVersion * ver )
{
	unsigned char cmd_0x5F_GetBridgeStatus = 0x5F;
    if ( USBComm_WriteToBulkEx( &cmd_0x5F_GetBridgeStatus, BulkLen ) <= 0 )
        return ERRORMSG_WRITE_BLUK_FAIL;

	unsigned char tmp_ReadBuf[BulkLen]{ 0 };
    if ( USBComm_ReadFromBulkEx( tmp_ReadBuf, BulkLen ) <= 0 )
        return ERRORMSG_READ_BLUK_FAIL;

	if ( 0xF1 != tmp_ReadBuf[0] )
		return ERRORMSG_READ_PACKET_ERROR;

    ver->hw = (tmp_ReadBuf[4 + 2] >> 5);
    ver->sw_major = tmp_ReadBuf[4 + 2] & 0x1F;
    ver->sw_sub = tmp_ReadBuf[4 + 3];

	// if support 1.2V is TL6a
    ver->hw_sub = 0;
	if ( ver->hw >= 6 ) {
        // over touch link 6a use new protocol get hw sub version
		if ( ((6 == ver->hw) && (ver->sw_major >= 2))   // touch link 6b fw version base on 2.x
             || (ver->hw > 6) )
            ver->hw_sub = tmp_ReadBuf[4 + 27];
		else
            ver->hw_sub = tmp_ReadBuf[4 + 26];          // touch link 6a use old protocol
	}

	return 1;
}

//------------------------------------------------------------------
//T:|0x6F|0x03|0x00|0x03|LED R/W|LED ON/OFF|
//------------------------------------------------------------------
int SetLED_Bulk(unsigned char LEDSetting,BOOL fFlashMode,BYTE pPeriod)
{
    unsigned char TBuffer[BulkLen]={0};
    int ret;
    TBuffer[0] = Bridge_T_System_OP;
    if(fFlashMode==false){
        TBuffer[1] = 0x03;
        TBuffer[2] = 0x00;
        TBuffer[3] = 0x03; //LED Status Control   
        TBuffer[4] = 0x00; //write   Data Buffer[1]
        TBuffer[5] = LEDSetting;  //Data Buffer[2]
        TBuffer[6] = pPeriod;  //Data Buffer[3]
    }else{
        if(pPeriod==0){
            TBuffer[1] = 0x02;
            TBuffer[2] = 0x00;
            TBuffer[3] = 0x06; //LED Status Control   
            TBuffer[4] = 0x00; //NormalMode   Data Buffer[1]
        }else{
            TBuffer[1] = 0x04;
            TBuffer[2] = 0x00;
            TBuffer[3] = 0x06; //LED Status Control   
            TBuffer[4] = 0x01; //FlashMode   Data Buffer[1]
            TBuffer[5] = LEDSetting;  //Data Buffer[2]
            TBuffer[6] = pPeriod;  //Data Buffer[3]
        }
    }
    ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
    if(ret<0)
    {        
        return ERRORMSG_WRITE_BLUK_FAIL;
    }    
    return 1;
}

//------------------------------------------------------------------
//T:|0x6F|0x02|0x00|0x03|LED R/W|
//R:|0x8F|0x01|0x00|0x03|LED ON/OFF|
//------------------------------------------------------------------
int GetLED_Bulk(unsigned char *LEDSetting)
{
    unsigned char TBuffer[BulkLen]={0};
    unsigned char RBuffer[BulkLen]={0};
    int ret,timeout=100;

    TBuffer[0] = Bridge_T_System_OP;
    TBuffer[1] = 0x02;
    TBuffer[2] = 0x00; 
    TBuffer[3] = 0x03; //LED Status Control
    TBuffer[4] = 0x80; //read

    ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
    if(ret<0)
    {        
        return ERRORMSG_WRITE_BLUK_FAIL;
    }

    SleepInProgram(5);

    while(timeout--)
    {
        ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
        if(ret>0)
        {
            if(RBuffer[0x00]==0x8F && RBuffer[0x03]==0x03)
            {
                *LEDSetting = RBuffer[0x04];
                break;
            }
        }
    }    

    if(timeout==0)
        return ERRORMSG_TIMEOUT;

    return 1;
}

//----------------------------------------------------------------------------------
//T:|0x6F|0x0A|0x00|0x02|RegWrite|ADDR0|ADDR1|ADDR2|ADDR3|Data0|Data1|Data2|Data3|
//----------------------------------------------------------------------------------
int  WriteRegTL_Bulk(unsigned int Addr, unsigned int Data)
{
    unsigned char TBuffer[BulkLen]={0};
    int ret;

    TBuffer[0] = Bridge_T_System_OP;
    TBuffer[1] = 0x0A;
    TBuffer[2] = 0x00;
    TBuffer[3] = 0x02; //Register Control
    TBuffer[4] = 0x00; //write
    TBuffer[5] = Addr&0xFF;
    TBuffer[6] = (Addr>>8)&0xFF;
    TBuffer[7] = (Addr>>16)&0xFF;
    TBuffer[8] = (Addr>>24)&0xFF;
    TBuffer[9] = Data&0xFF;
    TBuffer[10] = (Data>>8)&0xFF;
    TBuffer[11] = (Data>>16)&0xFF;
    TBuffer[12] = (Data>>24)&0xFF;

    ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
    if(ret<0)
    {        
        return ERRORMSG_WRITE_BLUK_FAIL;
    }    
    return 1;
}

//----------------------------------------------------------------------------------
//T:|0x6F|0x06|0x00|0x02|RegWrite|ADDR0|ADDR1|ADDR2|ADDR3|
//R:|0x8F|0x04|0x00|0x02|Data0|Data1|Data2|Data3|
//----------------------------------------------------------------------------------
int  ReadRegTL_Bulk(unsigned int Addr, unsigned int *Data)
{
    unsigned char TBuffer[BulkLen]={0};
    unsigned char RBuffer[BulkLen]={0};
    int ret,timeout=10;

    TBuffer[0] = Bridge_T_System_OP;
    TBuffer[1] = 0x06;
    TBuffer[2] = 0x00;
    TBuffer[3] = 0x02; //Register Control
    TBuffer[4] = 0x80; //Read
    TBuffer[5] = Addr&0xFF;
    TBuffer[6] = (Addr>>8)&0xFF;
    TBuffer[7] = (Addr>>16)&0xFF;
    TBuffer[8] = (Addr>>24)&0xFF;

    ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
    if(ret<0)
    {        
        return ERRORMSG_WRITE_BLUK_FAIL;
    }    
    SleepInProgram(10);
    while(timeout--)
    {
        ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
        if(ret>0)
        {
            if(RBuffer[0x00]==0x8F && RBuffer[0x03]==0x02)
            {
                *Data = RBuffer[0x04];
                *Data += RBuffer[0x05]*0x100;
                *Data += RBuffer[0x06]*0x10000;
                *Data += RBuffer[0x07]*0x1000000;
                break;
            }
        }
    }    
    if(timeout==0)
        return ERRORMSG_TIMEOUT;

    return 1;
}


//------------------------------------------------------------------
//T:|0x6F|0x03|0x00|0x04|Button W|Button ON/OFF|
//------------------------------------------------------------------
int SetButton_Bulk(unsigned char ButtonSetting)
{
    unsigned char TBuffer[BulkLen]={0};
    int ret;

    TBuffer[0] = Bridge_T_System_OP;
    TBuffer[1] = 0x03;
    TBuffer[2] = 0x00;
    TBuffer[3] = 0x04; //Button Status Control
    TBuffer[4] = 0x00; //write
    TBuffer[5] = ButtonSetting;

    ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
    if(ret<0)
    {        
        return ERRORMSG_WRITE_BLUK_FAIL;
    }    
    return 1;
}


//------------------------------------------------------------------
//T:|0x6F|0x02|0x00|0x04|Button R|
//R:|0x8F|0x01|0x00|0x04|Button Flag|Button78 value|
//------------------------------------------------------------------
int GetButton_Bulk(unsigned char *ButtonSetting)
{
    unsigned char TBuffer[BulkLen]={0};
    unsigned char RBuffer[BulkLen]={0};
    int ret,timeout=100;

    TBuffer[0] = Bridge_T_System_OP;
    TBuffer[1] = 0x02;
    TBuffer[2] = 0x00;
    TBuffer[3] = 0x04; //Button Status Control
    TBuffer[4] = 0x80; //read

    ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
    if(ret<0)
    {        
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    while(timeout--)
    {
        ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
        if(ret>0)
        {
            if(RBuffer[0x00]==0x8F && RBuffer[0x03]==0x04)
            {
                *ButtonSetting = RBuffer[0x04];        //Button Flag
                *ButtonSetting += (RBuffer[0x05]<<4);  //Button78 value
                break;
            }
        }
    }    
    if(timeout==0)
        return ERRORMSG_TIMEOUT;

    return 1;
}

//Define Table
#define	V18_Tolerance		10		// 10% = 1/10
#define	V28_Tolerance		20		// 5% = 1/20
#define	V33_Tolerance		20		// 5% = 1/20
#define	USB_Tolerance		10		// 10% = 1/10

typedef enum {
    VDD_5V = 0,
    VDD_3V3 = 1,
    VDD_2V8 = 2,
    VDD_1V8 = 3,
    VDD_4V3 = 4
} E_VDD;

union uDouble{
    double value;
    unsigned char array[8];
};
double ReadTPCurrent_TL3()
{
    unsigned char szReadBuf[0xFF] = {0};
    int ret = GetI2CStatus_Bulk( szReadBuf );
    if ( ret <= 0 ) return ret;

    int nTouchLinkHW = (szReadBuf[2]>>5);
    if ( nTouchLinkHW >= 5 ) {
        OutputDebugString("Touch Link 5\n");
        unsigned char szBuffer[BulkLen] = { 0x00 };
        szBuffer[0] = 0x69;     // command
        szBuffer[1] = 0x04;     // length low
        szBuffer[2] = 0x00;     // length hight
        szBuffer[3] = 0x58;     // read cnt low
        szBuffer[4] = 0x02;     // read cnt hight
        szBuffer[5] = 0x01;     // period time low
        szBuffer[6] = 0x00;     // period time hight

        ret = USBComm_WriteToBulkEx( szBuffer, BulkLen );
        if ( ret != BulkLen ) return ERRORMSG_WRITE_BLUK_FAIL;

        ret = USBComm_ReadFromBulkEx( szBuffer, BulkLen );
        if ( ret != BulkLen ) return ERRORMSG_READ_BLUK_FAIL;

        if ( Bridge_R_Current != szBuffer[0] && Bridge_R_Status != szBuffer[0] ) 
            return ERRORMSG_READ_PACKET_ERROR;

        if ( Bridge_R_Status == szBuffer[0] ) {
            if ( Bridge_Status_Error == szBuffer[4] && Error_VddGndShort == szBuffer[8] )
                return ERRORMSG_BRIDGE_STATUS_ERROR_VDDGND_Short;
            return ERRORMSG_BRIDGE_STATUS_ERROR;
        }

        if ( 0xFF == szBuffer[4] ) return ERRORMSG_GetTP_Trim_Error;
        if ( 0x01 != szBuffer[4] ) return ERRORMSG_BRIDGE_STATUS_BUS_FAULT;
           
        // unit: mA
        double dCurrent = szBuffer[5]|(szBuffer[6]<<8)|(szBuffer[7]<<16)|(szBuffer[8]<<24);
        return (dCurrent/1000);
    }

    int timeout;
    unsigned char WBuffer[BulkLen], RBuffer[BulkLen], DataBuf[512]={0};
    unsigned char ResponseID, ResponseParam;
    int Loop=3000;
    unsigned short ResponseLength, rLength=512, rBufIndex;
    unsigned short ADCZener2v4, ADCVDD1, ADCVDD2, ADCLdoVin;
    unsigned int u32Standard4V3, u32Standard3V3, u32Standard2V8, u32Standard1V8;
    unsigned int u32Tolerance4V3, u32Tolerance3V3, u32Tolerance2V8, u32Tolerance1V8;
    unsigned int Address = 0x0001F000, cnt;
    double fVBus, fLdoVin, fAccDelta=0, fAccCurrent=0;
    E_VDD Vdd1, Vdd2;
    uDouble uOffset, uSlope;
    memset(WBuffer, 0, sizeof(WBuffer));

    WBuffer[0] = 0x13;							//Command
    WBuffer[1] = 0x07;							//Length
    WBuffer[2] = 0x00;
    WBuffer[3] = 0x02;							//Read
    WBuffer[4] = (unsigned char)(Address);
    WBuffer[5] = (unsigned char)(Address>>8);
    WBuffer[6] = (unsigned char)(Address>>16);
    WBuffer[7] = (unsigned char)(Address>>24);
    WBuffer[8] = (unsigned char)(rLength);
    WBuffer[9] = (unsigned char)(rLength>>8);

    ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
    if(ret<0)    {
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    if(ret!=BulkLen)    {
        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }
    timeout = 10;

    while(timeout--)
    {
        ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
        if(ret>0)
        {
            ResponseID = RBuffer[0];
            ResponseLength = RBuffer[1] + (RBuffer[2]<<8);
            ResponseParam = RBuffer[3];
            for (cnt=0;cnt<60;cnt++)
                DataBuf[cnt] = RBuffer[cnt+4];

            if (ResponseID!=0x81) {
                return -30;
            }
            if (ResponseLength!=rLength) {
                return -31;
            }
            if (ResponseParam!=3) {
                return -32;
            }
            break;
        }
    }
    if(timeout==0) {
        return ERRORMSG_TIMEOUT;
    }

    if (ResponseLength>60)     {
        rBufIndex = 60;
        while(rBufIndex<(rLength-63)) {
            //Continue Packet
            timeout = 10;
            while(timeout--)
            {
                ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
                if(ret>0)
                {
                    ResponseID = RBuffer[0];
                    if (ResponseID!=0xFF) {
                        return -33;
                    }

                    for (cnt=0;cnt<63;cnt++)
                        DataBuf[cnt+rBufIndex] = RBuffer[cnt+1];

                    break;
                }
            }
            if(timeout==0)
                return ERRORMSG_TIMEOUT;

            rBufIndex += 63;
        }
        //Last Packet
        timeout = 10;
        while(timeout--)
        {
            ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
            if(ret>0)
            {
                ResponseID = RBuffer[0];
                if (ResponseID!=0xFF) {
                    return -34;
                }

                for (cnt=0;cnt<(unsigned int)(rLength-rBufIndex);cnt++)
                    DataBuf[cnt+rBufIndex] = RBuffer[cnt+1];

                break;
            } 
        }
        if(timeout==0)
            return ERRORMSG_TIMEOUT;
    }
    if((DataBuf[0]!='T')||(DataBuf[1]!='R')||(DataBuf[2]!='I'))
    {  //腹癸ａ柑⊿ΤTRIM
        return ERRORMSG_GetTP_Trim_Error;
    }
    //Read Touch Link ADC
    if(DataBuf[3]==0x01){
        Loop = 3000;
        for (cnt=0;cnt<Loop;cnt++)
        {
            WBuffer[0] = Bridge_T_Get_Status2;
            WBuffer[1] = 0; //Length L
            WBuffer[2] = 0; //Length H
            ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
            if(ret<0)
            {
                ////ShowMessage("USB_Write_Fail");
                return ERRORMSG_WRITE_BLUK_FAIL;
            }
            if(ret!=BulkLen)
            {
                ////ShowMessage("Non-complete");
                return ERRORMSG_NON_COMPLETE_TRANSFER;
            }

            timeout = 10;
            while(timeout--)
            {
                ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
                if(ret>0)
                {
                    ResponseID = RBuffer[0];
                    ResponseLength = RBuffer[1] + (RBuffer[2]<<8);
                    ResponseParam = RBuffer[3];
                    ADCZener2v4 =	RBuffer[9] + (RBuffer[10]<<8);
                    ADCVDD1 =		RBuffer[11] + (RBuffer[12]<<8);
                    ADCVDD2 =		RBuffer[13] + (RBuffer[14]<<8);
                    ADCLdoVin = 	RBuffer[17] + (RBuffer[18]<<8);
                    if (ResponseID!=0xF1) {
                        return -35;
                    }

                    if (ResponseLength!=23) {
                        //return FALSE;
                    }

                    if (ResponseParam!=0) {
                        return -36;
                    }
                    fVBus = (2.4*4095/ADCZener2v4);
                    fLdoVin = (2.4*ADCLdoVin/ADCZener2v4);
                    fAccDelta += (fVBus-fLdoVin);
                    break;
                }
            }    

            if(timeout==0)
                return ERRORMSG_TIMEOUT;
        }
        //Verify VDD1 & VDD2 Voltage
        u32Standard4V3 = ADCZener2v4 * 43 / 24;
        u32Standard3V3 = ADCZener2v4 * 33 / 24;
        u32Standard2V8 = ADCZener2v4 * 28 / 24;
        u32Standard1V8 = ADCZener2v4 * 18 / 24;
        u32Tolerance4V3 = u32Standard4V3 / 20;
        u32Tolerance3V3 = u32Standard3V3 / 20;
        u32Tolerance2V8 = u32Standard2V8 / 20;
        u32Tolerance1V8 = u32Standard1V8 / 20;

        //VDD1
        if ((ADCVDD1 < (u32Standard4V3 + u32Tolerance4V3)) && 
            (ADCVDD1 > (u32Standard4V3 - u32Tolerance4V3))) {
                Vdd1 = VDD_4V3;
        }
        else if ((ADCVDD1 < (u32Standard3V3 + u32Tolerance3V3)) && 
            (ADCVDD1 > (u32Standard3V3 - u32Tolerance3V3))) {
                Vdd1 = VDD_3V3;
        }
        else if ((ADCVDD1 < (u32Standard2V8 + u32Tolerance2V8)) && 
            (ADCVDD1 > (u32Standard2V8 - u32Tolerance2V8))) {
                Vdd1 = VDD_2V8;
        }
        else if ((ADCVDD1 < (u32Standard1V8 + u32Tolerance1V8)) && 
            (ADCVDD1 > (u32Standard1V8 - u32Tolerance1V8))) {
                Vdd1 = VDD_1V8;
        }
        else {
            return -37;
        }
        //VDD2
        if ((ADCVDD2 < (u32Standard4V3 + u32Tolerance4V3)) && 
            (ADCVDD2 > (u32Standard4V3 - u32Tolerance4V3))) {
                Vdd2 = VDD_4V3;
        }	
        else if ((ADCVDD2 < (u32Standard3V3 + u32Tolerance3V3)) && 
            (ADCVDD2 > (u32Standard3V3 - u32Tolerance3V3))) {
                Vdd2 = VDD_3V3;
        }	
        else if ((ADCVDD2 < (u32Standard2V8 + u32Tolerance2V8)) && 
            (ADCVDD2 > (u32Standard2V8 - u32Tolerance2V8))) {
                Vdd2 = VDD_2V8;
        }
        else if ((ADCVDD2 < (u32Standard1V8 + u32Tolerance1V8)) && 
            (ADCVDD2 > (u32Standard1V8 - u32Tolerance1V8))) {
                Vdd2 = VDD_1V8;
        }
        else {
            //Not in specified range
            return -38;
        }
        //Get Touch Link Offset
        if ((Vdd1==VDD_4V3)&&(Vdd2==VDD_4V3)) {
            uOffset.array[0] = DataBuf[4];
            uOffset.array[1] = DataBuf[5];
            uOffset.array[2] = DataBuf[6];
            uOffset.array[3] = DataBuf[7];
            uOffset.array[4] = DataBuf[8];
            uOffset.array[5] = DataBuf[9];
            uOffset.array[6] = DataBuf[10];
            uOffset.array[7] = DataBuf[11];
        } else if ((Vdd1==VDD_4V3)&&(Vdd2==VDD_3V3)) {
            uOffset.array[0] = DataBuf[12];
            uOffset.array[1] = DataBuf[13];
            uOffset.array[2] = DataBuf[14];
            uOffset.array[3] = DataBuf[15];
            uOffset.array[4] = DataBuf[16];
            uOffset.array[5] = DataBuf[17];
            uOffset.array[6] = DataBuf[18];
            uOffset.array[7] = DataBuf[19];
        } else if ((Vdd1==VDD_4V3)&&(Vdd2==VDD_2V8)) {
            uOffset.array[0] = DataBuf[20];
            uOffset.array[1] = DataBuf[21];
            uOffset.array[2] = DataBuf[22];
            uOffset.array[3] = DataBuf[23];
            uOffset.array[4] = DataBuf[24];
            uOffset.array[5] = DataBuf[25];
            uOffset.array[6] = DataBuf[26];
            uOffset.array[7] = DataBuf[27];
        } else if ((Vdd1==VDD_4V3)&&(Vdd2==VDD_1V8)) {
            uOffset.array[0] = DataBuf[28];
            uOffset.array[1] = DataBuf[29];
            uOffset.array[2] = DataBuf[30];
            uOffset.array[3] = DataBuf[31];
            uOffset.array[4] = DataBuf[32];
            uOffset.array[5] = DataBuf[33];
            uOffset.array[6] = DataBuf[34];
            uOffset.array[7] = DataBuf[35];
        } else if ((Vdd1==VDD_3V3)&&(Vdd2==VDD_3V3)) {
            uOffset.array[0] = DataBuf[36];
            uOffset.array[1] = DataBuf[37];
            uOffset.array[2] = DataBuf[38];
            uOffset.array[3] = DataBuf[39];
            uOffset.array[4] = DataBuf[40];
            uOffset.array[5] = DataBuf[41];
            uOffset.array[6] = DataBuf[42];
            uOffset.array[7] = DataBuf[43];
        } else if ((Vdd1==VDD_3V3)&&(Vdd2==VDD_2V8)) {
            uOffset.array[0] = DataBuf[44];
            uOffset.array[1] = DataBuf[45];
            uOffset.array[2] = DataBuf[46];
            uOffset.array[3] = DataBuf[47];
            uOffset.array[4] = DataBuf[48];
            uOffset.array[5] = DataBuf[49];
            uOffset.array[6] = DataBuf[50];
            uOffset.array[7] = DataBuf[51];
        } else if ((Vdd1==VDD_3V3)&&(Vdd2==VDD_1V8)) {
            uOffset.array[0] = DataBuf[52];
            uOffset.array[1] = DataBuf[53];
            uOffset.array[2] = DataBuf[54];
            uOffset.array[3] = DataBuf[55];
            uOffset.array[4] = DataBuf[56];
            uOffset.array[5] = DataBuf[57];
            uOffset.array[6] = DataBuf[58];
            uOffset.array[7] = DataBuf[59];
        } else if ((Vdd1==VDD_2V8)&&(Vdd2==VDD_2V8)) {
            uOffset.array[0] = DataBuf[60];
            uOffset.array[1] = DataBuf[61];
            uOffset.array[2] = DataBuf[62];
            uOffset.array[3] = DataBuf[63];
            uOffset.array[4] = DataBuf[64];
            uOffset.array[5] = DataBuf[65];
            uOffset.array[6] = DataBuf[66];
            uOffset.array[7] = DataBuf[67];
        } else if ((Vdd1==VDD_2V8)&&(Vdd2==VDD_1V8)) {
            uOffset.array[0] = DataBuf[68];
            uOffset.array[1] = DataBuf[69];
            uOffset.array[2] = DataBuf[70];
            uOffset.array[3] = DataBuf[71];
            uOffset.array[4] = DataBuf[72];
            uOffset.array[5] = DataBuf[73];
            uOffset.array[6] = DataBuf[74];
            uOffset.array[7] = DataBuf[75];
        } else if ((Vdd1==VDD_1V8)&&(Vdd2==VDD_1V8)) {
            uOffset.array[0] = DataBuf[76];
            uOffset.array[1] = DataBuf[77];
            uOffset.array[2] = DataBuf[78];
            uOffset.array[3] = DataBuf[79];
            uOffset.array[4] = DataBuf[80];
            uOffset.array[5] = DataBuf[81];
            uOffset.array[6] = DataBuf[82];
            uOffset.array[7] = DataBuf[83];
        } else {
            return -39;
        }
        //Get Touch Link Slope
        uSlope.array[0] = DataBuf[84];
        uSlope.array[1] = DataBuf[85];
        uSlope.array[2] = DataBuf[86];
        uSlope.array[3] = DataBuf[87];
        uSlope.array[4] = DataBuf[88];
        uSlope.array[5] = DataBuf[89];
        uSlope.array[6] = DataBuf[90];
        uSlope.array[7] = DataBuf[91];
        //return TP Current (mA)
        return (((fAccDelta/Loop)/6.2*1000)-uOffset.value)*uSlope.value;
    }else{
        return ERRORMSG_GetTP_Trim_Error;
    }
}

double GetAVGFromEliminateMethod(double * dBufferArray, int nTotalCount, int nThPercent)
{
    int minValidCount = nTotalCount * nThPercent / 100.0;
    if ( ( nTotalCount * nThPercent ) % 100 >= 50 ) minValidCount++;

    double dThStep = (dBufferArray[nTotalCount - 1] - dBufferArray[0]) / 100;
    double dCheckRange = dThStep;
    int nValidPos = 0, nValidNum = 0, nLastValidNum = 0;
    do {      
        for ( int startPos = 0; startPos <= nTotalCount - minValidCount; ++startPos )
        {
            double dThValue = dBufferArray[startPos] + dCheckRange;
            int checkPos = startPos;
            do {
                if ( checkPos >= nTotalCount || dBufferArray[checkPos] > dThValue ) 
                    break ;
                checkPos++;
            } while (1);

            int checkValidNum = checkPos - startPos;
            if ( checkValidNum >= minValidCount && nLastValidNum <= checkValidNum ) {
                if ( nLastValidNum == checkValidNum ) {
                    nValidNum++;
                } else {
                    nValidPos = startPos;
                    nLastValidNum = nValidNum = checkValidNum;
                }
            }
        }

        if ( nValidNum > 0 ) break ; 
        dCheckRange += dThStep;  
    } while (1);

    double dSum = 0.0;
    for ( int i = nValidPos; i < nValidPos + nValidNum; ++i )
        dSum += dBufferArray[i];

    return dSum/nValidNum;
}

double ReadTPCurrentAVG()
{
    int ret, timeout;
    unsigned char WBuffer[BulkLen], RBuffer[BulkLen], DataBuf[512]={0};
    unsigned char ResponseID, ResponseParam;
    const int Loop=3000;
    unsigned short ResponseLength, rLength=512, rBufIndex;
    unsigned short ADCZener2v4, ADCVDD1, ADCVDD2, ADCLdoVin;
    unsigned int u32Standard4V3, u32Standard3V3, u32Standard2V8, u32Standard1V8;
    unsigned int u32Tolerance4V3, u32Tolerance3V3, u32Tolerance2V8, u32Tolerance1V8;
    unsigned int Address = 0x0001F000, cnt;
    double fVBus, fLdoVin, fAccDelta=0, fAccCurrent=0;
    E_VDD Vdd1, Vdd2;
    uDouble uOffset, uSlope;
    memset(WBuffer, 0, sizeof(WBuffer));

    WBuffer[0] = 0x13;							//Command
    WBuffer[1] = 0x07;							//Length
    WBuffer[2] = 0x00;
    WBuffer[3] = 0x02;							//Read
    WBuffer[4] = (unsigned char)(Address);
    WBuffer[5] = (unsigned char)(Address>>8);
    WBuffer[6] = (unsigned char)(Address>>16);
    WBuffer[7] = (unsigned char)(Address>>24);
    WBuffer[8] = (unsigned char)(rLength);
    WBuffer[9] = (unsigned char)(rLength>>8);

    ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
    if(ret<0)    {
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    if(ret!=BulkLen)    {
        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }
    timeout = 10;

    while(timeout--)
    {
        ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
        if(ret>0)
        {
            ResponseID = RBuffer[0];
            ResponseLength = RBuffer[1] + (RBuffer[2]<<8);
            ResponseParam = RBuffer[3];
            for (cnt=0;cnt<60;cnt++)
                DataBuf[cnt] = RBuffer[cnt+4];

            if (ResponseID!=0x81) {
                return -30;
            }
            if (ResponseLength!=rLength) {
                return -31;
            }
            if (ResponseParam!=3) {
                return -32;
            }
            break;
        }
    }
    if(timeout==0) {
        return ERRORMSG_TIMEOUT;
    }

    if (ResponseLength>60)     {
        rBufIndex = 60;
        while(rBufIndex<(rLength-63)) {
            //Continue Packet
            timeout = 10;
            while(timeout--)
            {
                ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
                if(ret>0)
                {
                    ResponseID = RBuffer[0];
                    if (ResponseID!=0xFF) {
                        return -33;
                    }

                    for (cnt=0;cnt<63;cnt++)
                        DataBuf[cnt+rBufIndex] = RBuffer[cnt+1];

                    break;
                }
            }
            if(timeout==0)
                return ERRORMSG_TIMEOUT;

            rBufIndex += 63;
        }
        //Last Packet
        timeout = 10;
        while(timeout--)
        {
            ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
            if(ret>0)
            {
                ResponseID = RBuffer[0];
                if (ResponseID!=0xFF) {
                    return -34;
                }

                for (cnt=0;cnt<(unsigned int)(rLength-rBufIndex);cnt++)
                    DataBuf[cnt+rBufIndex] = RBuffer[cnt+1];

                break;
            } 
        }
        if(timeout==0)
            return ERRORMSG_TIMEOUT;
    }
    if((DataBuf[0]!='T')||(DataBuf[1]!='R')||(DataBuf[2]!='I')||(DataBuf[3]!=0x01))
    {  //腹癸ａ柑⊿ΤTRIM
        return ERRORMSG_GetTP_Trim_Error;
    }

    //Read Touch Link ADC
    double dszCurrent[Loop] = { 0.0 };
    for (cnt=0;cnt<Loop;cnt++)
    {
        WBuffer[0] = Bridge_T_Get_Status2;
        WBuffer[1] = 0; //Length L
        WBuffer[2] = 0; //Length H
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        if(ret<0)
        {
            ////ShowMessage("USB_Write_Fail");
            return ERRORMSG_WRITE_BLUK_FAIL;
        }
        if(ret!=BulkLen)
        {
            ////ShowMessage("Non-complete");
            return ERRORMSG_NON_COMPLETE_TRANSFER;
        }

        timeout = 10;
        while(timeout--)
        {
            ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
            if(ret>0)
            {
                ResponseID = RBuffer[0];
                ResponseLength = RBuffer[1] + (RBuffer[2]<<8);
                ResponseParam = RBuffer[3];
                ADCZener2v4 =	RBuffer[9] + (RBuffer[10]<<8);
                ADCVDD1 =		RBuffer[11] + (RBuffer[12]<<8);
                ADCVDD2 =		RBuffer[13] + (RBuffer[14]<<8);
                ADCLdoVin = 	RBuffer[17] + (RBuffer[18]<<8);
                if (ResponseID!=0xF1) {
                    return -35;
                }

                if (ResponseLength!=23) {
                    //return FALSE;
                }

                if (ResponseParam!=0) {
                    return -36;
                }
                fVBus = (2.4*4095/ADCZener2v4);
                fLdoVin = (2.4*ADCLdoVin/ADCZener2v4);
                fAccDelta = (fVBus-fLdoVin);

                
                break;
            }
        }    

        if(timeout==0)
            return ERRORMSG_TIMEOUT;
    
        //Verify VDD1 & VDD2 Voltage
        u32Standard4V3 = ADCZener2v4 * 43 / 24;
        u32Standard3V3 = ADCZener2v4 * 33 / 24;
        u32Standard2V8 = ADCZener2v4 * 28 / 24;
        u32Standard1V8 = ADCZener2v4 * 18 / 24;
        u32Tolerance4V3 = u32Standard4V3 / 20;
        u32Tolerance3V3 = u32Standard3V3 / 20;
        u32Tolerance2V8 = u32Standard2V8 / 20;
        u32Tolerance1V8 = u32Standard1V8 / 20;

        //VDD1
        if ((ADCVDD1 < (u32Standard4V3 + u32Tolerance4V3)) && 
            (ADCVDD1 > (u32Standard4V3 - u32Tolerance4V3))) {
                Vdd1 = VDD_4V3;
        }
        else if ((ADCVDD1 < (u32Standard3V3 + u32Tolerance3V3)) && 
            (ADCVDD1 > (u32Standard3V3 - u32Tolerance3V3))) {
                Vdd1 = VDD_3V3;
        }
        else if ((ADCVDD1 < (u32Standard2V8 + u32Tolerance2V8)) && 
            (ADCVDD1 > (u32Standard2V8 - u32Tolerance2V8))) {
                Vdd1 = VDD_2V8;
        }
        else if ((ADCVDD1 < (u32Standard1V8 + u32Tolerance1V8)) && 
            (ADCVDD1 > (u32Standard1V8 - u32Tolerance1V8))) {
                Vdd1 = VDD_1V8;
        }
        else {
            return -37;
        }
        //VDD2
        if ((ADCVDD2 < (u32Standard4V3 + u32Tolerance4V3)) && 
            (ADCVDD2 > (u32Standard4V3 - u32Tolerance4V3))) {
                Vdd2 = VDD_4V3;
        }	
        else if ((ADCVDD2 < (u32Standard3V3 + u32Tolerance3V3)) && 
            (ADCVDD2 > (u32Standard3V3 - u32Tolerance3V3))) {
                Vdd2 = VDD_3V3;
        }	
        else if ((ADCVDD2 < (u32Standard2V8 + u32Tolerance2V8)) && 
            (ADCVDD2 > (u32Standard2V8 - u32Tolerance2V8))) {
                Vdd2 = VDD_2V8;
        }
        else if ((ADCVDD2 < (u32Standard1V8 + u32Tolerance1V8)) && 
            (ADCVDD2 > (u32Standard1V8 - u32Tolerance1V8))) {
                Vdd2 = VDD_1V8;
        }
        else {
            //Not in specified range
            return -38;
        }
        //Get Touch Link Offset
        if ((Vdd1==VDD_4V3)&&(Vdd2==VDD_4V3)) {
            uOffset.array[0] = DataBuf[4];
            uOffset.array[1] = DataBuf[5];
            uOffset.array[2] = DataBuf[6];
            uOffset.array[3] = DataBuf[7];
            uOffset.array[4] = DataBuf[8];
            uOffset.array[5] = DataBuf[9];
            uOffset.array[6] = DataBuf[10];
            uOffset.array[7] = DataBuf[11];
        } else if ((Vdd1==VDD_4V3)&&(Vdd2==VDD_3V3)) {
            uOffset.array[0] = DataBuf[12];
            uOffset.array[1] = DataBuf[13];
            uOffset.array[2] = DataBuf[14];
            uOffset.array[3] = DataBuf[15];
            uOffset.array[4] = DataBuf[16];
            uOffset.array[5] = DataBuf[17];
            uOffset.array[6] = DataBuf[18];
            uOffset.array[7] = DataBuf[19];
        } else if ((Vdd1==VDD_4V3)&&(Vdd2==VDD_2V8)) {
            uOffset.array[0] = DataBuf[20];
            uOffset.array[1] = DataBuf[21];
            uOffset.array[2] = DataBuf[22];
            uOffset.array[3] = DataBuf[23];
            uOffset.array[4] = DataBuf[24];
            uOffset.array[5] = DataBuf[25];
            uOffset.array[6] = DataBuf[26];
            uOffset.array[7] = DataBuf[27];
        } else if ((Vdd1==VDD_4V3)&&(Vdd2==VDD_1V8)) {
            uOffset.array[0] = DataBuf[28];
            uOffset.array[1] = DataBuf[29];
            uOffset.array[2] = DataBuf[30];
            uOffset.array[3] = DataBuf[31];
            uOffset.array[4] = DataBuf[32];
            uOffset.array[5] = DataBuf[33];
            uOffset.array[6] = DataBuf[34];
            uOffset.array[7] = DataBuf[35];
        } else if ((Vdd1==VDD_3V3)&&(Vdd2==VDD_3V3)) {
            uOffset.array[0] = DataBuf[36];
            uOffset.array[1] = DataBuf[37];
            uOffset.array[2] = DataBuf[38];
            uOffset.array[3] = DataBuf[39];
            uOffset.array[4] = DataBuf[40];
            uOffset.array[5] = DataBuf[41];
            uOffset.array[6] = DataBuf[42];
            uOffset.array[7] = DataBuf[43];
        } else if ((Vdd1==VDD_3V3)&&(Vdd2==VDD_2V8)) {
            uOffset.array[0] = DataBuf[44];
            uOffset.array[1] = DataBuf[45];
            uOffset.array[2] = DataBuf[46];
            uOffset.array[3] = DataBuf[47];
            uOffset.array[4] = DataBuf[48];
            uOffset.array[5] = DataBuf[49];
            uOffset.array[6] = DataBuf[50];
            uOffset.array[7] = DataBuf[51];
        } else if ((Vdd1==VDD_3V3)&&(Vdd2==VDD_1V8)) {
            uOffset.array[0] = DataBuf[52];
            uOffset.array[1] = DataBuf[53];
            uOffset.array[2] = DataBuf[54];
            uOffset.array[3] = DataBuf[55];
            uOffset.array[4] = DataBuf[56];
            uOffset.array[5] = DataBuf[57];
            uOffset.array[6] = DataBuf[58];
            uOffset.array[7] = DataBuf[59];
        } else if ((Vdd1==VDD_2V8)&&(Vdd2==VDD_2V8)) {
            uOffset.array[0] = DataBuf[60];
            uOffset.array[1] = DataBuf[61];
            uOffset.array[2] = DataBuf[62];
            uOffset.array[3] = DataBuf[63];
            uOffset.array[4] = DataBuf[64];
            uOffset.array[5] = DataBuf[65];
            uOffset.array[6] = DataBuf[66];
            uOffset.array[7] = DataBuf[67];
        } else if ((Vdd1==VDD_2V8)&&(Vdd2==VDD_1V8)) {
            uOffset.array[0] = DataBuf[68];
            uOffset.array[1] = DataBuf[69];
            uOffset.array[2] = DataBuf[70];
            uOffset.array[3] = DataBuf[71];
            uOffset.array[4] = DataBuf[72];
            uOffset.array[5] = DataBuf[73];
            uOffset.array[6] = DataBuf[74];
            uOffset.array[7] = DataBuf[75];
        } else if ((Vdd1==VDD_1V8)&&(Vdd2==VDD_1V8)) {
            uOffset.array[0] = DataBuf[76];
            uOffset.array[1] = DataBuf[77];
            uOffset.array[2] = DataBuf[78];
            uOffset.array[3] = DataBuf[79];
            uOffset.array[4] = DataBuf[80];
            uOffset.array[5] = DataBuf[81];
            uOffset.array[6] = DataBuf[82];
            uOffset.array[7] = DataBuf[83];
        } else {
            return -39;
        }
        //Get Touch Link Slope
        uSlope.array[0] = DataBuf[84];
        uSlope.array[1] = DataBuf[85];
        uSlope.array[2] = DataBuf[86];
        uSlope.array[3] = DataBuf[87];
        uSlope.array[4] = DataBuf[88];
        uSlope.array[5] = DataBuf[89];
        uSlope.array[6] = DataBuf[90];
        uSlope.array[7] = DataBuf[91];
        //return TP Current (mA)
        dszCurrent[cnt] = ((fAccDelta/6.2*1000)-uOffset.value)*uSlope.value;

        /*CString strOutput = "";
        strOutput.Format("%04d VDD1 = %d VDD2 = %d Current = %0.4f fVBus = %0.4f fLdoVin = %0.4f fAccDelta = %0.4f uOffset.value = %0.4f uSlope.value= %0.4f\n", 
            cnt, Vdd1, Vdd2, dszCurrent[cnt], fVBus, fLdoVin, fAccDelta, uOffset.value, uSlope.value);
        OutputDebugString(strOutput);*/
    }

    std::sort(dszCurrent, dszCurrent+Loop);

    /*CString strOutput = _T("");
    int nCount = 0;
    for ( int i = 0; i < Loop; ++i ) {
        if ( dszCurrent[i] < 0 ) nCount++;
        strOutput.Format(_T("%04d Current = %0.4f\n"), i, dszCurrent[i]);
        OutputDebugString(strOutput);
    }
    strOutput.Format(_T("< 0 Count: %d\n"), nCount);
    OutputDebugString(strOutput);*/

    // 3.1  將Sorting 後的資料做?理, 將偏高及偏低的各 1%(?量) 排除, 留下 98% * 3000 = 2940 筆資料 --> SortData2
    int pos = Loop/100;
    int nSize = Loop - pos*2;
    memcpy(dszCurrent, &dszCurrent[pos], Loop - pos*2);

    // 3.2  算出 SortData2 中的最大值 =  MaxVol 和最小值 = MinVol,
    double minCurrent = dszCurrent[0];
    double maxCurrent = dszCurrent[nSize-1];

    /*if ( minCurrent < 0 )
        for ( int i = 0; i < nSize; ++i )
            dszCurrent[i] += abs(minCurrent);*/

    // 4.1  從 2940 筆中(SortData2) 取出偏高 的數值(80%MaxVol ~ MaxVol) 的數值 = MaxVolGroup
    // 4.2  從 2940 筆中(SortData2) 取出偏低 的數值(MinVol ~ (MinVol + 20%*MaxVol) 的數值 = MinVolGroup
    int minGroupIndex = 0, maxGroupIndex = 0;
    double maxCurrentGroup[Loop] = { 0.0 }, minCurrentGroup[Loop] = { 0.0 };
    for ( int i = 0; i < nSize; ++i ) {
        if ( dszCurrent[i] < minCurrent + (minCurrent + maxCurrent*0.2) ) minCurrentGroup[minGroupIndex++] = dszCurrent[i];
        if ( dszCurrent[i] > maxCurrent * 0.8 ) maxCurrentGroup[maxGroupIndex++] = dszCurrent[i];
    }

    // 5.1  將 MaxVolGroup 中的數值取排除法 (例如佔 60% 以上的平均) = MavVolAvg
    // 5.2  將 MinVolGroup 中的數值取排除法 (例如佔 60% 以上的平均) = MinVolAvg
    double minCurrentAVG = GetAVGFromEliminateMethod(minCurrentGroup, minGroupIndex, 60);
    double maxCurrentAVG = GetAVGFromEliminateMethod(maxCurrentGroup, maxGroupIndex, 60);

    // 6   取出 ActiveAvgCurrent = 1/2(MaxVolAvg+MinVolAvg)
    return (minCurrentAVG + maxCurrentAVG)/2.0;
}

int SWISPReset(void)
{
    int ret;
    unsigned char WBuffer[BulkLen]={0};
    WBuffer[0] = SWISP_Reset;
    WBuffer[1] = 0; //Length L
    WBuffer[2] = 0; //Length H
    ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
    if(ret<0)
    {
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    if(ret!=BulkLen)
    {
        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }
    return true;
}

int BridgeModeSelect(unsigned char Addr,unsigned char length,int nMode)
{
    unsigned char IBuffer[BulkLen]={0};    
    int ret;
    IBuffer[0] = Bridge_T_Mode_SEL;
    IBuffer[1] = length;
    IBuffer[2] = 0; //Length H    
    IBuffer[3] = nMode; //INT mode    
    IBuffer[4] = Addr; 
    //IBuffer[5] = length; 

    ret = USBComm_WriteToBulkEx(IBuffer, BulkLen);
    if(ret<0)
    {
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    return 1;
}

int BridgeModeSelect_Multi_Bytes(unsigned char *Cmd,unsigned char length,int nMode)
{
    unsigned char IBuffer[BulkLen]={0};    
    int ret;
    IBuffer[0] = Bridge_T_Mode_SEL;
    IBuffer[1] = length;
    IBuffer[2] = 0; //Length H    
    IBuffer[3] = nMode; //INT mode    
    for(int i=0;i<length-1;i++){
        IBuffer[4+i] = Cmd[i];
    }

    ret = USBComm_WriteToBulkEx(IBuffer, BulkLen);
    if(ret<0)
    {
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    return 1;
}


int Read_Write_INT_Flag_CMD_Packet(unsigned char Detect,unsigned char Flag)
{
    unsigned char IBuffer[BulkLen]={0};    
    int ret;
    IBuffer[0] = 0x68;
    IBuffer[1] = 0x02; //Length L
    IBuffer[2] = 0; //Length H    
    IBuffer[3] = Detect; //INT mode    
    if(Detect==0x01){
        IBuffer[1] = 0x01; //Length L
    }
    IBuffer[4] = Flag; 


    ret = USBComm_WriteToBulkEx(IBuffer, BulkLen);
    if(ret<0)
    {
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    return 1;
}
int DoPowerNoiseGetCMD(BYTE *pCMD,unsigned char *data,BYTE pLength)
{
    unsigned char IBuffer[BulkLen]={0};    
    unsigned char RBuff[UsbMaxReadLen] ={0};

    int ret;
    IBuffer[0] = 0x8E;
    IBuffer[1] = pLength; //Length L
    IBuffer[2] = 00; //Length H     

    IBuffer[3] = pCMD[0];
    IBuffer[4] = pCMD[1];
    IBuffer[5] = pCMD[2];
    IBuffer[6] = pCMD[3];
    IBuffer[7] = pCMD[4];
    IBuffer[8] = pCMD[5];
    IBuffer[9] = pCMD[6];
    IBuffer[10] = pCMD[7];
    IBuffer[11] = pCMD[8];
    ret = USBComm_ReadFromBulkEx(RBuff, BulkLen);

    if(ret == ERRORMSG_BRIDGE_STATUS_TIMEOUT){
        return ret;    
    }
    if(ret == ERRORMSG_BRIDGE_STATUS_ERROR){
        return ret;
    }
    memcpy(data,RBuff,9);
    return ret;
}
int DoPowerNoiseSendCMD(BYTE *pCMD,BYTE pLength)
{
    unsigned char IBuffer[BulkLen]={0};    
    int ret;
    IBuffer[0] = 0x50;
    IBuffer[1] = pLength; //Length L
    IBuffer[2] = 00; //Length H     

    IBuffer[3] = pCMD[0];
    IBuffer[4] = pCMD[1];
    IBuffer[5] = pCMD[2];
    IBuffer[6] = pCMD[3];
    IBuffer[7] = pCMD[4];
    IBuffer[8] = pCMD[5];
    IBuffer[9] = pCMD[6];
    IBuffer[10] = pCMD[7];
    IBuffer[11] = pCMD[8];

    ret = USBComm_WriteToBulkEx(IBuffer, BulkLen);
    if(ret<0)
    {
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    return ret;
}
BOOL CheckVoltBoardExist()
{
    int nRet = 0;
    BYTE pCMD[9]={0};
    BYTE pRead[9]={0};
    //Clear PB10
    pCMD[0] = GPIO_Control;
    pCMD[1] = GPIO_Clear;
    pCMD[2] = PORT_B;
    pCMD[3] = 0x0A;
    nRet = DoPowerNoiseSendCMD(pCMD,4);
    if(nRet<0)	return FALSE;

    //Get PB8
    pCMD[0] = GPIO_Control;
    pCMD[1] = GPIO_Get;
    pCMD[2] = PORT_B;
    pCMD[3] = 0x08;    
    nRet = DoPowerNoiseSendCMD(pCMD,4);
    if(nRet<0)	return FALSE;

    pCMD[0] = GPIO_Control;
    nRet = DoPowerNoiseGetCMD(pCMD,pRead,1);

    if(nRet<0)	return FALSE;
    if(pRead[0x04]==1)  return FALSE;
    ZeroMemory(pRead,9);


    //Set PB10
    pCMD[0] = GPIO_Control;
    pCMD[1] = GPIO_Set;
    pCMD[2] = PORT_B;
    pCMD[3] = 0x0A;
    nRet = DoPowerNoiseSendCMD(pCMD,4);
    if(nRet<0)	return FALSE;

    //Set PB10
    pCMD[0] = GPIO_Control;
    pCMD[1] = GPIO_Get;
    pCMD[2] = PORT_B;
    pCMD[3] = 0x08;
    nRet = DoPowerNoiseSendCMD(pCMD,4);
    if(nRet<=0)	return FALSE;

    pCMD[0] = GPIO_Control;
    nRet = DoPowerNoiseGetCMD(pCMD,pRead,1);
    if(nRet<0)	return FALSE;
    if(pRead[0x04]==0)  return FALSE;
    ZeroMemory(pRead,9);
    return TRUE;
}
int PowerNoiseControl(BYTE byVolt,DWORD wFrequency)
{
    //Check Power Board Exist;
    if(CheckVoltBoardExist()==FALSE)
        return -1;
    //--------------------------------------------------------------
    //Power Initial
    //--------------------------------------------------------------
    int nRet = 0;
    BYTE pCMD[9]={0};
    BYTE pRead[9]={0};
    //Clear PB0
    pCMD[0] = GPIO_Control;
    pCMD[1] = GPIO_Clear;
    pCMD[2] = PORT_B;
    pCMD[3] = 0x00;
    nRet = DoPowerNoiseSendCMD(pCMD,4);
    if(nRet<0)	return FALSE;

    //Clear PB3
    pCMD[0] = GPIO_Control;
    pCMD[1] = GPIO_Clear;
    pCMD[2] = PORT_B;
    pCMD[3] = 0x03;
    nRet = DoPowerNoiseSendCMD(pCMD,4);
    if(nRet<0)	return FALSE;

    //--------------------------------------------------------------
    //Powet Setting
    //--------------------------------------------------------------


    switch (byVolt)
    {
    case 0:   break;
    case 12: 
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 0x00;
        pCMD[4] = GPIO_Mode_Input;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return FALSE;

        pCMD[2] = PORT_B;
        pCMD[3] = 0x03;
        pCMD[4] = GPIO_Mode_Input;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return FALSE;

        break;

    case 16: 
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 0x00;
        pCMD[4] = GPIO_Mode_Input;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return FALSE;

        pCMD[2] = PORT_B;
        pCMD[3] = 0x03;
        pCMD[4] = GPIO_Mode_Output;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return FALSE;

        break;
    case 20: 
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 0x00;
        pCMD[4] = GPIO_Mode_Output;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return FALSE;

        pCMD[2] = PORT_B;
        pCMD[3] = 0x03;
        pCMD[4] = GPIO_Mode_Input;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return FALSE;

        break;
    case 24: 
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 0x00;
        pCMD[4] = GPIO_Mode_Output;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return FALSE;

        pCMD[2] = PORT_B;
        pCMD[3] = 0x03;
        pCMD[4] = GPIO_Mode_Output;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return FALSE;

        break;
    }

    pCMD[0] = PWM_Control;
    pCMD[1] = PWM_0;    
    pCMD[2] = AutoReloadMode;
    pCMD[3] = 50;
    pCMD[4] = InverterOff;
    pCMD[5] = wFrequency&0xFF;
    pCMD[6] = (wFrequency&0xFF00)>>8;
    pCMD[7] = (wFrequency&0xFF0000)>>16;
    pCMD[8] = (wFrequency&0xFF000000)>>24;

    nRet = DoPowerNoiseSendCMD(pCMD,9);
    if(nRet<0)	return FALSE;



    //Read Freq is correct?
    pCMD[0] = PWM_Control;
    nRet = DoPowerNoiseGetCMD(pCMD,pRead,1);
    if(nRet<0)	return FALSE;
    int nNoisze=0;
    nNoisze = (pRead[4])+(pRead[5]<<8)+(pRead[6]<<16);
    ZeroMemory(pRead,9);
    return nNoisze;
}
int PowerNoiseControlLED(BOOL fInitial,BOOL fBlueLight,BOOL fGreenLight)
{
    //Check Power Board Exist;
    if(CheckVoltBoardExist()==FALSE)
        return -1;
    //--------------------------------------------------------------
    //Power Initial
    //--------------------------------------------------------------
    int nRet = 0;
    BYTE pCMD[9]={0};
    BYTE pRead[9]={0};
    if(fInitial){
        //Clear PB12
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Clear;
        pCMD[2] = PORT_B;
        pCMD[3] = 12;
        nRet = DoPowerNoiseSendCMD(pCMD,4);
        if(nRet<0)	return FALSE;

        //Clear PB2
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Clear;
        pCMD[2] = PORT_B;
        pCMD[3] = 0x02;
        nRet = DoPowerNoiseSendCMD(pCMD,4);
        if(nRet<0)	return FALSE;
    }

    if(fBlueLight){
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 0x02;
        pCMD[4] = GPIO_Mode_Output;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return FALSE;
    }else{
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 0x02;
        pCMD[4] = GPIO_Mode_Input;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return FALSE;
    }

    if(fGreenLight){
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 12;
        pCMD[4] = GPIO_Mode_Output;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return FALSE;
    }else{
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 12;
        pCMD[4] = GPIO_Mode_Input;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------
//T:|0x6F|0x05|0x00|0x03|Touch-Link3 Voltage Setting|
//------------------------------------------------------------------
int SetTouchLink3_Voltage(double dVDD1,double dVDD2)
{
    unsigned char TBuffer[BulkLen]={0};
    int ret;
    int nVDD1,nVDD2;
    switch(int(dVDD1)){
        case 0:                nVDD1=0x00;     break;
        case 1:                nVDD1=0x01;     break;
        case 2:                nVDD1=0x02;     break;
        case 3:                nVDD1=0x03;     break;
        case 4:                nVDD1=0x04;     break;
        default:                nVDD1=0x03;     break;
    }
    switch(int(dVDD2)){
        case 0:                nVDD2=0x00;     break;
        case 1:                nVDD2=0x01;     break;
        case 2:                nVDD2=0x02;     break;
        case 3:                nVDD2=0x03;     break;
        case 4:                nVDD2=0x04;     break;
        default:                nVDD2=0x03;     break;
    }
    TBuffer[0] = Bridge_T_System_OP;
    TBuffer[1] = 0x02;
    TBuffer[2] = 0x00;
    TBuffer[3] = 0x05; //Voltage Setting for Touch-Link3
    TBuffer[4] = (nVDD1<<4)+nVDD2; //write

    ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
    if(ret<0)
    {        
        return ERRORMSG_WRITE_BLUK_FAIL;
    }    
    Sleep(50);
    return 1;
}
//------------------------------------------------------------------
//T:|0x6F|0x05|0x00|0x03|Touch-Link3 Voltage Setting|
//------------------------------------------------------------------
int SetTouchLink3_SleepMs(BYTE pSleepMs)
{
    unsigned char TBuffer[BulkLen]={0};
    int ret;
    int nVDD1,nVDD2;

    TBuffer[0] = Bridge_T_System_OP;
    TBuffer[1] = 0x02;
    TBuffer[2] = 0x00;
    TBuffer[3] = 0x0E; //Sleep
    TBuffer[4] = pSleepMs; //ms

    ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
    if(ret<0)
    {        
        return ERRORMSG_WRITE_BLUK_FAIL;
    }    
    return 1;
}
int GetHWICVersion()
{
    int nRet = -1;
    char StrShow[100];
    unsigned char GBuffer[BulkLen]={0};
    if(fSPIMode){
        ReadI2CReg_ProtocolA(GBuffer,0xF4,1);
        return GBuffer[0];
    }

    //Read Boot Version
    GBuffer[0] = 0x01;        
    nRet = WriteCmd(GBuffer, SWISPLength);
    if(nRet<0){
        return nRet;
    }
    memset(GBuffer,0x00,BulkLen);
    nRet = ReadCmd(GBuffer, SWISPLength);
    if(nRet<0){
        return nRet;
    }
    if(GBuffer[0]!=0x06){            
        return -1;            
    }
    return GBuffer[1];
}
int GetTPICVersion()
{
    int nRet = -1;
    char StrShow[100];
    unsigned char GBuffer[BulkLen]={0};
    //Read Boot Version
    GBuffer[0] = 0x01;        
    nRet = WriteCmd(GBuffer, SWISPLength);
    if(nRet<0){
        return nRet;
    }
    memset(GBuffer,0x00,BulkLen);
    nRet = ReadCmd(GBuffer, SWISPLength);
    if(nRet<0){
        return nRet;
    }
#if  IC_Module==IC_Module_A8008 
    if(GBuffer[0]!=0x06 || GBuffer[1]!=0x06 || (GBuffer[4]<0x04)){            
        return -1;            
    }
#elif IC_Module==IC_Module_A8010	
    if(GBuffer[0]!=0x06 || GBuffer[1]!=0x0A || (GBuffer[4]<0x04)){            
        return -1;            
    }
#elif IC_Module==IC_Module_A8015

#endif

    //Read SFR
    memset(GBuffer,0x00,BulkLen);
    GBuffer[0] = SWISP_ReadSFR;        
    GBuffer[1] = 0xE1;       //IC version address
    nRet = WriteCmd(GBuffer, SWISPLength);
    if(nRet<0){
        return nRet;
    }
    memset(GBuffer,0x00,BulkLen);
    nRet = ReadCmd(GBuffer, SWISPLength);
    if(GBuffer[0]!=0x8F){
        nRet = -1;
    }else{
        nRet = (GBuffer[1]);
    }
    return nRet;
}
int GetTPISPVersion()
{
    int nRet = -1;
    char StrShow[100];
    unsigned char GBuffer[BulkLen]={0};
    if(fSPIMode){
        ReadI2CReg(GBuffer,0x0c,4);
        return 0x0F;
    }

    //Read Boot Version
    GBuffer[0] = 0x01;        
    nRet = WriteCmd(GBuffer, SWISPLength);
    if(nRet<0){
        return nRet;
    }
    memset(GBuffer,0x00,BulkLen);
    nRet = ReadCmd(GBuffer, SWISPLength);
    if(nRet<0){
        return nRet;
    }
    return GBuffer[4];            
}

int SetI2CAddr(unsigned char Addr,unsigned char RetryTime,unsigned char NonClockStretchFlag)
{
    int ret;
    unsigned char WBuffer[BulkLen]={0};
    WBuffer[0] = Bridge_T_I2C_Addr;
    if(RetryTime!=0xFF){
        WBuffer[1] = 3; //Length L
    }else{
        WBuffer[1] = 1; //Length L
    }
    WBuffer[2] = 0; //Length H
    WBuffer[3] = (Addr<<1);
    if(RetryTime!=0xFF){
        WBuffer[4] = RetryTime;
        WBuffer[5] = NonClockStretchFlag;
    }

    ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
    if(ret<0)
    {
        //ShowMessage("USB_Write_Fail");
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    if(ret!=BulkLen)
    {
        ////ShowMessage("Non-complete");
        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }

    g_sI2cAddr = Addr;
    return 1;
}

void SetSPIDelaySpecialTime(BOOL fSpecialTime,BYTE pDelay1,BYTE pDelay2)
{
    if(fSpecialTime==false){
        DELAY1 = DEFAULTDELAY1;
        DELAY2 = DEFAULTDELAY2;
    }else{
        DELAY1 = pDelay1;
        DELAY2 = pDelay2;
    }
}
bool NotifyHWICP_ChangeMode(BOOL fSetSPIMode)
{

    int ret;    
    unsigned char EBuffer[BulkLen]={0};    
    unsigned char RData[BulkLen]={0};
    EBuffer[0] = Bridge_T_HWISP;
    EBuffer[1] = 2; //Length L
    EBuffer[2] = 0; //Length H
    EBuffer[3] = HWISP_ICP_Interface_Switch;  
    EBuffer[4] = fSetSPIMode;
    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    GetI2CStatus_Bulk(RData);
    if(RData[0x06]==fSetSPIMode)
        return true;
    else{
#ifdef _DEBUG
        OutputDebugString("Change Mode fail\r\n");
#endif // _DEBUG
        return false;
    }
}

bool SetSPI_I2CMode(BOOL fEnable)
{
    fSPI_ISPMode=fEnable;
    return true;
}


int Get_I2C_Short_Status()
{
    if ( nMultiDeviceID > 0 )
        return true;

    //Only for TIDO (HW 7) Ver(0201~)
    unsigned char GBuffer[BulkLen]={0};
    unsigned char SBuffer[BulkLen]={0},SBuff[BulkLen]={0};
    int ret;
    GBuffer[0] = Bridge_T_Get_Status;
    GBuffer[1] = 0x0;
    GBuffer[2] = 0x0;
    ret = USBComm_WriteToBulkEx(GBuffer, BulkLen);
    if(ret<0)
    {
        //ShowMessage("USB_Write_Fail");
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    SleepInProgram(10);
    ret = USBComm_ReadFromBulkEx(SBuffer,BulkLen);
    if(ret<0)
    {
        return ERRORMSG_READ_BLUK_FAIL;
    }
    if(SBuffer[0]!=0xF0)
    {
        return ERRORMSG_READ_PACKET_ERROR;
    }
    for(int i=0 ; i<SBuffer[1] ; i++)
    {
        SBuff[i]=SBuffer[4+i];
    }
    int nTouchLinkSWVersion = (SBuff[2]&0x1F)*100+SBuff[3];      
    int nTouchLinkHWVersion =(SBuff[2]>>5);       
    if(nTouchLinkHWVersion<7) return true;
    if(nTouchLinkSWVersion<201) return true;

    unsigned char TBuffer[BulkLen]={0},RBuffer[BulkLen]={0};
    TBuffer[0] = Bridge_T_System_OP;
    TBuffer[1] = 0x01;
    TBuffer[2] = 0x00;
    TBuffer[3] = 0x0B; //    0x0B: I2C Short Test. (Only valid in TIDO.)
    ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
    if(ret<0)
    {        
        return ERRORMSG_WRITE_BLUK_FAIL;
    }    
    ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
#ifdef _DEBUG
    char chOutPut[255]={0};
    sprintf(&chOutPut[0],"Get_I2C_Short_Status 0x%02x  0x%02x  0x%02x  0x%02x  -  0x%02x  0x%02x  0x%02x  0x%02x\r\n",RBuffer[0],RBuffer[1],RBuffer[2],RBuffer[3],RBuffer[4],RBuffer[5],RBuffer[6],RBuffer[7]);
    OutputDebugStringA(chOutPut);
#endif // _DEBUG
    if(ret==0)
        return ERRORMSG_READ_BLUK_FAIL;
    if(RBuffer[0]!=0x8F)
        return false;
    if(RBuffer[3]!=0x05)
        return false;
    if(RBuffer[4]!=0x00)
        return ERRORMSG_P2P3_Short;
    return 1;
}

int Set_I2C_Gearingp_Mode(BOOL fGearingMode)
{
    if ( nMultiDeviceID > 0 )
        return true;

    //Only for TIDO (HW 7) Ver(0201~)
    unsigned char GBuffer[BulkLen]={0};
    unsigned char SBuffer[BulkLen]={0},SBuff[BulkLen]={0};
    int ret;
    GBuffer[0] = Bridge_T_Get_Status;
    GBuffer[1] = 0x0;
    GBuffer[2] = 0x0;
    ret = USBComm_WriteToBulkEx(GBuffer, BulkLen);
    if(ret<0)
    {
        //ShowMessage("USB_Write_Fail");
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    SleepInProgram(10);
    ret = USBComm_ReadFromBulkEx(SBuffer,BulkLen);
    if(ret<0)
    {
        return ERRORMSG_READ_BLUK_FAIL;
    }
    if(SBuffer[0]!=0xF0)
    {
        return ERRORMSG_READ_PACKET_ERROR;
    }
    for(int i=0 ; i<SBuffer[1] ; i++)
    {
        SBuff[i]=SBuffer[4+i];
    }
    int nTouchLinkSWVersion = (SBuff[2]&0x1F)*100+SBuff[3];      
    int nTouchLinkHWVersion =(SBuff[2]>>5);       
    if(nTouchLinkHWVersion<7) return true;
    if(nTouchLinkSWVersion<201) return true;

    unsigned char IBuffer[BulkLen]={0};    
    IBuffer[0] = Bridge_T_Mode_SEL;
    IBuffer[1] = 1; //Length L 
    IBuffer[2] = 0; //Length H    
    if(fGearingMode==false)
        IBuffer[3] = 0; //Normal mode    
    else
        IBuffer[3] = 0x09; //    0x09: I2C Gearing Mode.
    IBuffer[4] = 0; 
    IBuffer[5] = 0; 
    ret = USBComm_WriteToBulkEx(IBuffer, BulkLen);
    if(ret<0)
    {
        //ShowMessage("USB_Write_Fail");
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    return 1;
}
bool ST1801_ReadCommandIO(unsigned char *pInData,int nCmdLen,unsigned char *pOutData,BOOL fUse16Bit)
{
    //unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0;    
    CString strOut;
    BYTE pStatus[8]={0},pVerify[8]={0};
    pCheckSum=0;
   
    if(fUse16Bit==true){
        if(SwitchI2C_16Bit_8Bit_ST1801(TRUE)==false){
            return false;
        }
    }else{
        if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
            return false;
        }
    }
    
    ChecksumCalculation(&pCheckSum,pInData,nCmdLen);
    pInData[nCmdLen] = pCheckSum;	//CheckSum   

    /*WriteI2CReg_2Bytes_ST1801(pInData,0xD0,8);
    */
    if(fUse16Bit){
        WriteI2CData_2Bytes_ST1801(pInData,0xD0,nCmdLen+1);
    }else{
        WriteI2CReg(pInData,0xD0,nCmdLen+1);
    }    

    pStatus[0]=0x01;
    if(fUse16Bit){
        WriteI2CData_2Bytes_ST1801(pStatus,0xF8,1);    
    }else{
        WriteI2CReg(pStatus,0xF8,1);    
    }

    SleepInProgram(nSleepTime);
    pStatus[0]=0x00;
    if(fUse16Bit){
        ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
    }else{
        ReadI2CReg(pStatus,0xF8,1);
    }   
    if(pStatus[0]==0x00){
#if (IC_Module == IC_Module_A8015)
        if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
            return ReadI2CReg_ProtocolA_HID( pOutData, 0x200, PageSize1K );
#endif
        if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_TouchLink_Bulk ) {
            ReadI2CReg_2Bytes_ST1801( pOutData, 0x200, 0x100 );
        } else if ( CommunicationMode::CommunicationMode_HID_OverI2C == g_emCommunicationMode ) {
            ReadI2CReg_ProtocolA_ADB( pOutData, 0x200, 0x100 );
		} else {
			ReadI2CReg_ProtocolA_HID( pOutData, 0x200, PageSize_HID );
		}
        return true;
    }else{
        int nCount=10;
        do 
        {
            // SleepInProgram(nSleepTime);
            if(fUse16Bit){
                ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
            }else{
                ReadI2CReg(pStatus,0xF8,1);
            }
            if(pStatus[0]==0x00){
#if (IC_Module == IC_Module_A8015)
				if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
					return ReadI2CReg_ProtocolA_HID( pOutData, 0x200, PageSize1K );
#endif
                if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_TouchLink_Bulk ) {
                    ReadI2CReg_2Bytes_ST1801( pOutData, 0x200, 0x100 );
                } else if ( CommunicationMode::CommunicationMode_Phone_ADB == g_emCommunicationMode ) {
                    ReadI2CReg_ProtocolA_ADB( pOutData, 0x200, 0x100 );
				} else {
					ReadI2CReg_ProtocolA_HID( pOutData, 0x200, PageSize_HID );
				}
                return true;
            }else{
                switch (pStatus[0])
                {
                case  0x80:    OutputDebugString("ISP Status: Unknown command ID.\r\n");       break;
                case  0x81:    OutputDebugString("ISP Status: Command packet checksum error.\r\n");       break;
                case  0x82:    OutputDebugString("ISP Status: Programming data checksum error.\r\n");       break;
                case  0x83:    OutputDebugString("ISP Status: Command time out because flash is busy.\r\n");       break;
                case  0x84:    OutputDebugString("ISP Status: Flash Unlock Key error\r\n");       break;
                case  0x85:    OutputDebugString("ISP Status: Boot Loader Unlock Key error\r\n");       break;
                case  0x86:    OutputDebugString("ISP Status: Flash Not Found\r\n");       break;
                case  0x87:    OutputDebugString("ISP Status: Flash Not Support\r\n");       break;
                }
            }
            nCount--;
            if(nCount==0) break;
        } while (1);     
        if(pStatus[0]==0x00){
#if (IC_Module == IC_Module_A8015)
			if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_HID_OverI2C )
				return ReadI2CReg_ProtocolA_HID( pOutData, 0x200, PageSize1K );
#endif
            if ( g_emCommunicationMode == CommunicationMode::CommunicationMode_TouchLink_Bulk ) {
                ReadI2CReg_2Bytes_ST1801( pOutData, 0x200, 0x100 );
            } else if ( CommunicationMode::CommunicationMode_Phone_ADB == g_emCommunicationMode ) {
                ReadI2CReg_ProtocolA_ADB( pOutData, 0x200, 0x100 );
			} else {
				ReadI2CReg_ProtocolA_HID( pOutData, 0x200, PageSize_HID );
			}
            return true;
        }else{
            return false;
        }
    }
    return true;
}
bool ST1801_WriteCommandIO(unsigned char *pInData,unsigned long Len,BOOL fUse16Bit)
{
    //     unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0;    
    BYTE pStatus[8]={0};
    if(fUse16Bit==true){
        if(SwitchI2C_16Bit_8Bit_ST1801(TRUE)==false){
            return false;
        }
    }else{
        if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
            return false;
        }
    }
    pCheckSum=0;    
    ChecksumCalculation(&pCheckSum,pInData,Len);
    pInData[Len] = pCheckSum;	//CheckSum   
    if(fUse16Bit){
        WriteI2CData_2Bytes_ST1801(pInData,0xD0,11);
    }else{
        WriteI2CReg(pInData,0xD0,11);
    }    
    pStatus[0]=0x01;
    if(fUse16Bit){
        WriteI2CData_2Bytes_ST1801(pStatus,0xF8,1);    
    }else{
        WriteI2CReg(pStatus,0xF8,1);    
    }
    SleepInProgram(nSleepTime);
    pStatus[0]=0x00;
    if(fUse16Bit){
        ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
    }else{
        ReadI2CReg(pStatus,0xF8,1);
    }   
    if(pStatus[0]==0x00){
        return true;
    }else if(pStatus[0]==0x01){ //Busy
        int nCount=10;
        do 
        {
            SleepInProgram(nSleepTime);
            if(fUse16Bit){
                ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
            }else{
                ReadI2CReg(pStatus,0xF8,1);
            }
            if(pStatus[0]==0x00){
                return true;
            }
            nCount--;
            if(nCount==0) break;
        } while (1);     
        if(pStatus[0]==0x00){
            return true;
        }else{
            return false;
        }
    }else{
        //0x80: Unknown command ID. 
        //0x81: Command packet checksum error. 
        if(pStatus[0]==0x81){
            OutputDebugStringA("Command packet checksum error.\r\n");
        }
        if(pStatus[0]==0x80){
            OutputDebugStringA("Unknown command ID. \r\n");
        }
        return false;
    }
}

bool SetST1802_WriteFlashHW_No_Read(BOOL fNoRead)
{
    fWriteflashNoRead = fNoRead;
    return true;
}

void EXPORT_API SetMultiDeviceID( int deviceID )
{
    if ( deviceID > 0 && deviceID <= 8 )
        nMultiDeviceID = deviceID;
}

int EXPORT_API GetFlashProgress()
{
    return g_pro;
}

int SetI2C_Clock( int nKHz )
{
    BYTE byteCommand[BulkLen] = { 0 };
    if ( nKHz == 0x00 )   nKHz = 400;

    nKHz = nKHz * 1000;
    byteCommand[0] = 0x50;
    byteCommand[1] = 0x05;
    byteCommand[2] = 0x00;
    byteCommand[3] = 0x03;
    byteCommand[4] = 0x00;//Set
    byteCommand[5] = nKHz&0xFF;//L
    byteCommand[6] = (nKHz>>8)&0xFF;//M
    byteCommand[7] = (nKHz>>16)&0xFF;//H
    USBComm_WriteToBulkEx( byteCommand, BulkLen );

    return 1;
}

int Get_I2C_Clock_KHz()
{
	BYTE pCommand[BulkLen] = { 0 };
	int nClock = 0;
	pCommand[0] = 0x50;
	pCommand[1] = 0x02;
	pCommand[2] = 0x00;
	pCommand[3] = 0x03;
	pCommand[4] = 0x01;//Read
	USBComm_WriteToBulkEx( pCommand, BulkLen );
	
    BYTE RData[BulkLen] = { 0 };
	USBComm_ReadFromBulkEx( RData, 64 );
	if ( (RData[0x00] == 0x8E) && (RData[0x01] == 0x04) && (RData[0x03] == 0x03) ) {
		nClock = (RData[0x06] << 16) + (RData[0x5] << 8) + RData[0x4];
	} else {
		nClock = 0;
	}

	return (nClock / 1000);
}

// James 2020/07/24 Add begin
int CommandIO_Write( _I2C_CommandIO_Packet_ * pkt )
{
    if ( NULL == pkt ) return -101;

    int ret = WriteI2CReg( (unsigned char *)pkt, 0xD0, sizeof(I2C_CommandIO_Packet) );
    if ( ret <= 0 ) return -102;

    return 0;
}

int CommandIO_SetReady()
{
    unsigned char tag = 0x01;

    int ret = WriteI2CReg( &tag, 0xF8, 1 );
    if ( ret <= 0 ) return -103;

    return 0;
}

int CommandIO_CheckReady()
{
    unsigned char tag = 0xFF;

    int ret = 1, retryNum = 0;
    do {
        ret = ReadI2CReg( &tag, 0xF8, 1 );
        if ( ret > 0 && 0x00 == tag ) return 0;

        Sleep(20);
    } while ( retryNum++ < 10 );

    if ( ret <= 0 ) return -104;

    return -105;
}

int CommandIO_Read( _I2C_CommandIO_Packet_ * pkt )
{
    if ( NULL == pkt ) return -1;

    int ret = ReadI2CReg( (unsigned char *)pkt, 0xD0, sizeof( I2C_CommandIO_Packet ) );   
    if ( ret <= 0 ) return -106;

    // 数据校验
    int nChecksum = ChecksumCalculation( NULL, (unsigned char *)pkt, pkt->validDataSize + 1 );
    if ( (nChecksum & 0xFF) != pkt->cmdData[pkt->validDataSize-1] ) 
        return -107;   

    return 0;
}

int ReadCommandIOData( unsigned char cmdType, unsigned short nAddr, unsigned char * pReadData, unsigned short nReadSize )
{
    if ( NULL == pReadData || nReadSize <= 0 ) 
        return -1;

    I2C_CommandIO_Packet pkt_in, pkt_out;
    int offset = 0;
    int nBlockSize = 24;

    int ret = 0;
    unsigned short readAddr = 0;
    do {
        if ( 0x04 == cmdType ) {            // AFE REG
            readAddr = nAddr + offset/2;
        } else {
            readAddr = nAddr + offset;
        }
        int pktDataSize = (nReadSize > nBlockSize) ? nBlockSize : nReadSize;
        pkt_in.cmdID = 0x02;
        pkt_in.validDataSize = 5;
        pkt_in.cmdData[0] = cmdType;
        pkt_in.cmdData[1] = (readAddr >> 8);      // hight byte
        pkt_in.cmdData[2] = (readAddr & 0xFF);    // low byte
        pkt_in.cmdData[3] = pktDataSize;
        pkt_in.cmdData[4] = ChecksumCalculation( NULL, (unsigned char *)&pkt_in, pkt_in.validDataSize + 1 );

        ret = CommandIO_Write( &pkt_in );
        if ( 0 != ret ) return ret;

        ret = CommandIO_SetReady();
        if ( 0 != ret ) return ret;

        ret = CommandIO_CheckReady();
        if ( 0 != ret ) return ret;

        ret = CommandIO_Read( &pkt_out );
        if ( 0 != ret ) return ret;

        // cmdData[0] = cmdType cmdData[1] = dataSize
        if ( 0x82 != pkt_out.cmdID || cmdType != pkt_out.cmdData[0] )
            continue ; // drop packet

        memcpy( pReadData + offset, &(pkt_out.cmdData[2]), pkt_out.cmdData[1] );

        offset += pktDataSize;
        nReadSize -= pktDataSize;
    } while (nReadSize>0);
    return 0;
}

int WriteCommandIOData( unsigned char cmdType, unsigned short nAddr, unsigned char * pWriteData, unsigned short nTotalSize )
{
    if ( nAddr <= 0 || NULL == pWriteData || nTotalSize <= 0 )
        return -1;

    I2C_CommandIO_Packet pkt_in;
    int offset = 0;
    int nBlockSize = 24;

    int ret = 0;
    unsigned short writeAddr = 0;
    do {
        if ( 0x04 == cmdType ) {            // AFE REG
            writeAddr = nAddr + offset/2;
        } else {
            writeAddr = nAddr + offset;
        }
        int pktDataSize = (nTotalSize > nBlockSize) ? nBlockSize : nTotalSize;
        pkt_in.cmdID = 0x01;
        pkt_in.validDataSize = pktDataSize + 5; // 1 byte checksum
        pkt_in.cmdData[0] = cmdType;
        pkt_in.cmdData[1] = (writeAddr >> 8);
        pkt_in.cmdData[2] = (writeAddr & 0xFF);
        pkt_in.cmdData[3] = pktDataSize;
        memcpy( &pkt_in.cmdData[4], pWriteData + offset, pktDataSize );
        pkt_in.cmdData[pkt_in.validDataSize-1] = ChecksumCalculation( NULL, (unsigned char *)&pkt_in, pkt_in.validDataSize + 1 );

        ret = CommandIO_Write( &pkt_in );
        if ( 0 != ret ) return ret;

        ret = CommandIO_SetReady();
        if ( 0 != ret ) return ret;

        ret = CommandIO_CheckReady();
        if ( 0 != ret ) return ret;

        offset += pktDataSize;
        nTotalSize -= pktDataSize;
    } while ( nTotalSize > 0 );

    return 0;
}

int GetSensingRawData( unsigned char * pDataBuffer, unsigned int pDataBufferSize )
{
    if ( NULL == pDataBuffer || NULL == pDataBufferSize )
        return -1;

    if ( pDataBufferSize < 0x50 )
        return -2;

    int nRet = 1;
    if ( (CommunicationMode::CommunicationMode_TouchLink_Bulk == g_emCommunicationMode) ) {
        nRet = ReadI2CReg( pDataBuffer, 0x40, 0x02 );
        if ( nRet < 0 ) return nRet;

        // data not ready
        if ( 0 == pDataBuffer[0] ) return -12;

        nRet = ReadI2CReg( &pDataBuffer[0x02], 0x42, pDataBuffer[1] );
        if ( nRet < 0 ) return nRet;
    } else if( CommunicationMode::CommunicationMode_Phone_ADB == g_emCommunicationMode ) {
        // TODO
    } else {
        nRet = HID_ReadRawdata( pDataBuffer, pDataBufferSize );
        if ( nRet <= 0 ) return nRet;
    }
	
    unsigned char nChecksum = (unsigned char)ChecksumCalculation( NULL, pDataBuffer, pDataBuffer[1] + 1 );
    if ( pDataBuffer[pDataBuffer[1] + 1] != nChecksum ) {
        printf("[API] cal = %d chk = %d\n", pDataBuffer[pDataBuffer[1] + 1], nChecksum);
        return -3;
    }

    return 0;
}

int WaitI2CReady()
{
    int ret = 0;
    for ( int i = 0; i < 5; ++i ) {
        unsigned char byteReadData = 0xFF;

        ret = ReadI2CReg( &byteReadData, 0x01, 1 );
        if ( ret <= 0 ) return ret;

        if ( (byteReadData&0x0F) != 1 )
            return 1;

        Sleep(10);
    }
    
    return 0;
}

int EXPORT_API StartFastReadRawdataMode( FM_ReadRawdata * ctx )
{
#if (IC_Module != IC_Module_ST1801) && (IC_Module != IC_Module_ST1802)
    return ERRORMSG_PROTOCOL_NOT_SUPPORT;
#endif

    if ( !ctx )
        return ERRORMSG_DATA_NULL;

    //                                   id      size      start
    std::vector< unsigned char > vecBuf{ 0x56, 0x00, 0x00, 0x00 };
    vecBuf.reserve( BulkLen );

    vecBuf.push_back( ctx->type );  // interface type
    vecBuf.push_back( ctx->addr );  // read raw data address
    
    // user want read raw data type, length and times
    for ( const auto & data : ctx->readRawdataType ) {
        vecBuf.push_back( data.dataType );      // raw data type, typical mutual(04), self(06) ...
        vecBuf.push_back( data.dataLength );    // raw data type length
        vecBuf.push_back( data.dataNum );       // raw data type read times
    }

    vecBuf.push_back( ctx->defaultReadLength ); // undefined raw data type length to read
    
    unsigned int tmpBufSize = static_cast<int>(vecBuf.size());
    unsigned int tmpValidDataSize = tmpBufSize - 3; // exclude command size

    vecBuf[1] = (tmpValidDataSize & 0xFF);
    vecBuf[2] = ((tmpValidDataSize & 0xFF00) >> 8);

    int res = USBComm_WriteToBulkEx( &vecBuf[0], tmpBufSize );
    if ( res < 0 ) return ERRORMSG_WRITE_BLUK_FAIL;

    int tmpRetry = 50;  // Total retry 500 ms
    unsigned char tmpReadBuf[BulkLen] = { 0 };
    do {
        res = USBComm_ReadFromBulkEx( tmpReadBuf, 7 );
        if ( res < 0 ) return ERRORMSG_READ_BLUK_FAIL;

        if ( (0x90 == tmpReadBuf[0])
             && (0 == tmpReadBuf[3])
             && (0xA5 == tmpReadBuf[4])
             && (0x5A == tmpReadBuf[5])
             && (0xA5 == tmpReadBuf[6]) )
        {
            return 1;
        }

        Sleep( 10 );
    } while ( --tmpRetry );

    return ERRORMSG_TIMEOUT;
}

int EXPORT_API StopFastReadRawdataMode()
{
#if (IC_Module != IC_Module_ST1801) && (IC_Module != IC_Module_ST1802)
	return ERRORMSG_PROTOCOL_NOT_SUPPORT;
#endif

    //                                   id      size      stop
	std::vector< unsigned char > vecBuf{ 0x56, 0x01, 0x00, 0x01 };

    int res = USBComm_WriteToBulkEx( &vecBuf[0], static_cast<unsigned int>(vecBuf.size()) );
	if ( res < 0 ) return ERRORMSG_WRITE_BLUK_FAIL;

	int tmpRetry = 50;  // Total retry 500 ms
	unsigned char tmpReadBuf[BulkLen] = { 0 };
	do {
		res = USBComm_ReadFromBulkEx( tmpReadBuf, 7 );
		if ( res < 0 ) return ERRORMSG_READ_BLUK_FAIL;

		if ( (0x90 == tmpReadBuf[0])
			 && (0 == tmpReadBuf[3])
			 && (0x55 == tmpReadBuf[4])
			 && (0xAA == tmpReadBuf[5])
			 && (0xA5 == tmpReadBuf[6]) )
		{
			return 1;
		}

		Sleep( 10 );
	} while ( --tmpRetry );

	return ERRORMSG_TIMEOUT;
}

int EXPORT_API SetTouchLinkVoltage( unsigned short nVDD1, unsigned short nVDD2 )
{
//     if ( (nVDD1 < 1600 || nVDD2 > 3600 || nVDD2 < 1600 || nVDD2 > 3600) && (nVDD1 != 0 && nVDD2 != 0) )
//         return -100;

    unsigned char cmd_SetVoltage[BulkLen]{ 0 };
    cmd_SetVoltage[0] = 0x6F;           // cmd
    cmd_SetVoltage[1] = 0x07;           // length L
    cmd_SetVoltage[2] = 0x00;           // length H

    cmd_SetVoltage[3] = 0x0E;           // sub cmd set voltage by value
    cmd_SetVoltage[4] = 0x02;           // set vdd1 & vdd2
    cmd_SetVoltage[5] = nVDD1 & 0xFF;   // vdd1 value L
    cmd_SetVoltage[6] = nVDD1 >> 8;     // vdd1 value H
    cmd_SetVoltage[7] = nVDD2 & 0xFF;   // vdd2 value L
    cmd_SetVoltage[8] = nVDD2 >> 8;     // vdd2 value H
    cmd_SetVoltage[9] = 0x00;           // reset pin pull high time, use default

    if ( USBComm_WriteToBulkEx( cmd_SetVoltage, BulkLen ) <= 0 )
        return ERRORMSG_WRITE_BLUK_FAIL;

    return 1;
}

int EXPORT_API ReadTPVoltage( double * pVDD1, double * pVDD2 )
{
    unsigned char szReadBuf[0xFF] = { 0 };
    int ret = GetI2CStatus_Bulk( szReadBuf );
    if ( ret <= 0 ) return ret;

    int nTouchLinkHW = (szReadBuf[2] >> 5);
    if ( nTouchLinkHW >= 6 ) {
        unsigned char szBuffer[BulkLen] = { 0x00 };
        szBuffer[0] = 0x6C;     // command
        szBuffer[1] = 0x04;     // length low
        szBuffer[2] = 0x00;     // length hight
        szBuffer[3] = 0x58;     // read cnt low
        szBuffer[4] = 0x02;     // read cnt hight
        szBuffer[5] = 0x01;     // period time low
        szBuffer[6] = 0x00;     // period time hight

        ret = USBComm_WriteToBulkEx( szBuffer, BulkLen );
        if ( ret != BulkLen ) return ERRORMSG_WRITE_BLUK_FAIL;

        ret = USBComm_ReadFromBulkEx( szBuffer, BulkLen );
        if ( ret != BulkLen ) return ERRORMSG_READ_BLUK_FAIL;

        if ( 0x8C != szBuffer[0] )
            return ERRORMSG_READ_PACKET_ERROR;

        unsigned int nVDD1 = szBuffer[4] | (szBuffer[5] << 8) | (szBuffer[6] << 16) | (szBuffer[7] << 24); // unit: mV
        unsigned int nVDD2 = szBuffer[8] | (szBuffer[9] << 8) | (szBuffer[10] << 16) | (szBuffer[11] << 24); // unit: mV

        *pVDD1 = (double)nVDD1 / 1000.0;
        *pVDD2 = (double)nVDD2 / 1000.0;
    } else {
        unsigned short ADCZener2v4, ADCVDD1, ADCVDD2;
      
        int nRet = 0;
     
        float fVolt_0, fVolt_1;
        unsigned int rtlen;
        unsigned char byCMD[64] = { 0 };

        memset( byCMD, 0, sizeof( byCMD ) );
        byCMD[0] = 0x5F;
        nRet = WriteUsb( byCMD, 64, rtlen );
        if ( nRet < 0 )
            return ERRORMSG_WRITE_BLUK_FAIL;
        
        memset( byCMD, 0, sizeof( byCMD ) );
        byCMD[0] = 0xF1;
        nRet = ReadUsb( byCMD, 64, rtlen );
        if ( nRet < 0 ) return ERRORMSG_READ_BLUK_FAIL;
        
        ADCZener2v4 = byCMD[9] + (byCMD[10] << 8);
        ADCVDD1 = byCMD[11] + (byCMD[12] << 8);
        ADCVDD2 = byCMD[13] + (byCMD[14] << 8);
       
        *pVDD1 = ADCVDD1 * 2.4 / ADCZener2v4;
        *pVDD2 = ADCVDD2 * 2.4 / ADCZener2v4;
    }

    return 1;
}

// James 2020/07/24 Add end