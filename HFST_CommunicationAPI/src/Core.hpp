#ifndef __TTK_COMMUNICATION_CORE_HPP__
#define __TTK_COMMUNICATION_CORE_HPP__

#include <vector>

#ifdef _USRDLL
#define EXPORT_API  __declspec(dllexport)
#else
#define EXPORT_API  __declspec(dllimport)
#endif

enum
{
	ERRORMSG_DATA_NULL = -3,
	ERRORMSG_LENGTH_TOO_LONG = -4,
	ERRORMSG_ATOM_READ_FLASH_FAIL = -5,
	ERRORMSG_ATOM_WRITE_FLASH_FAIL = -6,
	ERRORMSG_ATOM_ERASE_FLASH_FAIL = -7,
	ERRORMSG_NON_COMPLETE_TRANSFER = -8,
	ERRORMSG_READ_I2CREG_FAIL = -9,
	ERRORMSG_WRITE_I2CREG_FAIL = -10,
	ERRORMSG_READ_BLUK_FAIL = -11,
	ERRORMSG_WRITE_BLUK_FAIL = -12,
	ERRORMSG_BRIDGE_STATUS_NAK = -13,
	ERRORMSG_BRIDGE_STATUS_TIMEOUT = -14,
	ERRORMSG_BRIDGE_STATUS_ARB_Loss = -15,
	ERRORMSG_BRIDGE_STATUS_BUS_FAULT = -16,
	ERRORMSG_BRIDGE_STATUS_ERROR = -17,
	ERRORMSG_READ_PACKET_ERROR = -18,
	ERRORMSG_TIMEOUT = -19,
	ERRORMSG_RETRY_TIMEOUT = -20,
	ERRORMSG_ICP_READ_COMPARE_ERROR = -21,
	ERRORMSG_USB_RETRY_TIMEOUT_ERROR = -22,
	ERRORMSG_BRIDGE_STATUS_ERROR_VDDGND_Short = -23,
	ERRORMSG_GetTP_Trim_Error = -24,
	ERRORMSG_CheckSum_Error = -25,
	ERRORMSG_TP_Not_Connect = -26,
	ERRORMSG_Not_Touch_Panel_Detect_Mode_Status = -27,
	ERRORMSG_BRIDGE_STATUS_SCANNING = -28,
	ERRORMSG_P2P3_Short = -29,
	ERRORMSG_FLASH_UNLOCK = -30,
	ERRORMSG_FLASH_LOCK = -31,

	ERRORMSG_ADB_FILE_NOT_EXISTS = -50,
	ERRORMSG_ADB_FILE_OPEN_FILE = -51,
	ERRORMSG_ADB_READ_LENGTH_ERROR = -52,
	ERRORMSG_ADB_WRITE_TIMEOUT = -53,
	ERRORMSG_ADB_READ_TIMEOUT = -54
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
struct FM_RawdataType {
	unsigned char dataType;		///< what kind of raw data type that want TouchLink return
	unsigned char dataLength;	///< TouchLink read the data length of dataType 
	unsigned char dataNum;		///< TouchLink read 'dataNum' times of dataType
};

struct FM_ReadRawdata {
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
};

struct TouchLinkVersion {
	unsigned char    hw;        // TL4 & TL5 & TL6...
	unsigned char    hw_sub;    // 1 a, 2 b 
	unsigned char    sw_major;  // V1
	unsigned char    sw_sub;    // .02
};

#define IC_Module_A8008			6
#define IC_Module_A8010			10
#define IC_Module_ST1801		8
#define IC_Module_ST1802		0x0C
#define IC_Module_A8015			0x0F
#define IC_Module_A8018			0x10

#define FLASH_SIZE_NONE			0
#define FLASH_SIZE_64K			1
#define FLASH_SIZE_48K			2
#define FLASH_SIZE_32K			3

#define FLASH_TYPE_17K			0
#define FLASH_TYPE_64K			1
#define FLASH_TYPE_256Bytes		2
#define FLASH_TYPE_32K			3

#define BulkLen					(64)

#define Revision        (2)
#define Flash1k         (0x400)
#define Flash15k        (0x3C00)
#define Flash16k        (0x4000)
#define Flash30k        (0x7800)
#define Flash32k        (0x8000)
#define Flash33k        (0x8400)
#define Flash48k        (0xC000)
#define Flash60k        (0xF000)
#define Flash63k        (0xFC00)
#define Flash61k        (0xF400)
#define Flash64k        (0x10000)
#define Flash128k        (0x20000)
#define ReadTimeout     (20000)
#define RetryTimes      (3)
#define PageSize        (0x100)
#define PageSize512     (0x200)
#define PageSize1K      (0x400)
#define PageSize4K      (0x1000)
#define BlockSize       (0x200)
#define SWISPLength     (8)
#define PageSize_HID    (0xFC)//(0xFF)  

#define CRC8_POLY 0xD5
#define CRC16_POLY 0x1021
typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;

enum //SWISP
{
	SWISP_Flash_Advance = 0x12, // Advance for A8002

	SWISP_Response = 0x31,
	SWISP_Erase = 0x80,
	SWISP_Write_Data = 0x81,
	SWISP_Write_Flash = 0x82,
	SWISP_Read_Flash = 0x83,
	SWISP_Reset = 0x84,
	SWISP_Get_FWVer = 0x88, //for ST1572
	SWISP_Run_APROM = 0x89, //for ST1572
	SWISP_Get_Flash_Mode = 0x89, //for ST1572
	SWISP_Read_Checksum = 0x8B, //for ST1572
	SWISP_Boot_Sel = 0x8C, //for ST1572
	SWISP_Read_DataFlash = 0x8D, //for ST1572
	SWISP_UNLOCK = 0x87, //for ST1572
	SWISP_ReadSFR = 0x88,
	SWISP_Ready = 0x8F,
};

enum { //ST1801
	ST1801_SWISP_Unlock_Flash = 0x10,
	ST1801_SWISP_lock_Flash = 0x11,
	ST1801_SWISP_Chip_Erase = 0x12,
	ST1801_SWISP_Sector_Erase = 0x13,
	ST1801_SWISP_Page_Program = 0x14,
	ST1801_SWISP_Read_Flash = 0x15,
	ST1801_SWISP_32K_Erase = 0x16,
	ST1801_SWISP_Unlock_Flash_ISP = 0x90,
	ST1801_SWISP_Unlock_Boot_Loader = 0x1F,
};

#endif //__TTK_COMMUNICATION_CORE_HPP__
