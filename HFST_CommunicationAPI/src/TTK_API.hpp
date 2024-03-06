#ifndef __TTK_API_HPP__
#define __TTK_API_HPP__

#include "Core.hpp"

enum class CHIP_ID
{
	A8008 = 0x06,
	A8010 = 0x0A,
	A8015 = 0x0F,
	A2152 = 0x0C,
	A8018 = 0x10
};

enum class CommunicationMode {
	CommunicationMode_TouchLink_Bulk = 0,	// default bulk mode
	CommunicationMode_Phone_ADB,		    // phone adb communication 
	CommunicationMode_HID_OverI2C			// only support ST1802 for touch pad
};

struct TTK_HFST_HID_CONTEXT {
	bool bSupportVariableSize;
	int nHidReadMaxSize;
	int nHidWriteMaxSize;
	HANDLE hHidHandle;
	int nHidReportId;
};

constexpr unsigned short MAX_ADB_PKT_SIZE = 0xFF;
constexpr char ADB_CLIENT_CMD[]{ "/data/httu/sitronix_client " };	// transmit data through this interface

enum {
	ICP_WRITE_RAM = 0x00,
	ICP_WRITE_SFR = 0x01,
	ICP_READ_RAM = 0x80, 
	ICP_READ_SFR = 0x81
};

class EXPORT_API TTK_Communication_API
{
public:
	explicit TTK_Communication_API(CHIP_ID id);

public:
	//**************** Support hid for touch pad ****************//
	bool	CommunicationModeSelect(int cm); // 0-Bulk 1-Adb 2-HidOverI2C
	bool	HID_Init(int nProductId, int nVenorId, int nReportId, int nUsagePage = -1);
	void	HID_UnInit();
	char*	HID_GetLastErrorMessage();
	int		HID_GetLastErrorCode();
	int		HID_ReadRawdata(unsigned char* rawdata, int nReadSize);	// direct send read command, fw return data from 0x40

	//**************** For ADB ****************//
public:
	int ADB_Init(bool bAdbRunAsRoot, const char* pAdbFilePath, const char* pExtraService);
	int ADB_StartRecordRawData(FM_ReadRawdata* ctx);
	int ADB_StopRecordRawData(const char* pAdbFilePath, const char* pSavePath);
	int ADB_UnInit();
private:
	int			ReadPackage_ADB(unsigned char* data, unsigned int len);
	int			ReadFlash_ADB_Private(unsigned char* data, unsigned int addr, unsigned int len);
	int			WriteFlash_ADB_Private(unsigned char* data, unsigned int addr, unsigned int len);
	std::string ConvertADB_CMD(const std::vector< unsigned char >& data);
	int			ReadFlashSW_ADB(unsigned char* data, unsigned int addr, unsigned int len);
	int			WriteFlash_ADB(unsigned char* data, unsigned int addr, unsigned int len);
	int			ReadI2CReg_ProtocolA_ADB(unsigned char* data, unsigned int addr, unsigned int len);
	int			WriteI2CReg_ProtocolA_ADB(unsigned char* data, unsigned int addr, unsigned int len);
	int			WriteCmd_ADB(unsigned char* data, unsigned short len);
	int			ReadCmd_ADB(unsigned char* data, unsigned short len);
	int			ChecksumCalculation(unsigned short* pChecksum, unsigned char* pInData, unsigned long nLen);

	//**************** For All *****************//
	int  WriteUsbAutoRetry(unsigned char* data, unsigned int ilen, unsigned int& rtlen);
	bool ExWriteFlashPage_Bulk(unsigned short Addr, unsigned char* Data);
	bool ExReadFlashPage_Bulk(unsigned short Addr, unsigned char* Data);
	bool ExReadFlashPage_Bulk_SPI_ST1802(int Addr, unsigned char* Data);
	bool ExReadFlashPage_Bulk_SPI_ST1801(unsigned short Addr, unsigned char* Data);
	bool ExEraseFlashPage_Bulk(unsigned short  Addr);
	bool MassEraseFlash_Bulk(void);
	bool ReadFWVersion_SW(float* Ver);
	bool ReadChecksum_SW(unsigned int Length, unsigned int* Checksum);
	bool Jump_SWISP(void);
	bool JumpBack_SW(void);
	bool GetResponse_SW(unsigned short* Checksum);
	bool ReadDataFlashBlock_SW(unsigned int Addr, unsigned char* Data);
	int  Read_Packet_Bulk(unsigned char* RBuffer, unsigned short RLength);
	int  Read_Packet_INT_Bulk(unsigned char* RBuffer, unsigned short RLength);
	int  Read_TouchPanelDetectStatus();
	//---------------------------TTK communication API------------------------------
	//mode 0: Jump Bulk
	//mode 1: Jump HID
	//-----------------------------------------------------------------
	int SwitchBridge(unsigned char mode);
	int SetLED_Bulk(unsigned char LEDSetting, BOOL fFlashMode = false, BYTE pPeriod = 0);
	int GetLED_Bulk(unsigned char* LEDSetting);
	int SetButton_Bulk(unsigned char ButtonSetting);
	int GetButton_Bulk(unsigned char* ButtonSetting);
	int WriteRegTL_Bulk(unsigned int Addr, unsigned int Data);
	int ReadRegTL_Bulk(unsigned int Addr, unsigned int* Data);
	//int __fastcall ExWriteFlashHW(unsigned char *Data, unsigned int Addr ,unsigned int Len);

