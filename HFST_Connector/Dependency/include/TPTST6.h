#ifndef _TPTST6_H_
#define _TPTST6_H_

//=====================================================================================================
//
//				A8008 Touch Panel Test Library Specification V01.03.pdf
//
//=====================================================================================================
//=====================================================================================================
//
//			 	A8008 Development Application Note V03.01.pdf
//
//=====================================================================================================
#define	TPTST6_SUPORT_DEV_PARAMS_VER		0x03	//Current version = V0.3
#define	TPTST6_TESTLIB_COMPATIBLE_VER		0x01	//Current Compatible Version = V0.1
//The library release number is reset to 0 once FW Table version or Development Parameter Table version are changed
#define	TPTST6_LIB_RELEASE_VER				0x00	


//=====================================================================================================
//
//											Struct
//
//=====================================================================================================
typedef struct TPTST6_CallbackFunc
{
	int (*ReadI2cReg)(unsigned short RegAddr, unsigned short Len, unsigned char *pDataBuf);
	int (*WriteI2cReg)(unsigned short RegAddr, unsigned short Len, unsigned char *pDataBuf);
	int (*SleepMs)(unsigned int Millisecond);
}TPTST6_CB_FUNC;


#pragma pack(push, 1) //change packing to 1
//#pragma pack(show)

typedef struct TPTST6_DevParam
{
	unsigned char TabTag[4]; //"TDP0"
	unsigned char TabSize;
	unsigned char Version;
	unsigned char TpTestLibCompatibleVer;;
	unsigned short CtlParamRamAddr;
	unsigned short FwTabSize;
	unsigned char ScanNoiseNumTxCh;
}TPTST6_DEV_PARAM;


typedef struct TPTST6_CtlParamInRam
{
	unsigned char ScanCtl;
	unsigned int  FwTabAddr;
	unsigned char RawBufState[2];		//0 = Empty.	1 = Sensing.	2 = Full.
}TPTST6_CTL_PARAM_IN_RAM;

#pragma pack(pop)	//resotre packing
//#pragma pack(show)

#endif //end of _TPTST6_H_
