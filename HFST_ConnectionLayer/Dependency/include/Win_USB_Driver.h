#ifndef WIN_USB_DRIVER_H_
#define WIN_USB_DRIVER_H_
#include "stdafx.h"
enum 
{
	ERRORMSG_Win_USB_Driver_DLL_ERR = -1005,
	ERRORMSG_LOAD_Win_USB_Driver_DLL_ERR = -1006,
	ERRORMSG_GET_Win_USB_Driver_DLL_PROCESS_ERR = -1007

};

extern HINSTANCE g_hInst_Win_USB_Driver_DLL;   // handle of Win_USB_Driver.dll

int Init_WIN_USB_Driver(const CString& path);

typedef int (*HFST_WinUSB_Driver_CheckIsConnected)(int);
extern HFST_WinUSB_Driver_CheckIsConnected       WinUSB_Driver_CheckIsConnected;    // WinUSB_Driver_CheckIsConnected

typedef int (*HFST_WinUSB_Driver_Install)(int);
extern HFST_WinUSB_Driver_Install       WinUSB_Driver_Install;    // WinUSB_Driver_Install

typedef void (*HFST_WinUSB_Driver_RemoveFiles)();
extern HFST_WinUSB_Driver_RemoveFiles       WinUSB_Driver_RemoveFiles;    // WinUSB_Driver_RemoveFiles


#endif //WIN_USB_DRIVER_H_