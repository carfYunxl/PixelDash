
#ifndef __BULKDLL_H__
#define __BULKDLL_H__

#include "stdafx.h"

#define BULKDLL_API __declspec(dllexport)
/*
extern "C"
{


              BULKDLL_API int USBComm_FinishEx();

       BULKDLL_API int USBComm_InitEx(unsigned int TimeOut);

       //BULKDLL_API int USBComm_InitEx();

       BULKDLL_API int USBComm_CheckBulkInEx();

       BULKDLL_API int USBComm_CheckBulkOutEx();

       BULKDLL_API int USBComm_ReadFromBulkEx(unsigned char *Buffer, unsigned int BufferSize);

       BULKDLL_API int USBComm_WriteToBulkEx(unsigned char *Buffer, unsigned int BufferSize);



}
  */
extern "C"
{

    enum
    {
        ERRORMSG_BULK_DLL_TTK_ERR                =-1000,
        ERRORMSG_LOAD_BULK_DLL_ERR                =-1001,
        ERRORMSG_GET_BULK_DLL_PROCESS_ERR         =-1002

    };

    ///////////////////////////////////////
    //      TTK COMMUNICATION APIlkjhdxscf vb
    ///////////////////////////////////////

    int InitBulkDllAPI(const char *path);

	bool ConnectBulk();

    extern HINSTANCE g_hInst_Bulk_DLL;   // handle of BulkDll.dll

    typedef int (*USBComm_FinishEx_FUNC)();
    extern USBComm_FinishEx_FUNC       USBComm_FinishEx;    // USBComm_FinishEx
    typedef int (*USBComm_InitEx_FUNC)(unsigned int TimeOut, unsigned int nDeviceIdx);
    extern USBComm_InitEx_FUNC       USBComm_InitEx;      // USBComm_InitEx
    typedef int (*USBComm_CheckBulkInEx_FUNC)();
    extern USBComm_CheckBulkInEx_FUNC       USBComm_CheckBulkInEx;    // USBComm_CheckBulkInEx
    typedef int (*USBComm_CheckBulkOutEx_FUNC)();
    extern USBComm_CheckBulkOutEx_FUNC       USBComm_CheckBulkOutEx;    // USBComm_CheckBulkOutEx
    typedef int (*USBComm_ReadFromBulkEx_FUNC)(unsigned char *Buffer, unsigned int BufferSize);
    extern USBComm_ReadFromBulkEx_FUNC       USBComm_ReadFromBulkEx;    // USBComm_ReadFromBulkEx
    typedef int (*USBComm_WriteToBulkEx_FUNC)(unsigned char *Buffer, unsigned int BufferSize);
    extern USBComm_WriteToBulkEx_FUNC       USBComm_WriteToBulkEx;    // USBComm_WriteToBulkEx
}

#endif // BULK.dll
