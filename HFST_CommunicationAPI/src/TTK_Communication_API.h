#ifndef __TTK_Communication_API__
#define __TTK_Communication_API__

#define TTK_Communication_API __declspec(dllexport)
extern "C"
{
	enum
	{
		ERRORMSG_DATA_NULL							= -3, 
		ERRORMSG_LENGTH_TOO_LONG					= -4, 
		ERRORMSG_ATOM_READ_FLASH_FAIL				= -5,
		ERRORMSG_ATOM_WRITE_FLASH_FAIL				= -6, 
		ERRORMSG_ATOM_ERASE_FLASH_FAIL				= -7,
		ERRORMSG_NON_COMPLETE_TRANSFER				= -8, 
		ERRORMSG_READ_I2CREG_FAIL					= -9, 
		ERRORMSG_WRITE_I2CREG_FAIL					= -10, 
		ERRORMSG_READ_BLUK_FAIL						= -11, 
		ERRORMSG_WRITE_BLUK_FAIL					= -12, 
		ERRORMSG_BRIDGE_STATUS_NAK					= -13, 
		ERRORMSG_BRIDGE_STATUS_TIMEOUT				= -14, 
		ERRORMSG_BRIDGE_STATUS_ARB_Loss				= -15,
		ERRORMSG_BRIDGE_STATUS_BUS_FAULT			= -16, 
		ERRORMSG_BRIDGE_STATUS_ERROR				= -17,
		ERRORMSG_READ_PACKET_ERROR					= -18, 
		ERRORMSG_TIMEOUT							= -19, 
		ERRORMSG_RETRY_TIMEOUT						= -20,
		ERRORMSG_ICP_READ_COMPARE_ERROR				= -21, 
		ERRORMSG_USB_RETRY_TIMEOUT_ERROR			= -22,
		ERRORMSG_BRIDGE_STATUS_ERROR_VDDGND_Short	= -23, 
		ERRORMSG_GetTP_Trim_Error					= -24,
		ERRORMSG_CheckSum_Error						= -25, 
		ERRORMSG_TP_Not_Connect						= -26, 
		ERRORMSG_Not_Touch_Panel_Detect_Mode_Status = -27,
		ERRORMSG_BRIDGE_STATUS_SCANNING				= -28, 
		ERRORMSG_P2P3_Short							= -29, 
		ERRORMSG_FLASH_UNLOCK						= -30,
		ERRORMSG_FLASH_LOCK							= -31,
		
		ERRORMSG_ADB_FILE_NOT_EXISTS				= -50, 
		ERRORMSG_ADB_FILE_OPEN_FILE					= -51,
		ERRORMSG_ADB_READ_LENGTH_ERROR				= -52, 
		ERRORMSG_ADB_WRITE_TIMEOUT					= -53, 
		ERRORMSG_ADB_READ_TIMEOUT					= -54
	};

	// HID error code, include detail error code in 'HFST_HID_SDK' 
	// if code = -1001, -1000 mean 'HID Open error', 1 mean 'not find device'
	// if code = -1102, -1100 mean 'HID Read error', 2 mean 'time out'
	enum {
		ERRORMSG_PROTOCOL_NOT_SUPPORT = -999,
		ERRORMSG_HID_OPEN = -1000,

		ERRORMSG_HID_READ = -1100,
		ERRORMSG_HID_READ_DEVICE_NOT_OPEN,

		ERRORMSG_HID_WRITE = -1200,
		ERRORMSG_HID_WRITE_DEVICE_NOT_OPEN
	};

	enum  //transmitter
	{
		Bridge_T_HWISP = 0x10, Bridge_T_ICP = 0x11, SWISP_Operation = 0x12, Bridge_T_Get_Status2 = 0x5F, Bridge_T_Get_Status = 0x60,
		Bridge_T_ResetBridge = 0x61, Bridge_T_I2C_Addr = 0x62, Bridge_T_Reset_Touch = 0x63, Bridge_T_Trim_Clock
		= 0x64, Bridge_T_Pulse_Ctl = 0x65, Bridge_T_System_OP = 0x6F, Bridge_T_Read_Reg = 0x70,
		Bridge_T_Write_Reg = 0x71, Bridge_T_Write_CMD = 0x72, Bridge_T_Read_CMD = 0x73, Bridge_T_WriteRead_CMD
		= 0x74, Bridge_T_Mode_SEL = 0x7E, Bridge_T_Continuous = 0x7F, Bridge_SPI_Write_Command_Packet = 0x75, Bridge_SPI_Read_Command_Packet = 0x76,
		Bridge_SPI_AvdRead_Command_Packet = 0x77, Bridge_Read_SPI_AvdRead_Command_Packet = 0x79, Bridge_T_Write_CMD_A8018 = 0x18
	};

	enum  // ICP sub command
	{
		ICP_INIT = 0, ICP_ERASE = 1, ICP_WRITE = 2, ICP_READ = 3, ICP_RESET = 4,
	};

	enum  // ICP flash type
	{
		ICP_APROM = 1, ICP_DATA_FLASH = 2, ICP_LDROM = 3, ICP_CONFIG = 4,
	};

	enum  // ICP address define
	{
		ICP_APROM_ADDR = 0, ICP_DATA_FLASH_ADDR = 0x1F000, ICP_LDROM_ADDR = 0x100000, ICP_CONFIG_ADDR = 0x300000,
	};

	enum  // ICP Flash Size
	{
		ICP_APROM_SIZE = 0x8000, ICP_DATA_FLASH_SIZE = 0x1000, ICP_LDROM_SIZE = 0x1000, ICP_CONFIG_SIZE = 0x12,
	};
	enum  // MTP command
	{
		I2C_ICP_MTP_ENTER = 0x0B, I2C_ICP_MTP_CHIP_ERASE = 0x0C, I2C_ICP_MTP_SECTOR_ERASE = 0x0D, I2C_ICP_MTP_WRITE = 0x0E,
		I2C_ICP_MTP_READ = 0x0F, I2C_SPI_MTP_SWITCH = 0x10,
	};
	enum  //reciver
	{
		Bridge_R_Data = 0x80, Bridge_R_HWISP_Data = 0x81, Bridge_R_I2C_Data = 0x82, Bridge_R_Trim_Clock_Data = 0x83,
		Bridge_R_System_Response = 0x8F, Bridge_R_Status = 0xF0, Bridge_R_Continuous = 0xFF,
		Bridge_R_INT_Detect_Data_Packet = 0x84, Bridge_R_SPI_Data_Packet = 0x85, Bridge_R_INT_Detect_Data_Packet_Mode2 = 0x87,
		Bridge_R_Current = 0x89
	};

	enum  //Bridge Status
	{
		Bridge_Status_Running = 0x00, Bridge_Status_Initialization = 0x01, Bridge_Status_Ready = 0x02,
		Bridge_Status_Error = 0x03, BridgeStatus_TouchPanel_Detect_Mode = 0x06,//Bridge_R_Status_Scanning=0x06,
	};

	enum  //I2C Status
	{
		I2C_Status_ACK = 0x00, I2C_Status_Initialization = 0x01, I2C_Status_NAK = 0x02, I2C_Status_Timeout = 0x03,
		I2C_Status_Arbitration_Loss = 0x04, I2C_Status_Bus_Fault = 0x05,
	};

	enum  //Error Code
	{
		Error_Bulk_Length = 0x00, Error_First_Packet = 0x01, Error_Continuous_Packet = 0x02, Error_Unexpected =
		0x03, Error_UnknownCmd = 0x04, Error_ReadTimeout = 0x05, Error_VddGndShort = 0x06,
	};

	enum  //HWISP
	{	/*  0->Erase;
		1->Write;
		2->Read;
		3->Mass Erase;
		4->WriteByChip;
		5->WriteBySection.
		*/
		HWISP_Erase = 0x00, HWISP_Write = 0x01, HWISP_Read = 0x02,
		HWISP_Mass_Erase = 0x03, HWISP_WriteByChip = 0x04, HWISP_WriteBySection = 0x05,
		HWISP_ICP_Interface_Switch = 0x08,
	};

	//for Power Noise Control
	enum {
		PWM_Control = 1,
		GPIO_Control,
	};
	enum {
		PWM_0 = 0,
		PWM_1 = 1,
		PWM_2 = 2,
		PWM_3 = 3,
		PWM_4 = 4,
		PWM_5 = 5,
		PWM_6 = 6,
		PWM_7 = 7,
	};
	enum {
		OneShotMode = 0x00,
		AutoReloadMode,
	};
	enum {
		InverterOff = 0x00,
		InverterOn,
	};
	enum {
		GPIO_Open = 0,
		GPIO_Close,
		GPIO_Set,
		GPIO_Clear,
		GPIO_Get,
	};
	enum {
		PORT_A = 0,
		PORT_B,
		PORT_C,
		PORT_D,
		PORT_E,
	};
	enum {
		GPIO_Mode_Input = 0,
		GPIO_Mode_Output,
		GPIO_Mode_OpenDrain,
		GPIO_Mode_Quasi,

	};
	//~for Power Noise Control
	enum {
		ST1801_SPI_ERASE_4K = 0x20,
		ST1801_SPI_ERASE_32K = 0x52,
		ST1801_SPI_ERASE_64K = 0xD8,
		ST1801_SPI_ERASE_CHIP = 0x60,
	};

	//for ADB SW ISP R/W
	enum {
		ADB_SW_ISP_READ = 0x0b,
		ADB_SW_ISP_WRITE = 0x0a
	};

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

	typedef struct _TouchLinkVersion_ {
		unsigned char    hw;        // TL4 & TL5 & TL6...
		unsigned char    hw_sub;    // 1 a, 2 b 
		unsigned char    sw_major;  // V1
		unsigned char    sw_sub;    // .02
	} TouchLinkVersion;

#define IC_Module_A8008		6
#define IC_Module_A8010		10
#define IC_Module_ST1801		8
#define IC_Module_ST1802		0x0C
#define IC_Module_A8015		0x0F
#define IC_Module_A8018		0x10

#define FLASH_SIZE_NONE 0
#define FLASH_SIZE_64K  1
#define FLASH_SIZE_48K  2
#define FLASH_SIZE_32K  3

#define FLASH_TYPE_17K 0
#define FLASH_TYPE_64K 1
#define FLASH_TYPE_256Bytes 2
#define FLASH_TYPE_32K 3

#ifdef _PRJ_A8008_
#define Module_A8008 
#endif // _PRJ_A8008_

#ifdef _PRJ_A8010_
#define Module_A8010 
#endif // _PRJ_A8010_

#ifdef _PRJ_ST1801_
#define Module_ST1801 
#endif // _PRJ_ST1801_

#ifdef _PRJ_ST1802_
#define Module_ST1802 
#endif // _PRJ_ST1802_

#ifdef _PRJ_A8015_
#define Module_A8015 
#endif // _PRJ_A8015_

#ifdef _PRJ_A8018
#define Module_A8018
#endif

#ifdef Module_A8008	// A8008
#define FLASH_SIZE FLASH_TYPE_17K
#define FLASH_TYPE FLASH_TYPE_17K
#define IC_Module	IC_Module_A8008 
#endif 
#ifdef Module_A8015	// A8008
#define FLASH_SIZE FLASH_TYPE_32K
#define FLASH_TYPE FLASH_TYPE_32K
#define IC_Module	IC_Module_A8015 
#endif 

#ifdef Module_A8010	// A8010
#define FLASH_SIZE FLASH_SIZE_64K
#define FLASH_TYPE FLASH_TYPE_64K
#define IC_Module	IC_Module_A8010
#endif 

#ifdef Module_ST1801	// A8010
#define FLASH_SIZE FLASH_SIZE_48K
#define FLASH_TYPE FLASH_TYPE_256Bytes
#define IC_Module	IC_Module_ST1801
#endif 

#ifdef Module_ST1802	//
#define FLASH_SIZE FLASH_SIZE_48K
#define FLASH_TYPE FLASH_TYPE_256Bytes
#define IC_Module	IC_Module_ST1802
#endif 

#ifdef Module_A8018
#define FLASH_SIZE FLASH_SIZE_48K
#define FLASH_TYPE FLASH_TYPE_256Bytes
#define IC_Module	IC_Module_A8018
#endif
	//======== IC Module End ==============

#ifdef _USRDLL
#define EXPORT_API  __declspec(dllexport)
#else
#define EXPORT_API  __declspec(dllimport)
#endif

#define BulkLen         (64)

	// Support hid for touch pad
	// James 2020/11/02
	bool EXPORT_API CommunicationModeSelect( int cm ); // 0-Bulk 1-Adb 2-HidOverI2C
	bool EXPORT_API HID_Init( int nProductId, int nVenorId, int nReportId, int nUsagePage = -1 );
	void EXPORT_API HID_UnInit();
	EXPORT_API char * HID_GetLastErrorMessage();
	int  EXPORT_API HID_GetLastErrorCode();
	int  EXPORT_API HID_ReadRawdata( unsigned char * rawdata, int nReadSize );	// direct send read command, fw return data from 0x40
	// end

	// Support adb communication
	/*
	 * @brief Open ADB shell to allow communication and alloc resource
	 * @param [in]  bAdbRunAsRoot   Determine whether run ADB services as root
	 * @param [in]	pAdbFilePath	Absolute adb.exe file full path
	 * @param [in]  pExtraService   Extra user define service except open ADB shell
	 * @note  -1. Usually this interface only open ADB shell run "adb.exe shell"
	 *        -2. If bAdbRunAsRoot is true, it will run "adb.exe root" as first command, such as run "getevent -l" command first must have root authority
	 *        -3. If pExtraService is assign, after "adb.exe shell" it will run pExtraService command like "adb.exe pExtraService"
	 *            such as there is a user-defined service need run
	 */
	int EXPORT_API ADB_Init( bool bAdbRunAsRoot, const char * pAdbFilePath, const char * pExtraService );
	
	/*
	 * @brief Notify server automatic record raw data file
	 * @param [in] ctx	user define what kink of data to read
	 */
	int EXPORT_API ADB_StartRecordRawData( FM_ReadRawdata * ctx );

	/*
	 * @brief Notify server stop record raw data 
	 * @param [in] pAdbFilePath	Absolute adb.exe file full path
	 * @param [in] pSavePath	The path of pc that save server temporary record raw data file
	 * @note after stop record raw data, we must pull raw data file to pc and modify format if necessary
	 */
	int EXPORT_API ADB_StopRecordRawData( const char * pAdbFilePath, const char * pSavePath );

	/*
	 * @brief Release ADB resource 
	 */
	int EXPORT_API ADB_UnInit();

	int EXPORT_API WriteUsbAutoRetry(unsigned char *data, unsigned int ilen, unsigned int &rtlen);

    bool EXPORT_API ExWriteFlashPage_Bulk(unsigned short Addr, unsigned char *Data);
    bool EXPORT_API ExReadFlashPage_Bulk(unsigned short Addr, unsigned char *Data);
    bool EXPORT_API ExReadFlashPage_Bulk_SPI_ST1802(int Addr,unsigned char *Data);
    
//**************  HERE	//****************************/
	bool EXPORT_API ExReadFlashPage_Bulk_SPI_ST1801(unsigned short Addr,unsigned char *Data);
    bool EXPORT_API ExEraseFlashPage_Bulk(unsigned short  Addr);

	bool EXPORT_API MassEraseFlash_Bulk( void );
    bool EXPORT_API ReadFWVersion_SW(float *Ver);
	bool EXPORT_API ReadChecksum_SW(unsigned int Length, unsigned int *Checksum);
	bool EXPORT_API Jump_SWISP(void);
	bool EXPORT_API JumpBack_SW(void);
	bool EXPORT_API GetResponse_SW(unsigned short *Checksum);

	bool EXPORT_API ReadDataFlashBlock_SW(unsigned int Addr, unsigned char *Data);

	int EXPORT_API Read_Packet_Bulk(unsigned char *RBuffer, unsigned short RLength);
	int EXPORT_API Read_Packet_INT_Bulk(unsigned char *RBuffer, unsigned short RLength);
	int EXPORT_API Read_TouchPanelDetectStatus();      

	//---------------------------TTK communication API------------------------------
	//mode 0: Jump Bulk
	//mode 1: Jump HID
	//-----------------------------------------------------------------
	int EXPORT_API SwitchBridge(unsigned char mode);
	int EXPORT_API SetLED_Bulk(unsigned char LEDSetting,BOOL fFlashMode=false,BYTE pPeriod=0);
	int EXPORT_API GetLED_Bulk(unsigned char *LEDSetting);
	int EXPORT_API SetButton_Bulk(unsigned char ButtonSetting);
	int EXPORT_API GetButton_Bulk(unsigned char *ButtonSetting);

    int EXPORT_API WriteRegTL_Bulk(unsigned int Addr, unsigned int Data);
    int EXPORT_API ReadRegTL_Bulk(unsigned int Addr, unsigned int *Data);
    //int __fastcall ExWriteFlashHW(unsigned char *Data, unsigned int Addr ,unsigned int Len);

    int EXPORT_API EraseFlashHW(unsigned int Addr, unsigned int Len);
    int EXPORT_API ST1802EraseFlashHW(unsigned int Addr , unsigned int Len);
    int EXPORT_API ReadFlashHW(unsigned char *Data, unsigned int Addr, unsigned int Len);
    int EXPORT_API WriteFlashHW(unsigned char *Data, unsigned int Addr, unsigned int Len);
    int EXPORT_API WriteFlashHW_NoErase(unsigned char *Data, unsigned int Addr, unsigned int Len);
    int EXPORT_API WriteFlashSW_NoErase(unsigned char *Data, unsigned int Addr ,unsigned int Len);
    int EXPORT_API WriteFlashHW_ST1802_SPI_NoReadCheck(unsigned char *Data, unsigned int Addr ,unsigned int Len);
    int EXPORT_API EraseFlashSW(unsigned int Addr, unsigned int Len);
    int EXPORT_API ReadFlashSW(unsigned char *Data, unsigned int Addr, unsigned int Len);
    int EXPORT_API WriteFlashSW(unsigned char *Data, unsigned int Addr, unsigned int Len);
	int EXPORT_API GetFlashProgress();
	int EXPORT_API HFST_A8018_SDK_WriteURAMSwitchToPRAM( unsigned char * pWriteBuf, unsigned int nWriteSize );

	int EXPORT_API ReadI2CReg(unsigned char *data, unsigned int addr, unsigned int len);
	int EXPORT_API WriteI2CReg(unsigned char *data, unsigned int addr, unsigned int len);

	int EXPORT_API ResetTP(void);
	int EXPORT_API ResetBridge(void);
	int EXPORT_API SetI2CAddr(unsigned char Addr,unsigned char RetryTime,unsigned char NonClockStretchFlag);
	int EXPORT_API ReadUsb(unsigned char *data, unsigned int ilen, unsigned int &rtlen);
	int EXPORT_API WriteUsb(unsigned char *data, unsigned int ilen, unsigned int &rtlen);

	int EXPORT_API WriteCmd(unsigned char *WBuffer, unsigned short WLength);
	int EXPORT_API WriteCmd_Advancd(unsigned char *WBuffer, unsigned short WLength);
	int EXPORT_API ReadCmd(unsigned char *RBuffer, unsigned short RLength);
	int EXPORT_API ReadCmd_Advance(unsigned char *RBuffer, unsigned short RLength);
	int EXPORT_API WriteReadCmd(unsigned char *WBuffer, unsigned short WLength, unsigned short RLength);

	int EXPORT_API GetDllVersion(void);
	int EXPORT_API GetI2CStatus_Bulk(unsigned char *SBuff);
	int EXPORT_API GetTouchLinkVersion( TouchLinkVersion * ver );

	int EXPORT_API ReadI2CRegBeginINT(unsigned char Addr, unsigned char length);
	int EXPORT_API ReadI2CRegDataINT( unsigned char * Data, unsigned char length );
	int EXPORT_API ReadI2CRegEndINT(void);
	double EXPORT_API ReadTPCurrent_TL3();
    double EXPORT_API ReadTPCurrentAVG();
	int EXPORT_API ReadTPVoltage( double * pVDD1, double * pVDD2 );
	int EXPORT_API SWISPReset(void);
    void EXPORT_API SetProtocolC(BOOL fEnableProtocolC);
    void EXPORT_API SetProtocoSPI(BOOL fEnableSPI);
    void EXPORT_API SetSPIDelaySpecialTime(BOOL fSpecialTime,BYTE pDelay1,BYTE pDelay2);
    bool EXPORT_API NotifyHWICP_ChangeMode(BOOL fSetSPIMode);
	//For INT Test A8008
	int EXPORT_API BridgeModeSelect(unsigned char Addr, unsigned char length, int nMode);
	int EXPORT_API BridgeModeSelect_Multi_Bytes(unsigned char *Cmd,unsigned char length,int nMode);
	int EXPORT_API Read_Write_INT_Flag_CMD_Packet(unsigned char Detect, unsigned char Flag);
	int EXPORT_API PowerNoiseControl(BYTE byVolt, DWORD wFrequency);
	int EXPORT_API PowerNoiseControlLED(BOOL fInitial,BOOL fBlueLight,BOOL fGreenLight);
	int EXPORT_API SetTouchLink3_Voltage(double dVDD1,double dVDD2);
	int EXPORT_API SetTouchLink3_SleepMs(BYTE pSleepMs);

	/*
	 * @brief	Set Touch Link VDD1 & VDD2 Voltage
	 * @param	[in]	nVDD1	VDD1 Voltage(unit: mV), 1600 ~ 3600
	 * @param	[in]	nVDD2	VDD2 Voltage(unit: mV), 1600 ~ 3600
	 * @return	
	 *		- 1 is SUCCESS
	 *		- otherwise FAIL, ref ERRORMSG_XXX
	 * @note
	 *		- Only support Touch Link 6 and later
	 */
	int EXPORT_API SetTouchLinkVoltage( unsigned short nVDD1, unsigned short nVDD2 );
	int EXPORT_API GetTPICVersion();
	int EXPORT_API GetHWICVersion();
	int EXPORT_API GetTPISPVersion();
   
    int EXPORT_API DoPowerNoiseSendCMD(BYTE *pCMD,BYTE pLength);
    int EXPORT_API EraseFlashHW_ST1801_ChipErase();
    int EXPORT_API WriteFlashHW_ST1801_SPI_MassErase(unsigned char *Data, unsigned int Addr ,unsigned int Len);
    int EXPORT_API WriteFlashSW_SPI_ST1801_MassErase(unsigned char *Data, unsigned int Addr ,unsigned int Len);
    bool EXPORT_API ST1801_6MHz(int nClk);
    bool EXPORT_API SetSPI_I2CMode(BOOL fEnable);
    bool EXPORT_API SetA8015ISP_Mode(BOOL fEnable); 
    bool EXPORT_API SetST1802_WriteFlashHW_No_Read(BOOL fNoRead);    
    int  EXPORT_API Get_I2C_Short_Status();
    int  EXPORT_API Set_I2C_Gearingp_Mode(BOOL fGearingMode);
    bool EXPORT_API ST1801_WriteCommandIO(unsigned char *pInData,unsigned long Len,BOOL fUse16Bit=false);
    bool EXPORT_API ST1801_ReadCommandIO(unsigned char *pInData,int nCmdLen,unsigned char *pOutData,BOOL fUse16Bit=false);
    bool EXPORT_API Chip_Erase_ST1801_SW_ISP();
	bool EXPORT_API HFST_CheckIsA8018ICPI2CAddr(unsigned char i2cAddr);

    // ---------------------------------------------------------------------------
    // James 2020/08/21 for multi device, valid id is 1 ~ 8
    void EXPORT_API SetMultiDeviceID(int deviceID);
	
    // James 2020/07/24 For ST1802 Search Tool
    int EXPORT_API SetI2C_Clock( int nKHz );
	int EXPORT_API Get_I2C_Clock_KHz();
    int EXPORT_API ReadCommandIOData(unsigned char cmdType, unsigned short nAddr, unsigned char * pReadData, unsigned short nReadSize);
    int EXPORT_API WriteCommandIOData(unsigned char cmdType, unsigned short nAddr, unsigned char * pWriteData, unsigned short nWriteSize);
    int EXPORT_API GetSensingRawData(unsigned char * pDataBuffer, unsigned int pDataBufferSize);
    int EXPORT_API WaitI2CReady();

	/**
	 * Start fast read raw data mode, in this mode, TouchLink will read data according to user 
	 * defined in FM_ReadRawdata, and PC direct read data from TouchLink without send cmd
	 * 
	 * @param	[in]	ctx		user define what kink of data to read
	 */
	int EXPORT_API StartFastReadRawdataMode( FM_ReadRawdata * ctx );

	/**
	 * Stop fast read raw data mode
	 */
	int EXPORT_API StopFastReadRawdataMode();

	// A8018 Add
	/**
	 * Only valid for A8018 HW Word Write information block
	 * @param [in]	data	the data write to flash
	 * @param [in]	addr	information block address for word write, must even address
	 * @param [in]	len		the data size of 'data', must even length
	 * @return 1 PASS < 0 FAIL
	 */
	int EXPORT_API WriteFlashHW_WR( unsigned char * data, unsigned int addr, unsigned int len );

	int EXPORT_API A8018HW_FlashLock();
	int EXPORT_API A8018HW_FlashUnLock();

	enum { ICP_WRITE_RAM = 0x00, ICP_WRITE_SFR = 0x01, ICP_READ_RAM = 0x80, ICP_READ_SFR = 0x81 };
	int EXPORT_API A8018HW_ReadRAM( unsigned short nReadAddr, unsigned char * pReadBuf, unsigned int nReadSize, unsigned char nReadType = ICP_READ_RAM );
	int EXPORT_API A8018HW_WriteRAM( unsigned int nWriteAddr, const unsigned char * pDataBuf, unsigned int nDataSize, unsigned char nWriteType = ICP_WRITE_RAM, BOOL bPARAM = FALSE );
}
#endif

