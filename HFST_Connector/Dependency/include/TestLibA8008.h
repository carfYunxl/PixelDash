#ifndef _TESTLIBA8008_H
#define _TESTLIBA8008_H

#include "TPTST6.h" 

enum{
    MEM,
    SFR,
    ROM,
    DR_MEM,
    DR_REG
};

//................struct of FH raw comp............................
#define    TPCOMP6_MAX_FREQ_STAGE   10
#define    TPCOMP6_DC_COMP_SIZE				36
#pragma pack(push,1)
//#pragma pack(show)

typedef struct {
    unsigned char UpBlock_Count;
    short UpBlock_CompStart;
    short UpBlock_CompRxStep;
    short UpBlock_CompTxStep;
    unsigned char LowBlock_Count;
    short LowBlock_CompStart;
    short LowBlock_CompRxStep;
    short LowBlock_CompTxStep;
}TPCOMP6_FREQHOP_COMP_PARAM;

typedef struct{
    TPCOMP6_FREQHOP_COMP_PARAM Freq[TPCOMP6_MAX_FREQ_STAGE];
}TPCOMP6_FREQHOP_OUT_PARAM;

typedef struct{
    unsigned char DistCompSlop;
    char DistCompOffset;
    char DistComp[TPCOMP6_DC_COMP_SIZE];
}TPCOMP6_DISTCOMP_OUT_PARAM;

#pragma pack(pop)
//#pragma pack(show)
//................struct of FH raw comp............................

int CallBack_ReadI2cReg(unsigned short RegAddr, unsigned short Len, unsigned char *pDataBuf);
int CallBack_SleepMs(unsigned int Millisecond);
int CallBack_WriteI2cReg(unsigned short RegAddr, unsigned short Len, unsigned char *pDataBuf);

int A8008TestLibInit();
int A8008TestLibFinish();

