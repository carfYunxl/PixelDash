
// TTK_Communication_API.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"


#include <stdio.h> 
#include <memory.h>
#include <time.h>
#include <windows.h>
//#include <WinBaseh>
//#include <dbt>
#include "BulkDll.h"
#include "atlstr.h"
#include "TTK_Communication_API.h"

#define Revision        (2)
#define BufferNum       (64)
#define Flash45k        (0xB400)
#define Flash44k        (0xB000)
#define Flash16k5       (0x4200)
#define Flash1k        (0x400)
#define Flash15k        (0x3C00)
#define Flash16k        (0x4000)
#define Flash17k        (0x4400)
#define Flash30k        (0x7800)
#define Flash31k        (0x7C00)
#define Flash32k        (0x8000)
#define Flash33k        (0x8400)
#define Flash44k        (0xB000)
#define Flash63k        (0xFC00)
#define Flash64k        (0x10000)
#define DataFlashAddr   (0x1F000)
#define ReadTimeout     (20000)
#define RetryTimes      (3)
#define RetryTimesUSBIO (10)
#define PageSize        (0x100)
#define PageSize512     (0x200)
#define PageSize1K     (0x400)
#define BlockSize       (0x200)
#define UnitSize        (0x400)
#define SWISPLength     (8)

#define  ADD_L_SHIFT    0x07
#define  ADD_H_AND       0xFF

enum{
    TL3INFO_ERROR = -1,
    TL3INFO_STATUS_OLDFW = 0,
    TL3INFO_STATUS_60 = 1,
    TL3INFO_STATUS2_5F = 2,    
};
enum{
    TL3INFOHUB_DEFAULT = 0,
    TL3INFOHUB_CANTCONTROL = 0x010000,
    TL3INFOHUB_NORMAL = 0x020000,
    TL3INFOHUB_BYPASS = 0x030000, 
};
enum{
    TL3INFOMICROSWITCH_CONDUCTING = 0,
    TL3INFOMICROSWITCH_Open = 0x0100,
};
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
int nSleepTime = 1;
int nEraseSleepTime = 1;
int nSPIClock=6;
BOOL fMTP_Mode=false;
#if IC_Module==IC_Module_A8008
const int UsbMaxReadLen=512;
#elif  IC_Module==IC_Module_A8010
const int UsbMaxReadLen=1024;
#elif  IC_Module==IC_Module_A8011
const int UsbMaxReadLen=1024;
#elif  IC_Module==IC_Module_A8015
const int UsbMaxReadLen=512;
#elif  IC_Module==IC_Module_ST1801
const int UsbMaxReadLen=256;
#elif  IC_Module==IC_Module_ST1802
const int UsbMaxReadLen=256;
#endif
LARGE_INTEGER ticksPerSecond, start_tick, end_tick,RunA8010TestStart,RunA8010TestEnd;
char JumpSWISPString[9]={"STX_FWUP"};
#if IC_Module==IC_Module_A8010
BYTE DEFAULTDELAY1=0x05;
#elif  IC_Module==IC_Module_A8011
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
BOOL fSPI_I2CMode=false;
//=======================HV Board Setting ======================================
HV_Board_List pHVBL;
/*Struct_Bridge_Status Bridge_Status_Struct;*/
BOOL fSearchInitial=false;
BOOL fProtocolC=false;
BOOL fSPIMode=false;
BOOL fST1801MX_IC=FALSE;
BYTE pMXIC_WriteSize=32;
BOOL fWriteflashNoRead = false;
//===========================================================================

//===============================A8015=======================================
bool fA8015_ISP_Mode = false;
//===========================================================================

//===========Timing Record===========
LARGE_INTEGER TRecord[20];
//===========Timing Record===========end

//---------------------------------------------------------------------------
//   Important note about DLL memory management when your DLL uses the
//   static version of the RunTime Library:
//
//   If your DLL exports any functions that pass String objects (or structs/
//   classes containing nested Strings) as parameter or function results,
//   you will need to add the library MEMMGR.LIB to both the DLL project and
//   any other projects that use the DLL.  You will also need to use MEMMGR.LIB
//   if any other projects which use the DLL will be performing new or delete
//   operations on any non-TObject-derived classes which are exported from the
//   DLL. Adding MEMMGR.LIB to your project will change the DLL and its calling
//   EXE's to use the BORLNDMM.DLL as their memory manager.  In these cases,
//   the file BORLNDMM.DLL should be deployed along with your DLL.
//
//   To avoid using BORLNDMM.DLL, pass string information using "char *" or
//   ShortString parameters.
//
//   If your DLL uses the dynamic version of the RTL, you do not need to
//   explicitly add MEMMGR.LIB as this will be done implicitly for you
//---------------------------------------------------------------------------