	int EraseFlashHW(unsigned int Addr, unsigned int Len);
	int ST1802EraseFlashHW(unsigned int Addr, unsigned int Len);
	int ReadFlashHW(unsigned char* Data, unsigned int Addr, unsigned int Len);
	int WriteFlashHW(unsigned char* Data, unsigned int Addr, unsigned int Len);
	int WriteFlashHW_NoErase(unsigned char* Data, unsigned int Addr, unsigned int Len);
	int WriteFlashSW_NoErase(unsigned char* Data, unsigned int Addr, unsigned int Len);
	int WriteFlashHW_ST1802_SPI_NoReadCheck(unsigned char* Data, unsigned int Addr, unsigned int Len);
	int EraseFlashSW(unsigned int Addr, unsigned int Len);
	int ReadFlashSW(unsigned char* Data, unsigned int Addr, unsigned int Len);
	int WriteFlashSW(unsigned char* Data, unsigned int Addr, unsigned int Len);
	int GetFlashProgress();
	int HFST_A8018_SDK_WriteURAMSwitchToPRAM(unsigned char* pWriteBuf, unsigned int nWriteSize);
	int ReadI2CReg(unsigned char* data, unsigned int addr, unsigned int len);
	int WriteI2CReg(unsigned char* data, unsigned int addr, unsigned int len);
	int ResetTP(void);
	int ResetBridge(void);
	int SetI2CAddr(unsigned char Addr, unsigned char RetryTime, unsigned char NonClockStretchFlag);
	int ReadUsb(unsigned char* data, unsigned int ilen, unsigned int& rtlen);
	int WriteUsb(unsigned char* data, unsigned int ilen, unsigned int& rtlen);
	int WriteCmd(unsigned char* WBuffer, unsigned short WLength);
	int WriteCmd_Advancd(unsigned char* WBuffer, unsigned short WLength);
	int ReadCmd(unsigned char* RBuffer, unsigned short RLength);
	int ReadCmd_Advance(unsigned char* RBuffer, unsigned short RLength);
	int WriteReadCmd(unsigned char* WBuffer, unsigned short WLength, unsigned short RLength);
	int GetDllVersion(void);
	int GetI2CStatus_Bulk(unsigned char* SBuff);
	int GetTouchLinkVersion(TouchLinkVersion* ver);
	int ReadI2CRegBeginINT(unsigned char Addr, unsigned char length);
	int ReadI2CRegDataINT(unsigned char* Data, unsigned char length);
	int ReadI2CRegEndINT(void);
	double ReadTPCurrent_TL3();
	double ReadTPCurrentAVG();
	int  ReadTPVoltage(double* pVDD1, double* pVDD2);
	int  SWISPReset(void);
	void SetProtocolC(BOOL fEnableProtocolC);
	void SetProtocoSPI(BOOL fEnableSPI);
	void SetSPIDelaySpecialTime(BOOL fSpecialTime, BYTE pDelay1, BYTE pDelay2);
	bool NotifyHWICP_ChangeMode(BOOL fSetSPIMode);
	//For INT Test A8008
	int BridgeModeSelect(unsigned char Addr, unsigned char length, int nMode);
	int BridgeModeSelect_Multi_Bytes(unsigned char* Cmd, unsigned char length, int nMode);
	int Read_Write_INT_Flag_CMD_Packet(unsigned char Detect, unsigned char Flag);
	int PowerNoiseControl(BYTE byVolt, DWORD wFrequency);
	int PowerNoiseControlLED(BOOL fInitial, BOOL fBlueLight, BOOL fGreenLight);
	int SetTouchLink3_Voltage(double dVDD1, double dVDD2);
	int SetTouchLink3_SleepMs(BYTE pSleepMs);

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
	int SetTouchLinkVoltage(unsigned short nVDD1, unsigned short nVDD2);
	int GetTPICVersion();
	int GetHWICVersion();
	int GetTPISPVersion();

