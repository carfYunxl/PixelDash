#ifndef TTK_Communication_APIH
#define TTK_Communication_APIH

#include <vector>
#include <windows.h>
extern "C"
{
    enum
    {
        ERRORMSG_DATA_NULL              =-3,
        ERRORMSG_LENGTH_TOO_LONG        =-4,
        ERRORMSG_ATOM_READ_FLASH_FAIL   =-5,
        ERRORMSG_ATOM_WRITE_FLASH_FAIL  =-6,
        ERRORMSG_ATOM_ERASE_FLASH_FAIL  =-7,
        ERRORMSG_NON_COMPLETE_TRANSFER  =-8,
        ERRORMSG_READ_I2CREG_FAIL       =-9,
        ERRORMSG_WRITE_I2CREG_FAIL      =-10,
        ERRORMSG_READ_BLUK_FAIL         =-11,
        ERRORMSG_WRITE_BLUK_FAIL        =-12,
        ERRORMSG_BRIDGE_STATUS_NAK      =-13,
        ERRORMSG_BRIDGE_STATUS_TIMEOUT  =-14,
        ERRORMSG_BRIDGE_STATUS_ARB_Loss =-15,
        ERRORMSG_BRIDGE_STATUS_BUS_FAULT=-16,
        ERRORMSG_BRIDGE_STATUS_ERROR    =-17,
        ERRORMSG_READ_PACKET_ERROR      =-18,
        ERRORMSG_TIMEOUT                =-19,
        ERRORMSG_RETRY_TIMEOUT          =-20,
        ERRORMSG_ICP_READ_COMPARE_ERROR =-21,
        ERRORMSG_USB_RETRY_TIMEOUT_ERROR=-22,
        ERRORMSG_BRIDGE_STATUS_ERROR_VDDGND_Short=-23,
        ERRORMSG_GetTP_Trim_Error=-24,
        ERRORMSG_CheckSum_Error =  - 25,
        ERRORMSG_TP_Not_Connect =  - 26,  
        ERRORMSG_Not_Touch_Panel_Detect_Mode_Status =  - 27,  
        ERRORMSG_BRIDGE_STATUS_SCANNING=-28,

        ERRORMSG_BULK_TTK_ERR                =-1000,
        ERRORMSG_LOAD_DLL_ERR                =-1001,
        ERRORMSG_GET_DLL_PROCESS_ERR         =-1002

    };

    enum
    {
	    FLASH_SIZE_NONE =0,
	    FLASH_SIZE_16K5 =1,
	    FLASH_SIZE_45K  =2,        
    };

    enum
    {
	    FLASH_TYPE_16K5 =0,
	    FLASH_TYPE_45K  =1,        
    };

    enum
    {
	    IC_TYPE_ST1564 =1,
	    IC_TYPE_ST1536 =2,        
    };


    enum //transmitter
    {
	    Bridge_T_HWISP              = 0x10,
	    Bridge_T_ICP                = 0x11,
	    Bridge_T_Get_Status         = 0x60,
	    Bridge_T_Reset              = 0x61,
	    Bridge_T_I2C_Addr           = 0x62,
	    Bridge_T_Reset_Touch        = 0x63,
	    Bridge_T_System_OP          = 0x6F,
	    Bridge_T_Read_Reg           = 0x70,
	    Bridge_T_Write_Reg          = 0x71,
	    Bridge_T_Write_CMD          = 0x72,
	    Bridge_T_Read_CMD           = 0x73,
	    Bridge_T_WriteRead_CMD      = 0x74,
	    Bridge_T_Mode_SEL           = 0x7E,
	    Bridge_T_Continuous         = 0x7F,
    };

    enum //reciver
    {
	    Bridge_R_Data         = 0x80,
	    Bridge_R_HWISP_Data   = 0x81,
	    Bridge_R_I2C_Data     = 0x82,
	    Bridge_R_Status       = 0xF0,
	    Bridge_R_Continuous   = 0xFF,
    };

    enum //Bridge Status
    {
	    Bridge_Status_Running             = 0x00,
	    Bridge_Status_Initialization      = 0x01,
	    Bridge_Status_Ready               = 0x02,
	    Bridge_Status_Error               = 0x03,
    };

    enum //I2C Status
    {
	    I2C_Status_ACK                 = 0x00,
	    I2C_Status_Initialization      = 0x01,
	    I2C_Status_NAK                 = 0x02,
	    I2C_Status_Timeout             = 0x03,
	    I2C_Status_Arbitration_Loss    = 0x04,
	    I2C_Status_Bus_Fault           = 0x05,
    };

    enum //Error Code
    {
	    Error_Bulk_Length                = 0x00,
	    Error_First_Packet               = 0x01,
	    Error_Continuous_Packet          = 0x02,
	    Error_Unexpected                 = 0x03,
	    Error_UnknownCmd                 = 0x04,
	    Error_ReadTimeout                = 0x05,
    };

    enum //HWISP
    {
	    HWISP_Erase         = 0x00,
	    HWISP_Write         = 0x01,
	    HWISP_Read          = 0x02,
	    HWISP_Mass_Erase    = 0x03,
    };
    //for Power Noise Control
    enum{
	    PWM_Control=1,
	    GPIO_Control,
    };
    enum{
	    PWM_0=0,
	    PWM_1=1,
	    PWM_2=2,
	    PWM_3=3,
	    PWM_4=4,
	    PWM_5=5,
	    PWM_6=6,
	    PWM_7=7,
    };
    enum{
	    OneShotMode=0x00,
	    AutoReloadMode,
    };
    enum{
	    InverterOff=0x00,
	    InverterOn,
    };
    enum{
	    GPIO_Open=0,
	    GPIO_Close,
	    GPIO_Set,
	    GPIO_Clear,
	    GPIO_Get,
    };
    enum{
	    PORT_A=0,
	    PORT_B,
	    PORT_C,
	    PORT_D,
	    PORT_E,
    };
    enum{
	    GPIO_Mode_Input=0,
	    GPIO_Mode_Output,
	    GPIO_Mode_OpenDrain,
	    GPIO_Mode_Quasi,
    };

    // James 2021/07/19 for support fast read raw data mode(FM: Fast Mode)
    typedef struct _FM_RawdataType_ {
        unsigned char dataType;		///< what kind of raw data type that want TouchLink return
        unsigned char dataLength;	///< TouchLink read the data length of dataType 
        unsigned char dataNum;		///< TouchLink read 'dataNum' times of dataType
    } FM_RawdataType;

    typedef struct _FM_ReadRawdata_ {
        unsigned char type;			///< bit0 ~ bit3: 0: I2C			1: SPI
                                    ///< bit4 ~ bit7: 0: Check Mode		1: Repeat Mode
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

    //~for Power Noise Control
#define FLASH_SIZE FLASH_SIZE_NONE
#define FLASH_TYPE FLASH_TYPE_16K5

#define BulkLen         (64)

#define IC_TYPE IC_TYPE_ST1536

    bool __declspec(dllexport)   WriteI2CReg_Bulk(unsigned char Addr,unsigned char Data);
    bool __declspec(dllexport)   ReadI2CReg_Bulk(unsigned char Addr,unsigned char *Data);

    bool __declspec(dllexport)   WriteFlashUnit_Bulk(unsigned short Addr,unsigned char *Data);
    bool __declspec(dllexport)   ReadFlashUnit_Bulk(unsigned short Addr,unsigned char *Data);
    bool __declspec(dllexport)   EraseFlashUnit_Bulk(unsigned short Addr);
    bool __declspec(dllexport)   ExWriteFlashPage_Bulk(unsigned short Addr,unsigned char *Data);
    bool __declspec(dllexport)   ExReadFlashPage_Bulk(unsigned short Addr,unsigned char *Data);
    bool __declspec(dllexport)   ExEraseFlashPage_Bulk(unsigned short Addr);
    bool __declspec(dllexport)   ReadFWVersion_SW(float *Ver);
    bool __declspec(dllexport)   ReadChecksum_SW(unsigned int Length,unsigned int *Checksum);
    bool __declspec(dllexport)   JumpBack_SW(void);
    bool __declspec(dllexport)   GetResponse_SW(unsigned short *Checksum);
    bool __declspec(dllexport)   EraseFlashBlock_SW(unsigned int Addr);
    bool __declspec(dllexport)   WriteFlashBlock_SW(unsigned int Addr,unsigned char *Data);
    bool __declspec(dllexport)   ReadFlashBlock_SW(unsigned int Addr,unsigned char *Data);

    bool __declspec(dllexport)   EraseFlashPage_SW(unsigned int Addr);
    bool __declspec(dllexport)   WriteFlashPage_SW(unsigned int Addr,unsigned char *Data);
    bool __declspec(dllexport)   ReadFlashPage_SW(unsigned int Addr,unsigned char *Data);


    bool __declspec(dllexport)   ReadDataFlashBlock_SW(unsigned int Addr,unsigned char *Data);
    int __declspec(dllexport)   SwitchBridge(unsigned char mode);
    int __declspec(dllexport)   SetButton_Bulk(unsigned char ButtonSetting);
    int __declspec(dllexport)   GetButton_Bulk(unsigned char *ButtonSetting);

    int __declspec(dllexport)   WriteRegTL_Bulk(unsigned int Addr, unsigned int Data);
    int __declspec(dllexport)   ReadRegTL_Bulk(unsigned int Addr, unsigned int *Data);

    int InitTTKCOMMAPI(const char *path);

    extern HINSTANCE g_hInst_TTK_COMM_API_DLL;   // handle of TTK_Communication_API.dll

    //! EraseFlashHW
    typedef int (*EraseFlashHW_FUNC)(unsigned int Addr , unsigned int Len);
    extern EraseFlashHW_FUNC       EraseFlashHW;

    //! MassEraseFlash_Bulk
    typedef bool (*MassEraseFlash_Bulk_FUNC)();
    extern MassEraseFlash_Bulk_FUNC       MassEraseFlash_Bulk;

    //! ReadFlashHW
    typedef int (*ReadFlashHW_FUNC)(unsigned char *Data, unsigned int Addr , unsigned int Len);
    extern ReadFlashHW_FUNC       ReadFlashHW;

    //! WriteFlashHW_NoErase
    typedef int (*WriteFlashHW_NoErase_FUNC)(unsigned char *Data, unsigned int Addr , unsigned int Len);
    extern WriteFlashHW_NoErase_FUNC       WriteFlashHW_NoErase;

    // WriteFlashHW
    typedef int (*WriteFlashHW_FUNC)(unsigned char *Data, unsigned int Addr , unsigned int Len);
    extern WriteFlashHW_FUNC       WriteFlashHW;

    //! EraseFlashSW
    typedef int (*EraseFlashSW_FUNC)(unsigned int Addr , unsigned int Len);
    extern EraseFlashSW_FUNC       EraseFlashSW;

    //! ReadFlashSW
    typedef int (*ReadFlashSW_FUNC)(unsigned char *Data, unsigned int Addr , unsigned int Len);
    extern ReadFlashSW_FUNC       ReadFlashSW;

    //! WriteFlashSW
    typedef int (*WriteFlashSW_FUNC)(unsigned char *Data, unsigned int Addr , unsigned int Len);
    extern WriteFlashSW_FUNC       WriteFlashSW;

    //! ReadI2CReg
    typedef int (*ReadI2CReg_FUNC)(unsigned char *data, unsigned int addr, unsigned int len);
    extern ReadI2CReg_FUNC       ReadI2CReg1;

    //! WriteI2CReg
    typedef int (*WriteI2CReg_FUNC)(unsigned char *data, unsigned int addr, unsigned int len);
    extern WriteI2CReg_FUNC       WriteI2CReg1;

    //! ResetTP
    typedef int (*ResetTP_FUNC)();
    extern ResetTP_FUNC       ResetTP;

    //! ResetBridge
    typedef int (*ResetBridge_FUNC)();
    extern ResetBridge_FUNC       ResetBridge;

    //! SetI2CAddr
    typedef int (*SetI2CAddr_FUNC)(unsigned char Addr,unsigned char RetryTime,unsigned char NonClockStretchFlag);
    extern SetI2CAddr_FUNC       SetI2CAddr1;

    //! ReadUsb
    typedef int (*ReadUsb_FUNC)(unsigned char *data, unsigned int ilen, unsigned int &rtlen);
    extern ReadUsb_FUNC       ReadUsb;

    //! WriteUsb
    typedef int (*WriteUsb_FUNC)(unsigned char *data, unsigned int ilen, unsigned int &rtlen);
    extern WriteUsb_FUNC       WriteUsb;

    //! WriteCmd
    typedef int (*WriteCmd_FUNC)(unsigned char *WBuffer, unsigned short WLength);
    extern WriteCmd_FUNC       WriteCmd;

    //! ReadCmd
    typedef int (*ReadCmd_FUNC)(unsigned char *RBuffer, unsigned short RLength);
    extern ReadCmd_FUNC       ReadCmd;

    //! WriteReadCmd
    typedef int (*WriteReadCmd_FUNC)(unsigned char *WBuffer, unsigned short WLength, unsigned short RLength);
    extern WriteReadCmd_FUNC       WriteReadCmd;

    //! GetDllVersion
    typedef int (*GetDllVersion_FUNC)();
    extern GetDllVersion_FUNC       GetDllVersion;

    //! ReadI2CRegBeginINT
    typedef int (*ReadI2CRegBeginINT_FUNC)(unsigned char Addr,unsigned char length);
    extern ReadI2CRegBeginINT_FUNC       ReadI2CRegBeginINT;

    //! ReadI2CRegDataINT
    typedef int (*ReadI2CRegDataINT_FUNC)(unsigned char *Data,unsigned char length);
    extern ReadI2CRegDataINT_FUNC       ReadI2CRegDataINT;

    //! ReadI2CRegEndINT
    typedef int (*ReadI2CRegEndINT_FUNC)();
    extern ReadI2CRegEndINT_FUNC       ReadI2CRegEndINT;

    //! Read_TouchPanelDetectStatus
    typedef int (*Read_TouchPanelDetectStatus_FUNC)();
    extern Read_TouchPanelDetectStatus_FUNC       Read_TouchPanelDetectStatus;

    //! GetI2CStatus_Bulk
    typedef int (*GetI2CStatus_Bulk_FUNC)(unsigned char *SBuff);
    extern GetI2CStatus_Bulk_FUNC       GetI2CStatus_Bulk;

    //! ReadI2CReg
    typedef double (*ReadTPCurrent_FUNC)();
    extern ReadTPCurrent_FUNC       ReadTPCurrent;

    //! ReadI2CReg
    typedef double (*ReadTPCurrent_TL3_FUNC)();
    extern ReadTPCurrent_TL3_FUNC       ReadTPCurrent_TL3;

    //! EraseFlashHW
    typedef bool (*Jump_SWISP_FUNC)();
    extern Jump_SWISP_FUNC       Jump_SWISP;

    //£¡ BridgeModeSelect
    typedef int (*BridgeModeSelect_FUNC)(unsigned char Addr,unsigned char length,int nMode);
    extern BridgeModeSelect_FUNC       BridgeModeSelect;

    //£¡ BridgeModeSelect_Multi_Bytes
    typedef int (*BridgeModeSelect_Multi_Bytes_FUNC)(unsigned char *Cmd,unsigned char length,int nMode);
    extern BridgeModeSelect_Multi_Bytes_FUNC       BridgeModeSelect_Multi_Bytes;

    //£¡ Read_Write_INT_Flag_CMD_Packet
    typedef int (*Read_Write_INT_Flag_CMD_Packet_FUNC)(unsigned char Detect,unsigned char Flag);
    extern Read_Write_INT_Flag_CMD_Packet_FUNC       Read_Write_INT_Flag_CMD_Packet;

    //£¡ ReadI2CReg
    typedef int (*PowerNoiseControl_FUNC)(BYTE byVolt,DWORD wFrequency);
    extern PowerNoiseControl_FUNC       PowerNoiseControl;

    //£¡ ReadI2CReg
    typedef int (*PowerNoiseControlLED_FUNC)(BOOL fInitial,BOOL fBlueLight,BOOL fGreenLight);
    extern PowerNoiseControlLED_FUNC       PowerNoiseControlLED;

    //£¡ ReadI2CReg
    typedef int (*Read_Packet_Bulk_FUNC)(unsigned char *RBuffer, unsigned short RLength);
    extern Read_Packet_Bulk_FUNC       Read_Packet_Bulk;

    //£¡ ReadI2CReg
    typedef int (*SearchHV_Board_Support_FUNC)(int nDoCount,int* nVolt);
    extern SearchHV_Board_Support_FUNC       SearchHV_Board_Support;

    //£¡ ReadI2CReg
    typedef int (*HV_Board_DoCmd_FUNC)(int nVolt,DWORD wFrequency);
    extern HV_Board_DoCmd_FUNC       HV_Board_DoCmd;

    //£¡ ReadI2CReg
    typedef int (*SetTouchLink3_Voltage_FUNC)(double dVDD1,double dVDD2);
    extern SetTouchLink3_Voltage_FUNC       SetTouchLink3_Voltage;

    //£¡ ReadI2CReg
    typedef int (*SetTouchLinkVoltage_FUNC)(unsigned short nVDD1, unsigned short nVDD2);
    extern SetTouchLinkVoltage_FUNC       SetTouchLinkVoltage;

    //£¡ for control touch-link 3 voltage
    typedef int (*SetTouchLink3_SleepMs_FUNC)(BYTE pSleepMs);
    extern SetTouchLink3_SleepMs_FUNC       SetTouchLink3_SleepMs;

    //£¡ ReadI2CReg
    typedef int (*GetTPICVersion_FUNC)();
    extern GetTPICVersion_FUNC       GetTPICVersion;

    //£¡ ReadI2CReg
    typedef int (*GetTPISPVersion_FUNC)();
    extern GetTPISPVersion_FUNC       GetTPISPVersion;

    //£¡ ReadI2CReg
	typedef int (*GetHWICVersion_FUNC)();
	extern GetHWICVersion_FUNC       GetHWICVersion;

    //£¡ ReadI2CReg
    typedef int (*DoPowerNoiseSendCMD_FUNC)(BYTE *pCMD,BYTE pLength);
    extern DoPowerNoiseSendCMD_FUNC       DoPowerNoiseSendCMD;

    //£¡ ReadI2CReg
    typedef int (*SetLED_Bulk_FUNC)(unsigned char LEDSetting,BOOL fFlashMode,BYTE pPeriod);
    extern SetLED_Bulk_FUNC       SetLED_Bulk;        // ReadI2CReg

    typedef int (*GetLED_Bulk_FUNC)(unsigned char *LEDSetting);
    extern GetLED_Bulk_FUNC       GetLED_Bulk;       

    typedef int (*SwitchTouchLink3HubOn_FUNC)();
    extern SwitchTouchLink3HubOn_FUNC       SwitchTouchLink3HubOn;       

    typedef int (*GetTouchLink3SupoortHubStatus_FUNC)();
    extern GetTouchLink3SupoortHubStatus_FUNC       GetTouchLink3SupoortHubStatus;        

	typedef int (*Read_Packet_INT_Bulk_FUNC)(unsigned char *RBuffer, unsigned short RLength);
	extern Read_Packet_INT_Bulk_FUNC       Read_Packet_INT_Bulk;
	
	//SPI command
	typedef void (*SetProtocoSPI_FUNC)(BOOL EnableSPI);
	extern SetProtocoSPI_FUNC       SetProtocoSPI;
	
	typedef void (*NotifyHWICP_ChangeMode_FUNC)(BOOL EnableSPI);
	extern NotifyHWICP_ChangeMode_FUNC       NotifyHWICP_ChangeMode;

	typedef void (*SetSPIDelaySpecialTime_FUNC)(BOOL fSpecialTime,BYTE pDelay1,BYTE pDelay2);
	extern SetSPIDelaySpecialTime_FUNC       SetSPIDelaySpecialTime;
	
    //SPI ISP
    typedef bool (*SetSPI_I2CMode_FUNC)(BOOL fEnable);
    extern SetSPI_I2CMode_FUNC       SetSPI_I2CMode;   

	//SPI clock
    typedef bool (*ST1801_6MHz_FUNC)(int nClk);
    extern ST1801_6MHz_FUNC       ST1801_6MHz;

    //MTP mode
    typedef bool (*SET_MTP_MODE_FUNC)(int nClk);
    extern SET_MTP_MODE_FUNC       Set_MTP_Mode;

    typedef int (*pfnStartFastReadRawdataMode)(FM_ReadRawdata * ctx);
    extern pfnStartFastReadRawdataMode SDK_StartFastReadRawdataMode;

	typedef int (*pfnStopFastReadRawdataMode)();
	extern pfnStopFastReadRawdataMode SDK_StopFastReadRawdataMode;

    // HID 
    enum CommuncationMode { BULK = 0, ADB, HID_OVER_I2C };
    typedef bool ( *pfnCommunicationModeSelect)(int cm);
    extern pfnCommunicationModeSelect   CommunicationModeSelect;

    typedef bool ( *pfnHID_Init)(int nProductId, int nVenorId, int nReportId, int nUsagePage);
    extern pfnHID_Init  HID_Init;

    typedef void ( *pfnHID_UnInit)();
    extern pfnHID_UnInit    HID_UnInit;

    typedef int  ( *pfnHID_GetLastErrorCode)();
    extern pfnHID_GetLastErrorCode  HID_GetLastErrorCode;

    typedef int  ( *pfnHID_ReadRawdata)(unsigned char * rawdata, int nReadSize);
    extern pfnHID_ReadRawdata  HID_ReadRawdata;

    typedef char * ( pfnHID_GetLastErrorMessage)();
    extern pfnHID_GetLastErrorMessage  HID_GetLastErrorMessage;

	typedef bool (*HFST_CheckIsA8018ICPI2CAddr)(unsigned char);
	extern HFST_CheckIsA8018ICPI2CAddr  CheckIsA8018ICPI2CAddr;

    // ADB
    typedef int (*pfnADB_Init)(bool bAdbRunAsRoot, const char * pAdbFilePath, const char * pExtraService);
    extern pfnADB_Init ADB_Init;

    typedef int (*pfnADB_UnInit)();
    extern pfnADB_UnInit ADB_UnInit;

    typedef int (*pfnADB_StartRecordRawData)(FM_ReadRawdata* ctx);
    extern pfnADB_StartRecordRawData ADB_StartRecordRawData;

    typedef int (*pfnADB_StopRecordRawData)(const char* pAdbFilePath, const char* pSavePath);
    extern pfnADB_StopRecordRawData ADB_StopRecordRawData;
}
int SetI2CAddr(unsigned char Addr,unsigned char pRetryTime=0xFF,BOOL bNonClockStretchFlag=false);

int ReadI2CReg(unsigned char* data, unsigned int addr, unsigned int len);
int WriteI2CReg(unsigned char* data, unsigned int addr, unsigned int len);

int SetProtocolSPI_Ex(BOOL EnableSPI);
int SetSPI_I2CMode_Ex(BOOL EnableSPI);
int NotifyHWICP_ChangeMode_Ex(BOOL EnableSPI);
int SetSPIDelaySpecialTime_Ex(BOOL fSpecialTime, BYTE pDelay1, BYTE pDelay2);
int SetTouchLinkI2cClock(int clk);
int SetTouchLinkSpiClock(int clk);
int GetTouchLinkI2cClock(void);
int GetTouchLinkSpiClock(void);
int ResetTPEx(void);

extern CRITICAL_SECTION g_CS_CMD_PROTECT;
extern int g_TTK_CurBusType;
extern bool g_NonClockStretchFlag;

extern unsigned char gTLHwVer;
extern unsigned char gTLFwVerH;
extern unsigned char gTLFwVerL;


int ReadI2CRegBeginINTEx(int beginAddr, int len);
int ReadI2CRegDataINTEx(unsigned char* buf, int len);
int ReadI2CRegEndINTEx(void);

bool IsTouchLinkGreThanEquTo(unsigned char targetHwVer, unsigned char targetFwVerH, unsigned char targetFwVerL);
extern int gI2CRegAdd;
extern int gI2CReadLen;
extern int gI2cClk;
extern int gSpiClk;

int DoSwitchToDebugBridge();
int SwitchToTTKBridge();

int PM_WriteCodeSPI(int addr, unsigned char* data, int len);
int PM_ReadCodeSPI(int addr, unsigned char* data, int len);


int PM_Finish1SPI();
int PM_Finish2SPI();

int PM_WriteCodeI2C(int addr, unsigned char* data, int len);
int PM_ReadCodeI2C(int addr, unsigned char* data, int len);

BOOL WaitTouchLink3_VoltageStable(double dVDD, double dIOVDD);

//BULK OUT
#define TL_BULK_HW_RESET                0x63
#define TL_BULK_I2C_WRITE_CMD           0x72
#define TL_BULK_I2C_READ_AFT_WRITE_CMD  0x74
#define TL_BULK_SPI_READ_ALIGN_CMD      0x79
#define TL_BULK_SPI_WRITE_ALIGN_CMD     0x7A
#define TL_BULK_CONT_CMD                0x7F

//BULK IN
#define TL_BULK_I2C_READ_PKT            0x82
#define TL_BULK_SPI_READ_PKT            0x88
#define TL_BULK_CONT_PKT                0xFF

//BULK ERROR CODES
#define ERRORMSG_BULK_WRITE_FAIL        -3
#define ERRORMSG_BULK_READ_FAIL         -4
#define ERRORMSG_SPI_WRITE_FAIL         -6
#define ERRORMSG_SPI_READ_FAIL          -7

//
#define MAX_SPI_TRANSFER_SIZE           0x1000      //0x7F6
#define MAX_SPI_TRANSFER_SIZE_W         0x1000      //0x7F6

int TouchLinkSpiWrite(unsigned char* writeData, int writeLen, bool enDelay);
int TouchLinkSpiRead(unsigned char* readData, int readLen, unsigned char* transferData, int transferLen, bool enDelay);

int TouchLinkToggleReset(int period, int delay);

void ResetTimeLog();
void PrintTimeLog();

// James 2019/10/15
BOOL JumpToSitronixFW();
#endif