//-----------------------------------------------------------------
void SleepInProgram(int nSleep)
{
#ifdef _DEBUG
    CString strTmp;
    strTmp.Format("Want to delay %d ms\r\n",nSleep);
    OutputDebugString(strTmp);
#endif // _DEBUG
    QueryPerformanceFrequency(&ticksPerSecond); // CPU 每秒跑幾個 ticks 
    QueryPerformanceCounter(&start_tick); 
     LONGLONG diff=0;
    do 
    {
        QueryPerformanceCounter(&end_tick); // 結束時的計數位置 
        diff = (LONGLONG)(end_tick.QuadPart - start_tick.QuadPart)*(LONGLONG)1000000 / ticksPerSecond.QuadPart;        
        diff/=1000;
        if((diff) > nSleep){
            break;
        }
    } while (1);
#ifdef _DEBUG
    strTmp.Format("Sleep cost %d ms\r\n",diff);
    OutputDebugString(strTmp);
#endif // _DEBUG
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
    int retry = RetryTimesUSBIO;
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
    WCMD[0]=WBuffer[0];
    WCMD[1]=WBuffer[1];
    WCMD[2]=WBuffer[2];
    WCMD[3]=WBuffer[3];
    WCMD[4]=WBuffer[4];
    WCMD[5]=WBuffer[5];
    WCMD[6]=WBuffer[6];
    WCMD[7]=WBuffer[7];
    WCMD[8]=WBuffer[8];
    WCMD[9]=WBuffer[9];
    WCMD[10]=WBuffer[10];


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
                    memcpy(&RBuffer[0],&RData[0],9);
                    fBridgeScanning = false;
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

int ReadI2CReg_ProtocolA(unsigned char *data, unsigned int addr, unsigned int len)
{
    unsigned char WBuffer[BulkLen]={0};
    unsigned char RBuff[UsbMaxReadLen*4] ={0};
    short Retry=1;//RetryTimes;
    time_t start,end;
    int ret;

    if(len==0)
        return -1;
    if(fSPIMode){  
        WBuffer[0] = Bridge_SPI_Read_Command_Packet;            
        WBuffer[1] = 0x08; //Length L
        WBuffer[2] = 0x00; //Length H
        WBuffer[3] = DELAY1; //DataBuf[0] = Delay1. 4us * 10(0x0A) = 40us
        WBuffer[4] = DELAY2; //DataBuf[1] = Delay2. 4us * 25(0x19)=100us
        WBuffer[5] = 0x87;   //DataBuf[2] = Signature number.
        WBuffer[6] = 0x00;  // DataBuf[3] = Transfer method.
        WBuffer[7] =  len &0xFF;  // Length L
        WBuffer[8] =  len >>8;  // Length H
        WBuffer[9] = (addr>>8)|0x80; //addr H
        WBuffer[10] = addr &0xFF; //addr     
    }else{
        WBuffer[0] = Bridge_T_Read_Reg;            
        WBuffer[1] = 3; //Length L
        WBuffer[2] = 0; //Length H
        WBuffer[3] = (unsigned char)addr;
        WBuffer[4] = (unsigned char)(len & 0xFF); //Register Length L
        WBuffer[5] = (unsigned char)(len>>8);
    }  

    while(Retry--)
    {
        ret = USBComm_WriteToBulkEx(WBuffer, BulkLen);
        if(ret<0)           {
            return ERRORMSG_WRITE_BLUK_FAIL;
        }
        if(ret!=BulkLen)            {
            return ERRORMSG_NON_COMPLETE_TRANSFER;
        }
        ret = Read_Packet_Bulk(RBuff, (unsigned short)(len&0xFFFF));
        if(ret == ERRORMSG_BRIDGE_STATUS_TIMEOUT){
            return ret;    
        }
        if(ret == ERRORMSG_BRIDGE_STATUS_ERROR){
            return ret;
        }
        if(ret==true){
            memcpy(data,RBuff,len);
            return true;
        }
    }
    if(Retry<0){
        return ret;    
    }
    return ERRORMSG_READ_I2CREG_FAIL;
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
    if(fSPI_I2CMode){
        return ReadI2CReg(data,addr,len);
    }
    unsigned char WBuffer[255]={0};
    unsigned char RBuff[UsbMaxReadLen] ={0};
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
#if  (IC_Module==IC_Module_A8010) || (IC_Module==IC_Module_A8011)
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
bool ReadI2CReg_Bulk(unsigned char Addr,unsigned char *Data)
{
    unsigned char WBuffer[BulkLen]={0};
    short Retry=RetryTimes;
    int ret;
    WBuffer[0] = Bridge_T_Read_Reg;
    WBuffer[1] = 3; //Length L
    WBuffer[2] = 0; //Length H
    WBuffer[3] = Addr;
    WBuffer[4] = 1; //Register Length L
    WBuffer[5] = 0;

    while(Retry--)
    {    
        ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
        if(ret<0)        {
            return false;
        }
        if(ret!=BulkLen)        {
            return false;
        }

        ret = Read_Packet_Bulk(Data,1);
        if( ((ret<0)&&(ret!=ERRORMSG_BRIDGE_STATUS_NAK)) ) //When status is NAK, it retries several times.
        {
            return false;
        }        else{
            return true;
        }
    }
    if(Retry==0)    {
        return false;    
    }
    return true;
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
int WriteI2CReg_ProtocolA(unsigned char *data, unsigned int addr, unsigned int len)
{  
    unsigned int WriteL=0,i=0,totallen=(len+1);
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    unsigned char WBuffer[UsbMaxReadLen*2]={0};
#elif IC_Module==IC_Module_A8010
    unsigned char WBuffer[UsbMaxReadLen*2]={0};
#elif IC_Module==IC_Module_A8011
    unsigned char WBuffer[UsbMaxReadLen*2]={0};
#elif (IC_Module==IC_Module_A8008) ||(IC_Module==IC_Module_A8015)
    unsigned char WBuffer[1024]={0};
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
    unsigned char WBuffer[0x300]={0};
    unsigned char RBuff[UsbMaxReadLen] ={0};
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
    if(fSPI_I2CMode){
        return WriteI2CReg(data,addr,len);
    }
    int i=0;
    unsigned char WBuffer[2048]={0};
    unsigned char RBuff[UsbMaxReadLen] ={0};
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
#if  (IC_Module==IC_Module_A8010) || (IC_Module==IC_Module_A8011)
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
    if(fMTP_Mode)   return true;
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
        EBuffer[0] = 0x50;	//Miscellaneous Operation
        EBuffer[1] = 6;		//Length L
        EBuffer[2] = 0;		//Length H
        nClk*=1000;
        EBuffer[3] = 0x03;	//I2C Function
        EBuffer[4] = 0x00;	//Set Clock
        EBuffer[5] = nClk&0xFF;	//SPI Clock = 10MHz
        EBuffer[6] =( nClk&0xFF00)>>8;
        EBuffer[7] = nClk>>16;
        EBuffer[8] = nClk>>24;
        ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        if(ret<0)    {
            return false;
        }       
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
#if (IC_Module==IC_Module_A8010) || (IC_Module==IC_Module_A8011) ||(IC_Module==IC_Module_A8008)  ||(IC_Module==IC_Module_A8015)
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
#ifdef _DEBUG
        //     char strOut[255]={0};
        //     sprintf(&strOut[0],"SPI Clock1: %d\r\n",SpiClock);
        //     OutputDebugString(strOut);
#endif // _DEBUG
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
    if(fSPIMode==false){
        if(fMTP_Mode){
            EBuffer[3] = I2C_ICP_MTP_ENTER;	//GPIO Control    
        }else{
            EBuffer[3] = 0x03;	//GPIO Control    
        }
        
    }else{
                 EBuffer[3] = 0x01;	//GPIO Control    
    }
    EBuffer[4] = 0x00;	//GPIO Control    
    EBuffer[5] = 0x00;	//GPIO Control    

#endif
    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    if(fMTP_Mode){
        return true;
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
    if(RData[0x00]==0x81){
        if((RData[0x04]==0xC2)&&(RData[0x05]==0x25)||(RData[0x04]==0xC2)&&(RData[0x05]==0x20)||(RData[0x04]==0xC2)&&(RData[0x05]==0x10)){
            if(RData[0x05]==0x10){
                pMXIC_WriteSize=0xFF;
            }else{
                pMXIC_WriteSize=32;
            }
            fST1801MX_IC=true;
        }else  if((RData[0X04]==0x51)||(RData[0X05]==0xC8)){
            if(((RData[0x04]==0x51)&&(RData[0x05]==0x40)&&(RData[0x06]==0x11))||
                ((RData[0x04]==0xC8)&&(RData[0x05]==0x60)&&(RData[0x06]==0x11))
                ){
                    fST1801MX_IC=false;
                    pMXIC_WriteSize=0xFF;
            }
        }else{
            fST1801MX_IC=false;
        }
    }else if (RData[0x05] ==0xC2){
        if(((RData[0x05]==0xC2)&&(RData[0x06]==0x25))||
            ((RData[0x05]==0xC2)&&(RData[0x06]==0x20))||
            ((RData[0x05]==0xC2)&&(RData[0x06]==0x10))           
            ){
            if(RData[0x06]==0x20){
                pMXIC_WriteSize=0xFF;
            }else{
                pMXIC_WriteSize=32;
            }
            fST1801MX_IC=true;
        }else{
            fST1801MX_IC=false;
        }
    }else if((RData[0X05]==0x51)||(RData[0X05]==0xC8)){
        if(((RData[0x05]==0x51)&&(RData[0x06]==0x40)&&(RData[0x07]==0x11))||
            ((RData[0x05]==0xC8)&&(RData[0x06]==0x60)&&(RData[0x07]==0x11))
            ){
                fST1801MX_IC=false;
                pMXIC_WriteSize=0xFF;
        }
    }
#if (IC_Module==IC_Module_ST1802)
    Sleep(10);
#endif
    if(CheckST1801_SPI_FLASH_Busy()==false) return false;
    return true;
}
bool CheckST1802_FlashEraseBusyWait()
{
    if(fMTP_Mode)   return true;
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
            SleepInProgram(nEraseSleepTime);
            if(nEraseSleepTime<100){
                nEraseSleepTime+=10;
            }
        }
    }while(FlashStatus!=0x00);
    return true;
}
bool CheckST1801_SPI_FLASH_Busy()
{
    if(fMTP_Mode)   return true;
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
        if(TimeOut==0){
            return false;
        }else{
            TimeOut--;
            SleepInProgram(nSleepTime);
            if(nSleepTime<5){
        //        nSleepTime++;
            }
        }
    }while(FlashStatus&0x01);
    return true;
}
bool ExReadFlashPage_Bulk_I2C_ST1802(int Addr,unsigned char *Data)
{
    int ret=0;
    BYTE EBuffer[BulkLen]={0},RBuffer[BulkLen]={0},OutPutData[MAX_PAGE_SIZE_1024*2]={0};
    //Read Flash Data
    EBuffer[0] = 0x14;	//SPI Read
    if(fMTP_Mode){
        EBuffer[1] = 0x06;	//Length L
        EBuffer[2] = 0x00;		//Length H
        EBuffer[3] =  I2C_ICP_MTP_READ;
        EBuffer[4] = (unsigned char)(Addr>>8);	//A15-A8
        EBuffer[5] = (unsigned char)(Addr);		//A7-A0
        EBuffer[6] = 0x00;
        EBuffer[7] = 0x00;
        EBuffer[8] = 0x04;

    }else{
        EBuffer[1] = 0x07;	//Length L
        EBuffer[2] = 0x00;		//Length H
        EBuffer[3] = 0x07;
        EBuffer[4] = 0x00;
        EBuffer[5] = 0x04;
        EBuffer[6] = 0x03;	//Read Data
        EBuffer[7] = (unsigned char)(Addr>>16);	//A23-A16
        EBuffer[8] = (unsigned char)(Addr>>8);	//A15-A8
        EBuffer[9] = (unsigned char)(Addr);		//A7-A0
    }
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
    memcpy(Data,OutPutData,MAX_PAGE_SIZE_1024);
    return true;
}
bool ExReadFlashPage_Bulk_SPI_ST1802(int Addr,unsigned char *Data)
{
    int ret=0;
    BYTE EBuffer[BulkLen]={0},RBuffer[BulkLen]={0},OutPutData[MAX_PAGE_SIZE_1024*2]={0};
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
    
    memcpy(Data,OutPutData,MAX_PAGE_SIZE_1024);
    return true;
}
bool ExReadFlashPage_Bulk_SPI_ST1801(unsigned short Addr,unsigned char *Data)
{
    int ret=0;
    BYTE EBuffer[BulkLen]={0},RBuffer[BulkLen]={0},OutPutData[MAX_PAGE_SIZE_1024*2]={0};
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
    memcpy(Data,OutPutData,MAX_PAGE_SIZE_1024);
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
    curPageSize = MAX_PAGE_SIZE_1024;
#elif IC_Module==IC_Module_A8015
    curPageSize = PageSize512;
#elif IC_Module==IC_Module_A8011
    curPageSize = MAX_PAGE_SIZE_1024;
#elif (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    curPageSize = MAX_PAGE_SIZE_1024;
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
#elif IC_Module==IC_Module_A8011
    RBuffer[4] = Addr >> ADD_L_SHIFT;
    RBuffer[5] = Addr & ADD_H_AND;
    if(Addr >= Flash31k)
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

//---------------------------------------------------------------------
//Write
//|0x10|0x06|0x00|0x02|AddressH|AddressL|44K|0x00|0x04|
//Read
//|0x82|RLengthL|RLengthH|Reserved|Data0 |~|Data59|--------------60bytes
//|0xFF|Data60 |~|Data122|--------------------------------------63bytes
//|0xFF|Data123|~|Data185|--------------------------------------63bytes
//|0xFF|Data186|~|Data248|--------------------------------------63bytes
//|0xFF|Data249|~|Data311|--------------------------------------63bytes
//|0xFF|Data312|~|Data374|--------------------------------------63bytes
//|0xFF|Data249|~|Data311|--------------------------------------63bytes
//....
//|0xFF|~|Data1023|---------------------------------------------
//---------------------------------------------------------------------
bool ReadFlashUnit_Bulk(unsigned short Addr,unsigned char *Data)
{

    int ret;
    char StrShow[100];
    unsigned char RBuffer[BulkLen]={0};    
    unsigned short tempAddr=Addr;
    short Retry=RetryTimes;

    if( (Addr&0x3FF)!=0 )
    {
        return false;
    }

    RBuffer[0] = Bridge_T_HWISP;
    RBuffer[1] = 6; //Length L
    RBuffer[2] = 0; //Length H

    RBuffer[3] = HWISP_Read;

    if((tempAddr) >= Flash44k)
    {
        RBuffer[6] = 0x80;
        tempAddr -= Flash44k;
    }    else    {
        RBuffer[6] = 0;
    }
    RBuffer[4] = (tempAddr) >> 8;
    RBuffer[5] = (tempAddr) & 0xFF;
    RBuffer[7] = 0x00; 
    RBuffer[8] = 0x04; // 1024 byte
    while(Retry--)
    {
        ret = USBComm_WriteToBulkEx(RBuffer,BulkLen);
        if(ret<0)        {
            return false;
        }
        if(ret!=BulkLen)        {
            return false;
        }
        SleepInProgram(20);
        ret = Read_Packet_Bulk(Data,1024);
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
    if(fST1801MX_IC){
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
    if(fMTP_Mode){

    }else    if(fSPIMode){
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
    if(fMTP_Mode==false){
        ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        if(ret<0)    {
            return false;
        }
    }
    if(fMTP_Mode){
        EBuffer[0] = 0x14;	//I2C Write
        EBuffer[1] = 0x06;	//Length L 
        EBuffer[2] = 0x04;	//Length H 
        EBuffer[3] = I2C_ICP_MTP_WRITE;	
        EBuffer[4] = (unsigned char)(Addr>>8);	//A15-A8
        EBuffer[5] = (unsigned char)(Addr);		//A7-A0
        EBuffer[6] = 0x00;
        EBuffer[7] = 0x00;
        EBuffer[8] = 0x00;
        for(i=9; i<BulkLen ; i++,Windex++){
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
        } while (Windex<0x400);
        if(CheckST1801_SPI_FLASH_Busy()==false){
            return false;
        }else{
            return true;
        }
    }    if(fST1801MX_IC){
        //Page Program Command
        if(fSPIMode){
            //SPI
            if(pMXIC_WriteSize==32){
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
            }else if(pMXIC_WriteSize==0xFF){                        
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
                if(CheckST1801_SPI_FLASH_Busy()==false){
                    return false;
                }
            }
        }else{   
            //I2C
            if(pMXIC_WriteSize==32){
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
            }else if(pMXIC_WriteSize==0xFF){
                EBuffer[0] = 0x14;	//I2C Write
                EBuffer[1] = 0x05;	//Length L 
                EBuffer[2] = 0x01;	//Length H 

                EBuffer[3] = 0x05;	
                EBuffer[4] = 0x02;
                EBuffer[5] = (unsigned char)(Addr>>16);	//A23-A16
                EBuffer[6] = (unsigned char)(Addr>>8);	//A15-A8
                EBuffer[7] = (unsigned char)(Addr);		//A7-A0
                for(i=8 ; i<BulkLen ; i++,Windex++){
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
                if(CheckST1801_SPI_FLASH_Busy()==false){
                    return false;
                }
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
            if(CheckST1801_SPI_FLASH_Busy()==false){
                return false;
            }
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
        if(CheckST1801_SPI_FLASH_Busy()==false){
            return false;
        }
    }

   // if(CheckST1801_SPI_FLASH_Busy()==false) return false;
    return true;
}
bool ExWriteFlashPage_Bulk_A8010(unsigned short Addr,unsigned char *Data)
{
    unsigned char RPage[MAX_PAGE_SIZE_1024]={0};
    unsigned char WBuffer[BulkLen]={0};
    BOOL fFirstRun=true;
#ifdef IC_Module == IC_Module_A8011
    if(Addr>=0xC000 && Addr <0xFC00){
        return true;
    }
#endif // IC_Module == IC_Module_A8011
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
#if IC_Module== IC_Module_A8011
        WBuffer[4] = ((Addr+(j<<7))>>ADD_L_SHIFT);
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
#if IC_Module== IC_Module_A8011
        WBuffer[7] = 2;
        WBuffer[8] = 0x40;
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
    unsigned char RPage[MAX_PAGE_SIZE_1024]={0};
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
#if IC_Module== IC_Module_A8011
    return ExWriteFlashPage_Bulk_A8010(Addr,Data); 
#endif // _I
#if (IC_Module== IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    return ExWriteFlashPage_Bulk_ST1801(Addr,Data);
#endif // _I
#if (IC_Module== IC_Module_A8008) || (IC_Module== IC_Module_A8015)
    if(fSPIMode)
        return ExWriteFlashPage_Bulk_WriteBySection(Addr,Data);
#endif // _I
    unsigned char RPage[MAX_PAGE_SIZE_1024]={0};
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
#if  IC_Module==IC_Module_A8008 ||  IC_Module==IC_Module_A8010 ||  IC_Module==IC_Module_A8011|| IC_Module == IC_Module_A8015	||IC_Module==IC_Module_ST1801 || (IC_Module==IC_Module_ST1802)
    unsigned char RPage[MAX_PAGE_SIZE_1024]={0};
    unsigned char WBuffer[BulkLen]={0};
#endif

    char StrShow[100];
    unsigned short tempAddr=Addr;
    unsigned short	curPageSize=0;
    unsigned short	curTimes = 0;

#if IC_Module==IC_Module_A8008 || IC_Module==IC_Module_A8010 || IC_Module==IC_Module_A8011	|| IC_Module==IC_Module_A8015	||IC_Module==IC_Module_ST1801 || (IC_Module==IC_Module_ST1802)
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
#if IC_Module==IC_Module_A8008 || IC_Module==IC_Module_A8010 || IC_Module==IC_Module_A8011|| IC_Module==IC_Module_A8015	||IC_Module==IC_Module_ST1801 || (IC_Module==IC_Module_ST1802)
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
#elif  IC_Module==IC_Module_A8011	
        WBuffer[4] = Addr >> ADD_L_SHIFT;
        WBuffer[5] = Addr & ADD_H_AND;
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
//------------------------------------------------------------------------------
//|0x10|0x84|0x00|0x01|AddressH|AddressL|44K|0x00|0x00|Data0|~|Data56|-----57bytes
//|0x7F|Data57 |~|Data119|-------------------------------------------------63bytes
//|0x7F|Data120|~|Data127|------------------------------------------------8bytes
//Address + 128
//|0x10|0x84|0x00|0x01|AddressH|AddressL|44K|0x00|0x00|Data128|~|Data184|--57bytes
//|0x7F|Data185|~|Data247|------------------------------------------------63bytes
//|0x7F|Data248|~|Data255|------------------------------------------------8bytes
//Address + 128
//|0x10|0x84|0x00|0x01|AddressH|AddressL|44K|0x00|0x00|Data256|~|Data312|--57bytes
//|0x7F|Data313|~|Data375|------------------------------------------------63bytes
//|0x7F|Data376|~|Data383|------------------------------------------------8bytes
//Address + 128
//|0x10|0x84|0x00|0x01|AddressH|AddressL|44K|0x00|0x00|Data384|~|Data440|--57bytes
//|0x7F|Data441|~|Data503|------------------------------------------------63bytes
//|0x7F|Data504|~|Data511|------------------------------------------------8bytes
//-----------------------------------------------------------------------------
//
//Address + 128
//|0x10|0x84|0x00|0x01|AddressH|AddressL|44K|0x00|0x00|Data896|~|Data952|--57bytes
//|0x7F|Data953|~|Data1015|------------------------------------------------63bytes
//|0x7F|Data1016|~|Data1023|------------------------------------------------8bytes
//-----------------------------------------------------------------------------

bool WriteFlashUnit_Bulk(unsigned short Addr,unsigned char *Data)
{
    int ret,times=0,i=0,Windex=0,ChecksumR=0,ChecksumW=0;
    unsigned char WBuffer[BulkLen]={0};
    unsigned char RUnit[UnitSize]={0};
    unsigned short tempAddr=Addr;    

    if( (Addr&0x3FF)!=0 )
    {
        return false;
    }

    while(times<8)
    {        
        //=========================
        WBuffer[0] = Bridge_T_HWISP;
        WBuffer[1] = 0x86; //Length L
        WBuffer[2] = 0x00; //Length H        
        WBuffer[3] = HWISP_Write;        

        if((tempAddr) >= Flash44k)
        {
            WBuffer[6] = 0x80;
            WBuffer[4] = (tempAddr-Flash44k) >> 8;
            WBuffer[5] = (tempAddr-Flash44k) & 0xFF;
        }
        else
        {
            WBuffer[6] = 0;
            WBuffer[4] = (tempAddr) >> 8;
            WBuffer[5] = (tempAddr) & 0xFF;
        }       


        for(i=9 ; i<64 ; i++,Windex++)
        {
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
        WBuffer[0] = Bridge_T_Continuous;
        for(i=1 ; i<64 ; i++,Windex++)
        {
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
        for(i=1 ; i<11 ; i++,Windex++)
        {
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

        //=========================
        SleepInProgram(nSleepTime);
        tempAddr+=128;
        times++;
    }
    //*==============Verification============
    ret = ReadFlashUnit_Bulk(Addr,RUnit);
    if(ret<0)
    {
        return false;
    }
    for(i=0;i<UnitSize;i++)
    {
        ChecksumR+=RUnit[i];
        ChecksumW += Data[i];
    }

    if(ChecksumR!=ChecksumW)
    {
        return false;
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
    //====check status====end*/
    Repeat=2;
    while(Repeat--)
    {    
        for(i=0;i<8;i++)
        {
            ret = WriteI2CReg_Bulk(0x00,JumpSWISPString[i]);
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
bool EraseFlashPage_SW(unsigned int Addr)
{
    int ret;
    char StrShow[100];
    unsigned char EBuffer[BulkLen]={0};
    unsigned short CK;
    if( (Addr&0xFF)!=0 )
        return false;
#if IC_Module==IC_Module_A8008 ||  IC_Module==IC_Module_A8010	||  IC_Module==IC_Module_A8011	|| IC_Module==IC_Module_A8015
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
    EBuffer[0] = SWISP_Erase;
    EBuffer[1] = 0;
#if IC_Module==IC_Module_A8008 ||  IC_Module==IC_Module_A8010 ||  IC_Module==IC_Module_A8011	|| IC_Module==IC_Module_A8015
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
#elif IC_Module==IC_Module_A8011	
    if(Addr>=0xC000&&Addr<0xFC00){
        return true;
    }
#endif	
    ret = WriteCmd(EBuffer, SWISPLength);
    if(ret<0)
        return false;
    ret = GetResponse_SW(&CK);
    if(ret==false)
        return false;
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
#elif IC_Module==IC_Module_A8011
    if(fSPIMode){       
        return Write_A8010_SPI_ISP(Addr,Data);
    }
#elif (IC_Module==IC_Module_A8008 ) || (IC_Module==IC_Module_A8015)
    if(fSPIMode){       
        return Write_A8008_SPI_ISP(Addr,Data);
    }
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
#elif IC_Module==IC_Module_A8011	
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
#elif IC_Module==IC_Module_A8011	
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
#elif IC_Module==IC_Module_A8011	
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
                WriteI2CData_2Bytes_ST1801(pData,0xF1,1);
                SleepInProgram(nSleepTime);
                ReadI2CReg_2Bytes_ST1801(pData,0xF1,1);
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
void ChecksumCalculation(unsigned short *pChecksum,unsigned char *pInData,unsigned long Len)
{
    unsigned long i;
    unsigned char LowByteChecksum;
    for(i = 0; i < Len; i++)
    {
        *pChecksum += (unsigned short)pInData[i];
        LowByteChecksum = (unsigned char)(*pChecksum & 0xFF);
        LowByteChecksum = (LowByteChecksum) >> 7 | (LowByteChecksum) << 1;
        *pChecksum = (*pChecksum & 0xFF00) | LowByteChecksum;
    }
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
    if(fMTP_Mode){
        pCheckSum=0;
        RBuffer[0] = ST1801_SWISP_Unlock_Flash;
        RBuffer[1] = 0x01; //Valid Data Size L    
        ChecksumCalculation(&pCheckSum,RBuffer,2);
        RBuffer[2] = pCheckSum;	//CheckSum       
        if(fUse16Bit){
            WriteI2CData_2Bytes_ST1801(RBuffer,0xD0,3);
        }else{
            WriteI2CReg(RBuffer,0xD0,11);
        }    
    }else{
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
    for(int i=0;i<nLen;i+=4096){
#if (IC_Module==IC_Module_ST1802)
        if(Addr>=0x1F000){
            return true;
        }
        if((Addr+nLen)>=0x1E000){
            UnlockST1801BootLoader(FALSE);
        }
#endif
        pCheckSum=0;
        if(fMTP_Mode){
            RBuffer[0] = ST1801_SWISP_Sector_Erase;
            RBuffer[1] = 0x02; //Valid Data Size L    
            RBuffer[2] = Addr/16; // 1Page = 1K, 1Sector = 16K
            ChecksumCalculation(&pCheckSum,RBuffer,3);
            RBuffer[3] = pCheckSum;	//CheckSum   
            WriteI2CReg(RBuffer,0xD0,6);
        }else{
            RBuffer[0] = ST1801_SWISP_Sector_Erase;
            RBuffer[1] = 0x04; //Valid Data Size L    
            RBuffer[2] = Addr>>16; //Addr H
            RBuffer[3] = Addr>>8; //Addr M
            RBuffer[4] = Addr&0xFF; //Addr L
            ChecksumCalculation(&pCheckSum,RBuffer,5);
            RBuffer[5] = pCheckSum;	//CheckSum   
            WriteI2CReg(RBuffer,0xD0,8);
        }
        pStatus[0]=0x01;
        WriteI2CReg(pStatus,0xF8,1);    
        int nCount=100;
        SleepInProgram(nSleepTime);
        do 
        {
            SleepInProgram(nSleepTime);
            ReadI2CReg(pStatus,0xF8,1);
            if(pStatus[0]==0x00){
                break;
            }
            nCount--;
            if(nCount==0) break;
        } while (1);
        pStatus[0]=0x00;
        ReadI2CReg(pStatus,0xF8,1);
        if(pStatus[0]!=0x00){
            return false;
        }
        Addr+=4096;
    }

    return true;
}

bool   Write_ST1801_SW_ISP_SPI(unsigned int Addr,unsigned char *Data)
{
    DWORD dwStart=GetTickCount();
    BOOL fRet = true;
    CString strTmp;
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
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
    ChecksumCalculation(&DataCheckSum,Data,0x100);
    RBuffer[7] = DataCheckSum;	//CheckSum   
    ChecksumCalculation(&pCheckSum,RBuffer,0x08);
    RBuffer[8] = pCheckSum;	//CheckSum   
    WriteI2CReg_2Bytes_ST1801(RBuffer,0xD0,9);
    if(fSPIMode==true){
        WriteI2CReg_2Bytes_ST1801(Data,0x200,0x100);
    }else{
        WriteI2CData_2Bytes_ST1801(Data,0x200,0x100);
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
            }
            nCount--;
            OutputDebugString("...Cmd Process Waiting..\r\n");
        } while (nCount>0);
    }else{
        CString strOut;
        strOut.Format("Write Flash Error 0x%02X\r\n",pStatus[0]);
        OutputDebugString(strOut);      
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
    pCheckSum=0;
    if(fMTP_Mode){
        RBuffer[0] = ST1801_SWISP_Page_Program;
        RBuffer[1] = 0x03; //Valid Data Size L
        RBuffer[2] = Addr/16;	    //Address High
        ChecksumCalculation(&DataCheckSum,Data,0x400);
        RBuffer[3] = DataCheckSum;	//CheckSum   
        ChecksumCalculation(&pCheckSum,RBuffer,0x04);
        RBuffer[4] = pCheckSum;	//CheckSum   
        WriteI2CReg_2Bytes_ST1801(RBuffer,0xD0,5);
        WriteI2CData_2Bytes_ST1801(Data,0x200,0x400);
    }else{
        RBuffer[0] = ST1801_SWISP_Page_Program;
        RBuffer[1] = 0x07; //Valid Data Size L
        RBuffer[2] = Addr>>16;	    //Address High
        RBuffer[3] = Addr>>8;	    //Address High
        RBuffer[4] = Addr&0xFF;	    //Address Low
        RBuffer[5] = 0x01;	    //Length for Read High
        RBuffer[6] = 0x00;	//Length for Read Low    
        ChecksumCalculation(&DataCheckSum,Data,0x100);
        RBuffer[7] = DataCheckSum;	//CheckSum   
        ChecksumCalculation(&pCheckSum,RBuffer,0x08);
        RBuffer[8] = pCheckSum;	//CheckSum   
        WriteI2CReg_2Bytes_ST1801(RBuffer,0xD0,9);
        WriteI2CData_2Bytes_ST1801(Data,0x200,0x100);
    }
    pStatus[0]=0x01;
    WriteI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
    SleepInProgram(nSleepTime);
    ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
    if(pStatus[0]==0x00 ){
        fRet=true;
    }else if(pStatus[0]==0x81){
        fRet=false;
         OutputDebugStringA("CheckSum Error\r\n");
    }else if(pStatus[0]!=0x01){
        int nCount = 10;
        do 
        {            
            SleepInProgram(nSleepTime);
            ReadI2CReg_2Bytes_ST1801(pStatus,0xF8,1);
            nCount--;
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
    unsigned char RBuffer[PageSize*4]={0};
    int i,ret,iData=0;
    unsigned short pCheckSum=0;    
    CString strOut;
    BYTE pStatus[8]={0},pVerify[8]={0};
    pCheckSum=0;
    if(fMTP_Mode){
        RBuffer[0] = ST1801_SWISP_Read_Flash;
        RBuffer[1] = 0x02; //Valid Data Size L
        RBuffer[2] = Addr/16;	    //Address High
        if(fSPIMode==false){
            return ST1801_ReadCommandIO(RBuffer,3,Data,true);
        }
        ChecksumCalculation(&pCheckSum,RBuffer,3);
        RBuffer[3] = pCheckSum;	//CheckSum   
        WriteI2CReg_ProtocolA(RBuffer,0xD0,4);
    }else{
        RBuffer[0] = ST1801_SWISP_Read_Flash;
        RBuffer[1] = 0x06; //Valid Data Size L
        RBuffer[2] = Addr>>16;	    //Address High
        RBuffer[3] = Addr>>8;	    //Address High
        RBuffer[4] = Addr&0xFF;	    //Address Low
        RBuffer[5] = 0x01;	    //Length for Read High
        RBuffer[6] = 0x00;	//Length for Read Low
        if(fSPIMode==false){
            return ST1801_ReadCommandIO(RBuffer,7,Data,true);
        }
        ChecksumCalculation(&pCheckSum,RBuffer,7);
        RBuffer[7] = pCheckSum;	//CheckSum   
        WriteI2CReg_ProtocolA(RBuffer,0xD0,8);
    }
    pStatus[0]=0x01;
    WriteI2CReg_ProtocolA(pStatus,0xF8,1);
    SleepInProgram(nSleepTime);
    ReadI2CReg_ProtocolA(pStatus,0xF8,1);
    if(pStatus[0]==0x00){
        if(fMTP_Mode){
            ReadI2CReg_ProtocolA(Data,0x200,0x400);
        }else{
            ReadI2CReg_ProtocolA(Data,0x200,0x100);
        }
        return true;
    }else if(pStatus[0]==0x01){
        int nCount=10;
        do{
            SleepInProgram(10);
            pStatus[0]=0xFF;
            if(ReadI2CReg_ProtocolA(pStatus,0xF8,1)<=0){
                return false;
            }
            nCount--;
        } while (pStatus[0]!=0x00 || nCount>=0);      
        if(pStatus[0]==0x00){
            if(fMTP_Mode){
                ReadI2CReg_ProtocolA(Data,0x200,0x400);
            }else{
                ReadI2CReg_ProtocolA(Data,0x200,0x100);
            }
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
#elif IC_Module==IC_Module_A8011	
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
//|0x10|0x04|0x00|0x00|AddressH|AddressL|44K|-----------Unit 1024Bytes
//------------------------------------------------------------------
bool   EraseFlashUnit_Bulk(unsigned short Addr)
{
    int ret;
    char StrShow[100];
    unsigned char EBuffer[BulkLen]={0}; 
    unsigned short tempAddr=Addr;   
    if( (Addr&0x3FF)!=0 )    {
        return false;
    }

    EBuffer[0] = Bridge_T_HWISP;
    EBuffer[1] = 4; //Length L
    EBuffer[2] = 0; //Length H

    EBuffer[3] = HWISP_Erase;
    if((tempAddr) >= Flash44k)
    {
        EBuffer[6] = 0x80;
        tempAddr -= Flash44k;
    }    else    {
        EBuffer[6] = 0;
    }

    EBuffer[4] = (tempAddr) >> 8;
    EBuffer[5] = (tempAddr) & 0xFF;

    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    if(ret!=BulkLen)    {
        return false;
    }
    SleepInProgram(40);
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
        EBuffer[6] = 0x80; //因為超過16k, 要通知Touch-Link
    }else{
        EBuffer[6] = 0;
    }
#elif  IC_Module==IC_Module_A8011	
    EBuffer[4] = Addr >> ADD_L_SHIFT;
    EBuffer[5] = Addr & ADD_H_AND;
    if(Addr >= Flash31k){
        EBuffer[4] = Addr >> ADD_L_SHIFT;
        EBuffer[5] = Addr & 0xFF;
        EBuffer[6] = 0x80; //因為超過16k, 要通知Touch-Link
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
bool MassEraseFlash_Bulk(void)
{
    int ret;    
    unsigned char EBuffer[BulkLen]={0};    

    EBuffer[0] = Bridge_T_HWISP;
    EBuffer[1] = 4; //Length L
    EBuffer[2] = 0; //Length H
    EBuffer[3] = HWISP_Mass_Erase;  
    EBuffer[6] = 0x00; //All erase
    ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
    if(ret<0)    {
        return false;
    }
    SleepInProgram(40);
    return true;
}


//------------------------------------------------------------------
int   ST1801_SPI_FLASH_LOCK(BOOL fLock)
{
    if(fMTP_Mode){
        return true;
    }
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
    if(!fST1801MX_IC){
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
    if(fST1801MX_IC){
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
    if(fMTP_Mode){
        EBuffer[0] = 0x14;	//SPI Read
        EBuffer[1] = 0x03;	//Length L
        EBuffer[2] = 0x00;		//Length H
        EBuffer[3] = I2C_ICP_MTP_SECTOR_ERASE;		//Delay1
        EBuffer[4] = (unsigned char)(Addr>>8);	//A15-A8
        EBuffer[5] = (unsigned char)(Addr);		//A7-A0
        ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
        if(ret<0)    {
            return false;
        }else{
            return ret;
        }
    }

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
    SleepInProgram(nEraseSleepTime);
    if(CheckST1802_FlashEraseBusyWait()==false){
        return false;
    }
 //   SleepInProgram(nEraseSleepTime);
    return true;
}

int   EraseFlashHW_ST1801(unsigned int Addr)
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
    unsigned char PBuffer[MAX_PAGE_SIZE_1024*4]; // 1024 bytes
    int PAddr=Addr;
    unsigned char ReadFlag = true;
    unsigned short curPageSize=0;
    unsigned short validMask=0; 

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
    int ret,i=0;
    unsigned int Eindex=0,curPos,rtLen;
#if IC_Module==IC_Module_A8008 
    unsigned char PBuffer[MAX_PAGE_SIZE_512]; // 1024 bytes
#elif  IC_Module==IC_Module_A8010
    unsigned char PBuffer[MAX_PAGE_SIZE_1024]; // 1024 bytes
#elif  IC_Module==IC_Module_A8011
    unsigned char PBuffer[MAX_PAGE_SIZE_1024]; // 1024 bytes
#elif IC_Module == IC_Module_A8015
    unsigned char PBuffer[MAX_PAGE_SIZE_512];
#elif (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    unsigned char PBuffer[MAX_PAGE_SIZE_1024*4]; // 1024 bytes
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
#elif  IC_Module==IC_Module_A8011
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
int ST1802ReadFlashHW(unsigned char *Data, unsigned int Addr , unsigned int Len)
{
    int ret,i=0;
    unsigned int Rindex=0,rtLen,readCnt=0,tmpCnt,dataLen;
    unsigned int startPos,endPos,offset;
    unsigned short curPageSize=0;     

    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
    unsigned char PBuffer[MAX_PAGE_SIZE_1024]={0}; // 1024 bytes
    if(ST1801FlashWakeUp()==false){
        return false;
    }

    int PAddr=Addr;
    while(Rindex < Len)
    {
        curPageSize = PageSize1K;
        if(PAddr>0xFFFF){
#ifdef _DEBUG
            CString strTmp;
            strTmp.Format(_T("Read ST1802 Addr :0x%06X\r\n"),PAddr);
            OutputDebugString(strTmp);
#endif // _DEBUG
        }
        if(fSPIMode==false){
            ret = ExReadFlashPage_Bulk_I2C_ST1802(PAddr,PBuffer);          
        }else{
            ret = ExReadFlashPage_Bulk_SPI_ST1802(PAddr,PBuffer);
        }
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
    if(ST1801ResetTouchLinkPullHight()==false){
        return false;
    }
    return true;
}
int ReadFlashHW(unsigned char *Data, unsigned int Addr , unsigned int Len)
{
#if IC_Module==IC_Module_ST1801
    return ST1802ReadFlashHW(Data,Addr,Len);
#endif
#if IC_Module==IC_Module_ST1802
    return ST1802ReadFlashHW(Data,Addr,Len);
#endif
    int ret,i=0;
    unsigned int Rindex=0,rtLen,readCnt=0,tmpCnt,dataLen;
    unsigned int startPos,endPos,offset;
    unsigned short curPageSize=0;     
   
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
#if (IC_Module==IC_Module_A8008) || (IC_Module==IC_Module_A8015)
    unsigned char PBuffer[MAX_PAGE_SIZE_512]={0}; // 1024 bytes
    if(fSPI_I2CMode){
        if(ST1801FlashWakeUp()==false){
            return false;
        }
    }
#elif IC_Module==IC_Module_A8010	|| IC_Module==IC_Module_A8011 || IC_Module==IC_Module_ST1801|| IC_Module==IC_Module_ST1802
    unsigned char PBuffer[MAX_PAGE_SIZE_1024]={0}; // 1024 bytes
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
#if IC_Module==IC_Module_A8010 ||IC_Module==IC_Module_A8011	||IC_Module==IC_Module_ST1801|| IC_Module==IC_Module_ST1802
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
    unsigned char PBuffer[MAX_PAGE_SIZE_1024]={0};
    unsigned short PAddr=(unsigned short)Addr;
    unsigned short curPageSize=0; 
    unsigned short validMask=0; 
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
#if IC_Module==IC_Module_A8008 ||  IC_Module==IC_Module_A8010	||  IC_Module==IC_Module_A8011	|| IC_Module==IC_Module_A8015
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
    if(fMTP_Mode){
        curPageSize = 1024;
    }
    unsigned char PBuffer[MAX_PAGE_SIZE_1024]={0};
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
    if(fMTP_Mode){
        curPageSize = 0x400;
        validMask = 0x400;
    }else if(fST1801MX_IC){
        if(pMXIC_WriteSize==32){
            curPageSize = 32; //MXIC one Page 32Bytes
            validMask = 32;
        }else if(pMXIC_WriteSize==0xFF){
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
    DWORD dwStart=GetTickCount();
    CString strTmp;
    curPageSize = 4096;
    validMask = 0x3FF;
    if(fMTP_Mode){
        curPageSize = 1024;
    }
    unsigned char PBuffer[MAX_PAGE_SIZE_1024]={0};
    int PAddr=Addr;
    if(ST1801FlashWakeUp()==false) return false;
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
    if(fMTP_Mode){
        for(int i=0;i<Len;i+=Flash1k){
            switch(i){
                case 0:
                case (Flash15k):
                case (Flash30k):
                if(fSPIMode){
                    ret = EraseFlashHW_ST1801(PAddr);
                }else{
                    ret = EraseFlashHW_ST1802(PAddr);
                }
                break;
            }
            if(ret == false){
                if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
                return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
            }     
        }
     }else{
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
     }
    if(ST1801FlashWakeUp()==false) return false;
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    Windex=0;
    PAddr=Addr;
    if(fMTP_Mode){
        curPageSize = 0x400;
        validMask = 0x400;
    }else if(fST1801MX_IC){
        if(pMXIC_WriteSize==32){
            curPageSize = 32; //MXIC one Page 32Bytes
            validMask = 32;
        }else if(pMXIC_WriteSize==0xFF){
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
        if(fMTP_Mode){
            for(i=0 ; (i<curPageSize) && (Windex<Len) ; i++,Windex++){
                PBuffer[i] = Data[Windex];
            }
        }else{
            if(Windex==0)
            {
                for(i=PAddr%validMask ; (i<curPageSize) && (Windex<Len) ; i++,Windex++){
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
    if(fWriteflashNoRead==false){
        BYTE pReadBuffer[0x20000]={0};    	 
        if(ST1801_SPI_FLASH_LOCK(TRUE)==false)  
            return false;
        dwStart=GetTickCount();
        Sleep(100);
        ReadFlashHW(pReadBuffer,Addr,Len);    
        for(int i=Addr;i<Len;i++){
            if(Data[i]!=pReadBuffer[i]){
                fRet=false;
            }
        }
    }
    if(ST1801ResetTouchLinkPullHight()==false){
        return false;
    }
    return fRet;
}
int WriteFlashHW_ST1801_SPI(unsigned char *Data, unsigned int Addr ,unsigned int Len)
{
#if  (IC_Module==IC_Module_ST1801) ||(IC_Module==IC_Module_ST1802)
    return WriteFlashHW_ST1802_SPI( Data,  Addr , Len);
#endif
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
    unsigned char PBuffer[MAX_PAGE_SIZE_1024]={0};
    int PAddr=Addr;
    if(ST1801FlashWakeUp()==false) return false;
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
    while(Windex < Len)
    {
        //ret = ExEraseFlashPage_Bulk(PAddr&(0xFFFF^validMask));
        ret = EraseFlashHW_ST1801(PAddr);
        if(ret == false){
            if(ST1801_SPI_FLASH_LOCK(TRUE)==false)   return false;
            return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
        }     
        PAddr+=curPageSize;
        Windex+=curPageSize;
    }
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    Windex=0;
    PAddr=Addr;
    if(fST1801MX_IC){
        curPageSize = 32; //MXIC one Page 32Bytes
        validMask = 32;
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
    //    if(fST1801MX_IC){
            ret = ExWriteFlashPage_Bulk(PAddr, PBuffer);
    //    }else{
   //          ret = ExWriteFlashPage_Bulk(PAddr&(0xFFFF^validMask), PBuffer);
   //     }
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
    unsigned char PBuffer[MAX_PAGE_SIZE_1024]={0};
    unsigned short PAddr=(unsigned short)Addr;
    if(ST1801FlashWakeUp()==false) return false;
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    if(Data==NULL)   return ERRORMSG_DATA_NULL;
    ret = EraseFlashHW_ST1801_ChipErase();  
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    Windex=0;
    PAddr=Addr;
    if(fST1801MX_IC){
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
        if(fST1801MX_IC){
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
    return  WriteFlashHW_ST1801_SPI(Data,  Addr , Len);
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
    unsigned char PBuffer[MAX_PAGE_SIZE_1024]={0};
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
#elif IC_Module==IC_Module_A8011	
            if(i<0xC700 || i>0xFC00){
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
    unsigned char PBuffer[MAX_PAGE_SIZE_1024]={0};
    int PAddr=Addr;
    if(ST1801FlashWakeUp()==false) return false;
    if(ST1801_SPI_FLASH_LOCK(FALSE)==false)   return false;
    if(Data==NULL)
        return ERRORMSG_DATA_NULL;
    Windex=0;
    PAddr=Addr;
    if(fST1801MX_IC){
        if(pMXIC_WriteSize==32){
            curPageSize = 32; //MXIC one Page 32Bytes
            validMask = 32;
        }else if(pMXIC_WriteSize==0xFF){
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
    unsigned char PBuffer[MAX_PAGE_SIZE_1024]={0};
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
        if(fSPI_I2CMode==false)
            ST1801FlashWakeUp();
        if(SwitchI2C_16Bit_8Bit_ST1801(FALSE)==FALSE){
            return false;
        }
        UnLock_Flash_ST1801_SW_ISP();
        if(fSPI_I2CMode==true){
            if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
                return false;
            }
        }
        if(fSPI_I2CMode==true){
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
    int ret,i=0;
    unsigned int Eindex=0;
    unsigned char BBuffer[4096]={0};
    unsigned int BAddr = Addr;
    unsigned char ReadFlag = false;
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    return Sector_Erase_ST1801_SW_ISP(Addr,Len);
#endif // IC_Module==IC_Module_ST1801
#if IC_Module==IC_Module_A8010
    if(fSPIMode){
        return Sector_Erase_A8010_SPI_ISP(Addr,Len);
    }
#endif
#if IC_Module==IC_Module_A8011
    if(fSPIMode){
        return Sector_Erase_A8010_SPI_ISP(Addr,Len);
    }
#endif
#if (IC_Module==IC_Module_A8008) || (IC_Module==IC_Module_A8015)
    if(fSPIMode){
        return Sector_Erase_A8008_SPI_ISP(Addr,Len);
    }
#endif
    while(Eindex < Len)
    {        
        ReadFlag = false;
        if(Eindex==0)
        {
            //===============read back===========
            if((BAddr&0xFF) || (((Addr+Len)-BAddr)<PageSize && (Addr+Len)&0xFF)){
                if(BAddr >= DataFlashAddr)
                    ret = ReadDataFlashBlock_SW(((BAddr-DataFlashAddr)&0xFFFFFE00), BBuffer);
                else
                    ret = ReadFlashPage_SW(BAddr&0xFFFFFE00, BBuffer);
                if(ret == false)
                {
                    return ERRORMSG_ATOM_READ_FLASH_FAIL;
                }
                ReadFlag = true;
            }                 
            for(i=BAddr&0xFF ; (i<PageSize) && (Eindex<Len) ; i++,Eindex++)      
            {
                BBuffer[i] = 0xFF;
            }          
        }else{
            //===============read back===========
            if(((Addr+Len)-(BAddr&0xFFFFFF00))<PageSize && (Addr+Len)&0xFF)     
            {
                if(BAddr >= DataFlashAddr)
                    ret = ReadDataFlashBlock_SW(((BAddr-DataFlashAddr)&0xFFFFFE00), BBuffer);
                else
                    ret = ReadFlashPage_SW(BAddr&0xFFFFFF00, BBuffer);
                if(ret == false)
                {
                    //ShowMessage("EraseFlashSW:ReadFlashBlock_SW");
                    return ERRORMSG_ATOM_READ_FLASH_FAIL;
                }
                ReadFlag = true;
            }
            for(i=0 ; (i<PageSize) && (Eindex<Len) ; i++,Eindex++)
            {
                BBuffer[i] = 0xFF;
            }
        }
        ret = EraseFlashPage_SW(BAddr&0xFFFFFF00);
        if(ret == false)
        {
            return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
        }
        if(ReadFlag == true)
        {
            ret = WriteFlashPage_SW(BAddr&0xFFFFFF00, BBuffer);
            if(ret == false)
            {
                return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
            }
        }
#if IC_Module==IC_Module_A8010
        BAddr+=PageSize1K;
#else
        BAddr+=PageSize;
#endif
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
       BAddr+=(PageSize*15);    
#endif
    }
    return true;
}

//------------------------------------------------------------------
int ReadFlashSW(unsigned char *Data, unsigned int Addr , unsigned int Len)
{
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
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
        return false;
    }
    BankSize = PageSize;
    if(fMTP_Mode)   BankSize=PageSize1K;
#else
    if(fSPIMode){
        if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
            return false;
        }
    }
#endif
    DWORD dwStart=GetTickCount();
    while(Rindex < Len)
    {
        ret = ReadFlashPage_SW((BAddr&0xFFFFFF00), BBuffer);
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
    }

#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    OutputDebugStringA("Change to 8 bit\r\n");
    if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
        OutputDebugStringA("Change to 8bit error!");
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
    unsigned char BBuffer[PageSize1K]={0};
    unsigned int BAddr = Addr;
    char strBuf[0x100],*pstrBuf=strBuf;
    CString strTmp;
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
    if(fSPI_I2CMode==false)
        ST1801FlashWakeUp();
    if(fSPI_I2CMode==true){
        if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
            return false;
        }
    }
    Sector_Erase_ST1801_SW_ISP(Addr,Len);
    if(fSPI_I2CMode==false){
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
        if(fSPI_I2CMode==false){
            if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
                return false;
            }
        }
    }
#endif
#if IC_Module==IC_Module_A8011
    if(fSPIMode)
        Sector_Erase_A8010_SPI_ISP(Addr,Len);
#endif
    if(fSPI_I2CMode==true){
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
    if(fSPIMode){                    
        return WriteFlashSW_SPI(Data,Addr,Len);
    }
    int ret,i=0;
    unsigned int Windex=0;
    unsigned char BBuffer[PageSize1K]={0};
    unsigned int BAddr = Addr;
    char strBuf[0x100],*pstrBuf=strBuf;
    CString strTmp;
#if (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
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
#if IC_Module==IC_Module_A8010
    if(fSPIMode)
        Sector_Erase_A8010_SPI_ISP(Addr,Len);
#endif
#if (IC_Module==IC_Module_A8008) || (IC_Module==IC_Module_A8015)
    if(fSPIMode){
        Sector_Erase_A8008_SPI_ISP(Addr,Len);
    }
#endif
#if IC_Module==IC_Module_A8011
    if(fSPIMode)
        Sector_Erase_A8010_SPI_ISP(Addr,Len);
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
    if(fMTP_Mode) BankSize = PageSize1K;
    if(SwitchI2C_16Bit_8Bit_ST1801(true)==false){
        return false;
    }
    UnLock_Flash_ST1801_SW_ISP();
#endif
    DWORD dwStart=GetTickCount();
    while((Windex < Len) )
    {
        if(Windex==0)
        {
            //===============read back===========         
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

        ret = WriteFlashPage_SW(BAddr&BAddrMaskComplete,BBuffer);    
        if(ret == false)
        {
            SleepInProgram(50);
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
//     if(SwitchI2C_16Bit_8Bit_ST1801(false)==false){
//         return false;
//     }
    Lock_Flash_ST1801_SW_ISP(false);
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
    if(fSPI_I2CMode==false)
        ST1801FlashWakeUp();
    if(fSPI_I2CMode==true){
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
    CString strTmp;
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
    int ret;   
    //SleepInProgram(nSleepTime); // for TDU Read Int. 
    ret = Read_Packet_INT_Bulk(Data,length);    
    if(fUseNewINTProtocol){

    }
    return ret;

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
    nTouchLinkSWVersion = (SBuff[2]&0x1F)*100+SBuff[3];      
    nTouchLinkHWVersion =(SBuff[2]>>5);       
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
void LOG_ERROR( const char * msg )
{
    if ( NULL == msg )
        return ;

#ifdef _PID6101
    FILE * fp = fopen( "error_6101.log", "a+" );
#elif _PID6102
    FILE * fp = fopen( "error_6102.log", "a+" );
#elif _PID6103
    FILE * fp = fopen( "error_6103.log", "a+" );
#elif _PID6104
    FILE * fp = fopen( "error_6104.log", "a+" );
#else
    FILE * fp = fopen( "error.log", "a+" );
#endif
    if ( NULL == fp ) return ;

    fwrite( msg, strlen(msg), 1, fp );
    fclose( fp );
    fp = NULL;
}

double ReadTPCurrent_TL3(void)
{
    int ret, timeout;
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

    char msg[255] = { 0 };
    ret = USBComm_WriteToBulkEx(WBuffer,BulkLen);
    if(ret<0)    {
        sprintf(msg, "USBComm_WriteToBulkEx Fail, ret = %d\n",ret);
        LOG_ERROR(msg);
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    if(ret!=BulkLen)    {
        sprintf(msg, "USBComm_WriteToBulkEx Result Length Error, must = %d ret = %d \n",BulkLen,ret);
        LOG_ERROR(msg);
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
                sprintf(msg, "Touch Link Reponse ID Error, Must 0x81, But Return 0x%0x\n", ResponseID);
                LOG_ERROR(msg);
                return -100;
            }
            if (ResponseLength!=rLength) {
                sprintf(msg, "Touch Link Reponse Length Error, Must %d, But Return %d\n", rLength, ResponseLength);
                LOG_ERROR(msg);
                return -101;
            }
            if (ResponseParam!=3) {
                sprintf(msg, "Touch Link Reponse Param Error, Must 3(Data Flash ISP Data), But Return %d\n", ResponseParam);
                LOG_ERROR(msg);
                return -102;
            }
            break;
        }
    }
    if(timeout==0) {
        LOG_ERROR("USBComm_ReadFromBulkEx Time Out 1\n");
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
                        sprintf(msg, "Touch Link Reponse Continue ID Error, Must 0xFF, But Return 0x%0x\n", ResponseID);
                        LOG_ERROR(msg);
                        return -103;
                    }

                    for (cnt=0;cnt<63;cnt++)
                        DataBuf[cnt+rBufIndex] = RBuffer[cnt+1];

                    break;
                }
            }
            if(timeout==0) {
                LOG_ERROR("USBComm_ReadFromBulkEx Time Out 2\n");
                return ERRORMSG_TIMEOUT;
            }

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
                    sprintf(msg, "Touch Link Reponse Last Continue ID Error, Must 0xFF, But Return 0x%0x\n", ResponseID);
                    LOG_ERROR(msg);
                    return -104;
                }

                for (cnt=0;cnt<(unsigned int)(rLength-rBufIndex);cnt++)
                    DataBuf[cnt+rBufIndex] = RBuffer[cnt+1];

                break;
            } 
        }
        if(timeout==0) {
            LOG_ERROR("USBComm_ReadFromBulkEx Time Out 3\n");
            return ERRORMSG_TIMEOUT;
        }
    }
    if((DataBuf[0]!='T')||(DataBuf[1]!='R')||(DataBuf[2]!='I'))
    {  //版號對了€裡面沒有TRIM值
        LOG_ERROR("NO TRIM\n");
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
                sprintf(msg, "USBComm_WriteToBulkEx Status Fail, ret = %d\n",ret);
                LOG_ERROR(msg);
                return ERRORMSG_WRITE_BLUK_FAIL;
            }
            if(ret!=BulkLen)
            {
                ////ShowMessage("Non-complete");
                sprintf(msg, "USBComm_WriteToBulkEx Status Length Fail, must = %d ret = %d\n",BulkLen,ret);
                LOG_ERROR(msg);
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
                        sprintf(msg, "LoopCnt: %d timeout: %d Touch Link Reponse Status ID Error, Must 0xF1, But Return 0x%0x\n", cnt, timeout, ResponseID);
                        LOG_ERROR(msg);
                        return -105;
                    }

                    if (ResponseLength!=23) {
                        //return FALSE;
                    }

                    if (ResponseParam!=0) {
                        sprintf(msg, "Touch Link Reponse Status Param Error, Must 0(Running), But Return %d\n", ResponseParam);
                        LOG_ERROR(msg);
                        return -106;
                    }
                    fVBus = (2.4*4095/ADCZener2v4);
                    fLdoVin = (2.4*ADCLdoVin/ADCZener2v4);
                    fAccDelta += (fVBus-fLdoVin);
                    break;
                }
            }    

            if(timeout==0) {
                LOG_ERROR("USBComm_ReadFromBulkEx Time Out 4\n");
                return ERRORMSG_TIMEOUT;
            }
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
            sprintf(msg, "VDD1 Not In Range, ADCVDD1: %d Stand4V3: %d Toler4V3: %d Stand3V3: %d Toler3V3: %d Stand2V8: %d Toler2V8: %d Stand1V8: %d Toler1V8: %d\n", ADCVDD1, u32Standard4V3, u32Tolerance4V3, u32Standard3V3, u32Tolerance3V3, u32Standard2V8, u32Tolerance2V8, u32Standard1V8, u32Tolerance1V8);
            LOG_ERROR(msg);
            return -107;
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
            sprintf(msg, "VDD2 Not In Range, ADCVDD1: %d Stand4V3: %d Toler4V3: %d Stand3V3: %d Toler3V3: %d Stand2V8: %d Toler2V8: %d Stand1V8: %d Toler1V8: %d\n", ADCVDD2, u32Standard4V3, u32Tolerance4V3, u32Standard3V3, u32Tolerance3V3, u32Standard2V8, u32Tolerance2V8, u32Standard1V8, u32Tolerance1V8);
            LOG_ERROR(msg);
            return -108;
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
            sprintf(msg,"VDD ERROR, VDD1 = %d VDD2 = %d\n", Vdd1, Vdd2);
            LOG_ERROR(msg);
            return -109;
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
        sprintf(msg, "TRIM Data Error, must 0x01, read 0x%0x\n",DataBuf[3]);
        LOG_ERROR(msg);
        return ERRORMSG_GetTP_Trim_Error;
    }
}

double ReadTPCurrent(void)
{
    int ret, timeout;
    unsigned char WBuffer[BulkLen], RBuffer[BulkLen], DataBuf[512]={0};
    unsigned char ResponseID, ResponseParam, Loop=30;
    unsigned short ResponseLength, rLength=512, rBufIndex;
    unsigned short ADCZener2v4, ADCVDD1, ADCVDD2, ADCLdoVin;
    unsigned int u32Standard5V0, u32Standard3V3, u32Standard2V8, u32Standard1V8;
    unsigned int u32Tolerance5V0, u32Tolerance3V3, u32Tolerance2V8, u32Tolerance1V8;
    unsigned int Address = 0x0001F000, cnt;
    double fVBus, fLdoVin, fAccDelta=0, fAccCurrent=0;
    E_VDD Vdd1, Vdd2;
    uDouble uOffset, uSlope;
    //Read Touch Link Data Flash              
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
    if(ret<0)
    {
        return ERRORMSG_WRITE_BLUK_FAIL;
    }
    if(ret!=BulkLen)
    {

        return ERRORMSG_NON_COMPLETE_TRANSFER;
    }
    //First Packet
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
                return FALSE;
            }
            if (ResponseLength!=rLength) {
                return FALSE;
            }
            if (ResponseParam!=3) {
                return FALSE;
            }
            break;
        }
    }
    if(timeout==0) {
        return ERRORMSG_TIMEOUT;
    }
    if (ResponseLength>60) 
    {
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
                        return FALSE;
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
                    return FALSE;
                }

                for (cnt=0;cnt<(unsigned int)(rLength-rBufIndex);cnt++)
                    DataBuf[cnt+rBufIndex] = RBuffer[cnt+1];

                break;
            } 
        }
        if(timeout==0)
            return ERRORMSG_TIMEOUT;
    }
    //Trim Data Verify Tag
    if((DataBuf[0]!='T')||(DataBuf[1]!='R')||(DataBuf[2]!='I')||(DataBuf[3]!='M'))
    {  //版號對了€裡面沒有TRIM值
        return ERRORMSG_GetTP_Trim_Error;
    }
    //Read Touch Link ADC
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
                //ADC12V =		byCMD[15] + (byCMD[16]<<8);
                ADCLdoVin = 	RBuffer[17] + (RBuffer[18]<<8);
                //ADC5 =			byCMD[19] + (byCMD[20]<<8);
                //ADC6 =			byCMD[21] + (byCMD[22]<<8);
                //ADC7 =			byCMD[23] + (byCMD[24]<<8);

                if (ResponseID!=0xF1) {
                    return FALSE;
                }
                if (ResponseLength!=21) {
                    return FALSE;
                }
                if (ResponseParam!=0) {
                    return FALSE;
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
    u32Standard5V0 = ADCZener2v4 * 50 / 24;
    u32Standard3V3 = ADCZener2v4 * 33 / 24;
    u32Standard2V8 = ADCZener2v4 * 28 / 24;
    u32Standard1V8 = ADCZener2v4 * 18 / 24;
    u32Tolerance5V0 = u32Standard5V0 / USB_Tolerance;
    u32Tolerance3V3 = u32Standard3V3 / V33_Tolerance;
    u32Tolerance2V8 = u32Standard2V8 / V28_Tolerance;
    u32Tolerance1V8 = u32Standard1V8 / V18_Tolerance;

    //VDD1
    if ((ADCVDD1 < (u32Standard5V0 + u32Tolerance5V0)) && 
        (ADCVDD1 > (u32Standard5V0 - u32Tolerance5V0))) {
            //VDD1 = 5.0V
            Vdd1 = VDD_5V;
    }
    else if ((ADCVDD1 < (u32Standard3V3 + u32Tolerance3V3)) && 
        (ADCVDD1 > (u32Standard3V3 - u32Tolerance3V3))) {
            //VDD1 = 3.3V
            Vdd1 = VDD_3V3;
    }
    else if ((ADCVDD1 < (u32Standard2V8 + u32Tolerance2V8)) && 
        (ADCVDD1 > (u32Standard2V8 - u32Tolerance2V8))) {
            //VDD1 = 2.8V
            Vdd1 = VDD_2V8;
    }
    else {
        //Not in specified range
        return FALSE;
    }
    //VDD2
    if ((ADCVDD2 < (u32Standard3V3 + u32Tolerance3V3)) && 
        (ADCVDD2 > (u32Standard3V3 - u32Tolerance3V3))) {
            //VDD2 = 3.3V
            Vdd2 = VDD_3V3;
    }	
    else if ((ADCVDD2 < (u32Standard2V8 + u32Tolerance2V8)) && 
        (ADCVDD2 > (u32Standard2V8 - u32Tolerance2V8))) {
            //VDD2 = 2.8V
            Vdd2 = VDD_2V8;
    }
    else if ((ADCVDD2 < (u32Standard1V8 + u32Tolerance1V8)) && 
        (ADCVDD2 > (u32Standard1V8 - u32Tolerance1V8))) {
            //VDD2 = 1.8V
            Vdd2 = VDD_1V8;
    }
    else {
        //Not in specified range
        return FALSE;
    }
    //Get Touch Link Offset
    if ((Vdd1==VDD_5V)&&(Vdd2==VDD_3V3)) {
        uOffset.array[0] = DataBuf[4];
        uOffset.array[1] = DataBuf[5];
        uOffset.array[2] = DataBuf[6];
        uOffset.array[3] = DataBuf[7];
        uOffset.array[4] = DataBuf[8];
        uOffset.array[5] = DataBuf[9];
        uOffset.array[6] = DataBuf[10];
        uOffset.array[7] = DataBuf[11];
    } else if ((Vdd1==VDD_5V)&&(Vdd2==VDD_2V8)) {
        uOffset.array[0] = DataBuf[12];
        uOffset.array[1] = DataBuf[13];
        uOffset.array[2] = DataBuf[14];
        uOffset.array[3] = DataBuf[15];
        uOffset.array[4] = DataBuf[16];
        uOffset.array[5] = DataBuf[17];
        uOffset.array[6] = DataBuf[18];
        uOffset.array[7] = DataBuf[19];
    } else if ((Vdd1==VDD_5V)&&(Vdd2==VDD_1V8)) {
        uOffset.array[0] = DataBuf[20];
        uOffset.array[1] = DataBuf[21];
        uOffset.array[2] = DataBuf[22];
        uOffset.array[3] = DataBuf[23];
        uOffset.array[4] = DataBuf[24];
        uOffset.array[5] = DataBuf[25];
        uOffset.array[6] = DataBuf[26];
        uOffset.array[7] = DataBuf[27];
    } else if ((Vdd1==VDD_3V3)&&(Vdd2==VDD_3V3)) {
        uOffset.array[0] = DataBuf[28];
        uOffset.array[1] = DataBuf[29];
        uOffset.array[2] = DataBuf[30];
        uOffset.array[3] = DataBuf[31];
        uOffset.array[4] = DataBuf[32];
        uOffset.array[5] = DataBuf[33];
        uOffset.array[6] = DataBuf[34];
        uOffset.array[7] = DataBuf[35];
    } else if ((Vdd1==VDD_3V3)&&(Vdd2==VDD_2V8)) {
        uOffset.array[0] = DataBuf[36];
        uOffset.array[1] = DataBuf[37];
        uOffset.array[2] = DataBuf[38];
        uOffset.array[3] = DataBuf[39];
        uOffset.array[4] = DataBuf[40];
        uOffset.array[5] = DataBuf[41];
        uOffset.array[6] = DataBuf[42];
        uOffset.array[7] = DataBuf[43];
    } else if ((Vdd1==VDD_3V3)&&(Vdd2==VDD_1V8)) {
        uOffset.array[0] = DataBuf[44];
        uOffset.array[1] = DataBuf[45];
        uOffset.array[2] = DataBuf[46];
        uOffset.array[3] = DataBuf[47];
        uOffset.array[4] = DataBuf[48];
        uOffset.array[5] = DataBuf[49];
        uOffset.array[6] = DataBuf[50];
        uOffset.array[7] = DataBuf[51];
    } else if ((Vdd1==VDD_2V8)&&(Vdd2==VDD_2V8)) {
        uOffset.array[0] = DataBuf[52];
        uOffset.array[1] = DataBuf[53];
        uOffset.array[2] = DataBuf[54];
        uOffset.array[3] = DataBuf[55];
        uOffset.array[4] = DataBuf[56];
        uOffset.array[5] = DataBuf[57];
        uOffset.array[6] = DataBuf[58];
        uOffset.array[7] = DataBuf[59];
    } else if ((Vdd1==VDD_2V8)&&(Vdd2==VDD_1V8)) {
        uOffset.array[0] = DataBuf[60];
        uOffset.array[1] = DataBuf[61];
        uOffset.array[2] = DataBuf[62];
        uOffset.array[3] = DataBuf[63];
        uOffset.array[4] = DataBuf[64];
        uOffset.array[5] = DataBuf[65];
        uOffset.array[6] = DataBuf[66];
        uOffset.array[7] = DataBuf[67];
    } else {
        return FALSE;
    }
    //Get Touch Link Slope
    uSlope.array[0] = DataBuf[68];
    uSlope.array[1] = DataBuf[69];
    uSlope.array[2] = DataBuf[70];
    uSlope.array[3] = DataBuf[71];
    uSlope.array[4] = DataBuf[72];
    uSlope.array[5] = DataBuf[73];
    uSlope.array[6] = DataBuf[74];
    uSlope.array[7] = DataBuf[75];
    return (((fAccDelta/Loop)/6.2*1000)-uOffset.value)*uSlope.value;
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
}
int HV_Board_DoCmd(int nVolt,DWORD wFrequency)
{
    if(CheckVoltBoardExist()==FALSE || fSearchInitial==false)
        return -1;
    int nRet = 1;
    int nHVBLCount = sizeof(HV_Board_List)/sizeof(HV_INFO);

    //=======================Here to Get HV_Board version=============================
    int nHV_Board_Version=0; 
    //===========================================================================
    BOOL fFind=TRUE;
    int nFind=-1;
    for(int k=0;k<5;k++){
        if(nVolt==pHVBL.pHV[nHV_Board_Version].nVoltage[k]){
            fFind = true;   
            nFind=k;
            break;
        }
    }
    if(fFind==false)  return -1;
    //--------------------------------------------------------------
    //Power Initial
    //--------------------------------------------------------------

    BYTE pCMD[9]={0};
    BYTE pRead[9]={0};
    //Clear PB0
    pCMD[0] = GPIO_Control;
    pCMD[1] = GPIO_Clear;
    pCMD[2] = PORT_B;
    pCMD[3] = 0x00;
    nRet = DoPowerNoiseSendCMD(pCMD,4);
    if(nRet<0)	return -1;

    //Clear PB3
    pCMD[0] = GPIO_Control;
    pCMD[1] = GPIO_Clear;
    pCMD[2] = PORT_B;
    pCMD[3] = 0x03;
    nRet = DoPowerNoiseSendCMD(pCMD,4);
    if(nRet<0)	return -1;

    //--------------------------------------------------------------
    //Power Setting
    //--------------------------------------------------------------
    switch (nFind)
    {
    case 0:   
        PowerNoiseControlLED(TRUE,FALSE,FALSE);
        PowerNoiseControlLED(FALSE,FALSE,FALSE);
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 0x00;
        pCMD[4] = GPIO_Mode_Input;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return -1;

        pCMD[2] = PORT_B;
        pCMD[3] = 0x03;
        pCMD[4] = GPIO_Mode_Input;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return -1;
        wFrequency=0;
        break;
    case 1: 
        PowerNoiseControlLED(TRUE,FALSE,FALSE);
        PowerNoiseControlLED(FALSE,FALSE,TRUE);
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 0x00;
        pCMD[4] = GPIO_Mode_Input;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return -1;

        pCMD[2] = PORT_B;
        pCMD[3] = 0x03;
        pCMD[4] = GPIO_Mode_Input;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return -1;
        break;

    case 2: 
        PowerNoiseControlLED(TRUE,FALSE,FALSE);
        PowerNoiseControlLED(FALSE,TRUE,FALSE);
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 0x00;
        pCMD[4] = GPIO_Mode_Input;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return -1;

        pCMD[2] = PORT_B;
        pCMD[3] = 0x03;
        pCMD[4] = GPIO_Mode_Output;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return -1;

        break;
    case 3: 
        PowerNoiseControlLED(TRUE,FALSE,FALSE);
        PowerNoiseControlLED(FALSE,TRUE,TRUE);
        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 0x00;
        pCMD[4] = GPIO_Mode_Output;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return -1;

        pCMD[2] = PORT_B;
        pCMD[3] = 0x03;
        pCMD[4] = GPIO_Mode_Input;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return -1;

        break;
    case 4: 
        PowerNoiseControlLED(TRUE,FALSE,FALSE);
        PowerNoiseControlLED(FALSE,FALSE,FALSE);
        SleepInProgram(200);
        PowerNoiseControlLED(FALSE,TRUE,TRUE);
        SleepInProgram(500);
        PowerNoiseControlLED(FALSE,FALSE,FALSE);
        SleepInProgram(200);
        PowerNoiseControlLED(FALSE,TRUE,true);
        SleepInProgram(500);


        pCMD[0] = GPIO_Control;
        pCMD[1] = GPIO_Open;    
        pCMD[2] = PORT_B;
        pCMD[3] = 0x00;
        pCMD[4] = GPIO_Mode_Output;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return -1;

        pCMD[2] = PORT_B;
        pCMD[3] = 0x03;
        pCMD[4] = GPIO_Mode_Output;
        nRet = DoPowerNoiseSendCMD(pCMD,5);
        if(nRet<0)	return -1;

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
    if(nRet<0)	return -1;

    return nRet;
}
int SearchHV_Board_Support(int nDoCount,int* nVolt)
{
    if(CheckVoltBoardExist()==FALSE)
        return -1;
    fSearchInitial=true;
    //=======================HV Board Setting ======================================
    pHVBL.pHV[0].nHV_Board_Version=0;
    pHVBL.pHV[0].nVoltage[0]=0;
    pHVBL.pHV[0].nVoltage[1]=6;
    pHVBL.pHV[0].nVoltage[2]=11;
    pHVBL.pHV[0].nVoltage[3]=15;
    pHVBL.pHV[0].nVoltage[4]=20;


    //===========================================================================
    int nRet = 1;
    int nHVBLCount = sizeof(HV_Board_List)/sizeof(HV_INFO);

    //=======================Here to Get HV_Board version=============================
    int nHV_Board_Version=0; 
    //===========================================================================

    int *nFind = new int[nDoCount];
    ZeroMemory(nFind,nDoCount*sizeof(int));
    //for(int i=0;i<nHVBLCount;i++){
    for(int j=0;j<nDoCount;j++){
        for(int k=0;k<5;k++){
            if(nVolt[j]==pHVBL.pHV[nHV_Board_Version].nVoltage[k]){
                nFind[j] = true;   
                break;
            }
        }
    }
    // }
    for(int i=0;i<nDoCount;i++){
        if(nFind[i]==FALSE){
            nRet = -1;
            break;
        }
    }
    if(nFind) delete [] nFind;
    return nRet;
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
#elif IC_Module==IC_Module_A8011	
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

int SwitchTouchLink3HubOn()
{
    return 1;
    int rt;
    UCHAR   buf[8]={0};
    int     *ptr = NULL;
    int     ver=0;
    rt = GetI2CStatus_Bulk(buf);
    if(rt<0)         return 0;        
    ver = (buf[2]>>5);      
    if(ver<=2)    return 1;

    ver = (buf[2]&0x1F)*100+buf[3];      
    if(ver<107)  return 1;

    BOOL fSwithCmd=false;
    const int WBufferNum=64;
    unsigned char WBuffer[WBufferNum]={0};
    const int RBufferNum=64;
    unsigned char RBuffer[RBufferNum]={0};
    memset(WBuffer,0,WBufferNum);
    WBuffer[0] = 0x6F;
    WBuffer[1] = 0x02;
    WBuffer[2] = 0x00;
    WBuffer[3] = 0x0A; //Hub Setting. (Only valid in Touch Link-3.)    
    DWORD dwStartTickCount=GetTickCount();
    DWORD dwEndTickCount=0;
    for(int i=0;i<100;i++){
        WBuffer[0] = 0x60;
        rt = USBComm_WriteToBulkEx(WBuffer,WBufferNum);
        if(rt<0){
            break;
        }else{
            rt = USBComm_ReadFromBulkEx(RBuffer,RBufferNum);
            if(rt<0){
                break;
            }
        }
    }
    dwEndTickCount=GetTickCount();
    if(rt>0){
        int nFrameRate =  (WBufferNum*100)/(dwEndTickCount-dwStartTickCount);               
        if(nFrameRate<50){
            WBuffer[4] = 0x02;
            fSwithCmd=true;
        }
    }
    if(fSwithCmd){
        WBuffer[0] = 0x6F;
        WBuffer[1] = 0x02;
        WBuffer[2] = 0x00;
        WBuffer[3] = 0x0A; //Hub Setting. (Only valid in Touch Link-3.)    
        USBComm_WriteToBulkEx(WBuffer,64);
    }
    return 1;
    //
}
/* 
enum{
TL3INFO_ERROR = -1;
TL3INFO_STATUS_60 = 1;
TL3INFO_STATUS2_5F = 2;    
}
enum{
TL3INFOHUB_DEFAULT = 0;
TL3INFOHUB_NORMAL = 0x020000;
TL3INFOHUB_BYPASS = 0x030000;    
}
enum{
TL3INFOMICROSWITCH_CONDUCTING = 0;
TL3INFOMICROSWITCH_Open = 0x0100;
}

*/
int GetTouchLink3SupoortHubStatus()
{
    return 1;
    int rt;
    UCHAR   buf[8]={0};
    int     *ptr = NULL;
    int     ver=0;
    rt = GetI2CStatus_Bulk(buf);
    if(rt<0)         return TL3INFO_ERROR;        
    ver = (buf[2]>>5);      
    if(ver<=2)    return TL3INFO_ERROR;

    int ret=0;
    int nRet=TL3INFO_ERROR;
    CString  strTemp,strLog;
    BOOL fRet = true;
    const int WBufferNum=64;
    unsigned char WBuffer[WBufferNum]={0};
    const int RBufferNum=64;
    unsigned char RBuffer[RBufferNum]={0};
    memset(RBuffer,0,RBufferNum);
    memset(WBuffer,0,WBufferNum);
    WBuffer[0] = 0x5F;
    ret = USBComm_WriteToBulkEx(WBuffer,64);
    if(ret>0){
        ret = USBComm_ReadFromBulkEx(RBuffer,RBufferNum);
        if(RBuffer[0]==0xF1&& RBuffer[1]<0x17){
            WBuffer[0] = 0x60;
            ret = USBComm_WriteToBulkEx(WBuffer,64);
            ret = USBComm_ReadFromBulkEx(RBuffer,RBufferNum);
            if(ret>0){
                if(RBuffer[0]==0xF0&& RBuffer[1]<=0x05){
                    nRet = TL3INFO_STATUS_OLDFW ;
                }else if(RBuffer[0]==0xF0){
                    nRet = (RBuffer[0x09]<<16)+TL3INFO_STATUS_60;
                }
                /*
                DataBuf[5] = Hub Status.
                0x00: Default. (Reserved)
                0X01: Hub Cannot Control.
                0x02: Normal Mode.
                0x03: Bypass Mode.
                */
            }
        }else{
            nRet = (RBuffer[0x19]<<16)+(RBuffer[0x1A]<<8)+TL3INFO_STATUS2_5F;
            /*
            DataBuf[0x19] = Hub Status. 
            0x00: Default. (Reserved)
            0X01: Hub Cannot Control.
            0x02: Normal Mode.
            0x03: Bypass Mode.
            DataBuf[0x1A] = MicroSwitch Status.
            0x00: MicroSwitch Conducting.
            0X01: MicroSwitch Open.
            */
        }
    }  

    return nRet;
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
void SetSPIDelaySpecialTimeDefaultValue(BYTE pDelay1,BYTE pDelay2)
{
    DEFAULTDELAY1 = pDelay1;
    DEFAULTDELAY2 = pDelay2;
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
    fSPI_I2CMode=fEnable;
    return true;
}


int Get_I2C_Short_Status()
{
#ifdef _PID6101
#elif  _PID6102
#elif  _PID6103
#elif  _PID6104
#elif  _PID6105
#elif  _PID6106
#elif  _PID6107
#elif  _PID6108
    return true;
#endif // _PID_6111
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
    nTouchLinkSWVersion = (SBuff[2]&0x1F)*100+SBuff[3];      
    nTouchLinkHWVersion =(SBuff[2]>>5);       
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
#ifdef _PID6101
#elif  _PID6102
#elif  _PID6103
#elif  _PID6104
#elif  _PID6105
#elif  _PID6106
#elif  _PID6107
#elif  _PID6108
    return true;
#endif // _PID_6111
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
    nTouchLinkSWVersion = (SBuff[2]&0x1F)*100+SBuff[3];      
    nTouchLinkHWVersion =(SBuff[2]>>5);       
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
        ReadI2CReg_2Bytes_ST1801(pOutData,0x200,0x100);
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
                ReadI2CReg_2Bytes_ST1801(pOutData,0x200,0x100);
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
            ReadI2CReg_2Bytes_ST1801(pOutData,0x200,0x100);
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
bool SetA8015ISP_Mode(BOOL fEnable)
{
    fA8015_ISP_Mode=fEnable;
    return true;
}
bool Set_MTP_Mode(BOOL fEnable)
{
    fMTP_Mode=fEnable;
    return true;
}

bool SetST1802_WriteFlashHW_No_Read(BOOL fNoRead)
{
    fWriteflashNoRead = fNoRead;
     return true;
}