	int DoPowerNoiseSendCMD(BYTE* pCMD, BYTE pLength);
	int EraseFlashHW_ST1801_ChipErase();
	int WriteFlashHW_ST1801_SPI_MassErase(unsigned char* Data, unsigned int Addr, unsigned int Len);
	int WriteFlashSW_SPI_ST1801_MassErase(unsigned char* Data, unsigned int Addr, unsigned int Len);
	bool ST1801_6MHz(int nClk);
	bool SetSPI_I2CMode(BOOL fEnable);
	bool SetA8015ISP_Mode(BOOL fEnable);
	bool SetST1802_WriteFlashHW_No_Read(BOOL fNoRead);
	int  Get_I2C_Short_Status();
	int  Set_I2C_Gearingp_Mode(BOOL fGearingMode);
	bool ST1801_WriteCommandIO(unsigned char* pInData, unsigned long Len, BOOL fUse16Bit = false);
	bool ST1801_ReadCommandIO(unsigned char* pInData, int nCmdLen, unsigned char* pOutData, BOOL fUse16Bit = false);
	bool Chip_Erase_ST1801_SW_ISP();
	bool HFST_CheckIsA8018ICPI2CAddr(unsigned char i2cAddr);

	// ---------------------------------------------------------------------------
	// James 2020/08/21 for multi device, valid id is 1 ~ 8
	void SetMultiDeviceID(int deviceID);

	// James 2020/07/24 For ST1802 Search Tool
	int SetI2C_Clock(int nKHz);
	int Get_I2C_Clock_KHz();
	int ReadCommandIOData(unsigned char cmdType, unsigned short nAddr, unsigned char* pReadData, unsigned short nReadSize);
	int WriteCommandIOData(unsigned char cmdType, unsigned short nAddr, unsigned char* pWriteData, unsigned short nWriteSize);
	int GetSensingRawData(unsigned char* pDataBuffer, unsigned int pDataBufferSize);
	int WaitI2CReady();

	/**
	 * Start fast read raw data mode, in this mode, TouchLink will read data according to user
	 * defined in FM_ReadRawdata, and PC direct read data from TouchLink without send cmd
	 *
	 * @param	[in]	ctx		user define what kink of data to read
	 */
	int StartFastReadRawdataMode(FM_ReadRawdata* ctx);

	/**
	 * Stop fast read raw data mode
	 */
	int StopFastReadRawdataMode();

	// A8018 Add
	/**
	 * Only valid for A8018 HW Word Write information block
	 * @param [in]	data	the data write to flash
	 * @param [in]	addr	information block address for word write, must even address
	 * @param [in]	len		the data size of 'data', must even length
	 * @return 1 PASS < 0 FAIL
	 */
	int WriteFlashHW_WR(unsigned char* data, unsigned int addr, unsigned int len);
	int A8018HW_FlashLock();
	int A8018HW_FlashUnLock();
	int A8018HW_ReadRAM(unsigned short nReadAddr, unsigned char* pReadBuf, unsigned int nReadSize, unsigned char nReadType = ICP_READ_RAM);
	int A8018HW_WriteRAM(unsigned int nWriteAddr, const unsigned char* pDataBuf, unsigned int nDataSize, unsigned char nWriteType = ICP_WRITE_RAM, BOOL bPARAM = FALSE);

private:
	void SleepInProgram(int nSleep);
	bool ExWriteFlashPage_Bulk_A8010(unsigned short Addr, unsigned char* Data);
	bool ExWriteFlashPage_Bulk_ST1801(unsigned short Addr, unsigned char* Data);
	bool CheckST1801_SPI_FLASH_Busy();
private:
	CHIP_ID		m_Chipid;

	//**** For HID ****//
	int		m_HID_Progress{0};
	int		m_HID_LastError{0};
	char	m_HID_ErrorBuffer[MAX_PATH]{0};
	CommunicationMode	m_CommunicationMode{ CommunicationMode::CommunicationMode_TouchLink_Bulk };
	TTK_HFST_HID_CONTEXT m_HID_Context;

	//**** For ADB ****//
	HANDLE	m_hAdbContext{ nullptr };

	//**** For All ****//
	BOOL fUseNewINTProtocol;

	int nSleepTime = 1;
	int nSPIClock = 6;
	unsigned char g_sI2cAddr = 0;
	constexpr unsigned int g_CRC24_PAGE_1K_FF = 0xCB0117;

	TouchLinkVersion g_TouchLinkVersion;
	BYTE DEFAULTDELAY2 = 0x0D;
	BYTE DELAY1 = DEFAULTDELAY1;//0x05;   //0x14
	BYTE DELAY2 = DEFAULTDELAY2;//0x0D;
	BOOL fSPI_ISPMode = false;

	BOOL fProtocolC = false;
	BOOL fSPIMode = false;
	BYTE pFlashWritePageSize = 0xFF;
	//int pFlashErasePageSize = 4096;
	BOOL fWriteflashNoRead = false;
	static int nMultiDeviceID = -1;
};

#endif //__TTK_API_HPP__