extern "C"
{

int UnloadTestLibApi(void);

int InitTestLibApi(const char *path);

extern HINSTANCE g_hInst_TestLibDLL;   // handle of TestLib.dll

typedef int (*TPTST6_TpTestLibInitFunc)(TPTST6_CB_FUNC *CbFuncs, unsigned char PageNumber);
extern TPTST6_TpTestLibInitFunc   TPTST6_TpTestLibInit;

typedef int (*TPTST6_TpTestLibFinishFunc)();
extern TPTST6_TpTestLibFinishFunc TPTST6_TpTestLibFinish;

typedef int (*TPTST6_GetVersionFunc)(unsigned char *pLibVer);
extern TPTST6_GetVersionFunc   TPTST6_GetVersion;

typedef int (*TPTST6_Host2DeviceCmdFunc)(unsigned char CmdId, unsigned char DataSize, unsigned char *pOutDataBuf);
extern TPTST6_Host2DeviceCmdFunc   TPTST6_Host2DeviceCmd;

typedef int (*TPTST6_Device2HostCmdFunc)(unsigned char *pCmdId, unsigned char *pDataSize, unsigned char *pInDataBuf);
extern TPTST6_Device2HostCmdFunc    TPTST6_Device2HostCmd;

typedef int (*TPTST6_GetSensingRawDataFunc)(unsigned char *pRawDataOutputPacket);
extern TPTST6_GetSensingRawDataFunc   TPTST6_GetSensingRawData;

typedef int (*TPTST6_PowerDownWakeupDeviceFunc)(unsigned char DelayMs);
extern TPTST6_PowerDownWakeupDeviceFunc    TPTST6_PowerDownWakeupDevice;

typedef unsigned char *(*TPTST6_GetErrStrFunc)();
extern TPTST6_GetErrStrFunc    TPTST6_GetErrStr;

typedef int (*TPTST6_ReadDeviceMemFunc)(unsigned char Type, unsigned char *pStartAddr, unsigned short Length, unsigned char *DataBuf);
extern TPTST6_ReadDeviceMemFunc    TPTST6_ReadDeviceMem;

typedef int (*TPTST6_WriteDeviceMemFunc)(unsigned char Type, unsigned char *pStartAddr, unsigned short Length, unsigned char *DataBuf);
extern TPTST6_WriteDeviceMemFunc    TPTST6_WriteDeviceMem;

typedef int (*TPTST6_ReadDevParamFunc)(unsigned char *pDevParamTab, unsigned char *pCtlParamInRam);
extern TPTST6_ReadDevParamFunc    TPTST6_ReadDevParam;

typedef int (*TPTST6_ReadFwTabFunc)(unsigned char *pFwTab);
extern TPTST6_ReadFwTabFunc    TPTST6_ReadFwTab;

typedef int (*TPTST6_WriteFwTabFunc)(unsigned char *pFwTab);
extern TPTST6_WriteFwTabFunc    TPTST6_WriteFwTab;

typedef int (*TPTST6_SetScanCtlFunc)(unsigned char ScanCtl);
extern TPTST6_SetScanCtlFunc    TPTST6_SetScanCtl;

typedef int (*TPTST6_ReadFwRxParamTblFunc)(unsigned char *pRxParamTbl);
extern TPTST6_ReadFwRxParamTblFunc    TPTST6_ReadFwRxParamTbl;

typedef int (*TPTST6_WriteFwRxParamTblFunc)(unsigned char * pRxParamTbl);
extern TPTST6_WriteFwRxParamTblFunc    TPTST6_WriteFwRxParamTbl;

typedef bool (*GetTouchCanvasZSupportFunc)();
extern GetTouchCanvasZSupportFunc    GetTouchCanvasZSupport;

typedef int (*GetTouchCanvasFocalKeyPosFunc)(unsigned char *pKeyTbl);
extern GetTouchCanvasFocalKeyPosFunc    GetTouchCanvasFocalKeyPos;

typedef int (*Get_XYKey_NumFunc)(unsigned char *pXYKeyNum);
extern Get_XYKey_NumFunc   Get_XYKey_Num;

typedef int (*Get_ResFunc)(unsigned char *pXYRes);
extern Get_ResFunc   Get_Res;

//======TPComp related function======
typedef int (*TCompInitialFunc)(int NumRx, int NumTx);
extern TCompInitialFunc TComp_Initial;

typedef int (*TCompFinishFunc)();
extern TCompFinishFunc TComp_Finish;

typedef int (*TPComp6_CollectFHCompDataFunc)(int FreqIndex, short *AvgRawData);
extern TPComp6_CollectFHCompDataFunc TPComp6_CollectFHCompData;


typedef int (*GenFHCompFunc)(int NumRx, int NumTx, int tpidx, int* freqNum, void* FHCompParam, char* errStr);
extern GenFHCompFunc   GenFHComp;

typedef int (*GenFHRawCompTabFunc)(void *pFHCompParam, int NumFHCompParam,
                             unsigned char *NumTxArea1, unsigned char *RawCompHigh, unsigned char *CompScale,
                             unsigned char *FreqCompParam, char* errStr);
extern GenFHRawCompTabFunc   GenFHRawCompTab;

//Generate distance compensation data for 0, 20, 12V
typedef int (*GenDistCompFunc)(int NumRx, int NumTx, int volIndex, int freqId, char* errStr);
extern GenDistCompFunc GenDistComp;

//Calculate distance compensation data for each TP
typedef int (*CalDistCompFunc)(void* outParam, int tpid, char* errStr);
extern CalDistCompFunc CalDistComp;

//Generate distance compensation Tab
typedef int (*GenDistCompTabFunc)(void *pDCParam, int NumDCParam, unsigned char *DistCompSlop, 
                                 char *DistCompOffset, unsigned char DistComp[18]);
extern GenDistCompTabFunc GenDistCompTab;
//======TPComp related function======
}
#endif