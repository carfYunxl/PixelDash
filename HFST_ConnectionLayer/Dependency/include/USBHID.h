#ifndef USBHID_H
#define USBHID_H

#include "stdafx.h"

extern "C"
{
    ///////////////////////////////////////
    //      USB HID API
    ///////////////////////////////////////

	enum {
		ERRORMSG_HID_DLL_TTK_ERR                =-1000,
		ERRORMSG_LOAD_HID_DLL_ERR                =-1001,
		ERRORMSG_GET_HID_DLL_PROCESS_ERR         =-1002

    };

    int InitUSBHID(const char *path);

	bool ConnectHID(int vid, int pid);

    extern HINSTANCE g_hInst_USBHID_DLL;   // handle of USBHID.dll

    typedef int (*HID_OpenHID_FUNC)(int vid,int pid);
    extern HID_OpenHID_FUNC       OpenHID;    // OpenHID
	typedef void (*HID_CloseHID_FUNC)();
    extern HID_CloseHID_FUNC       CloseHID;    // CloseHID
	typedef bool (*HID_WriteHID_FUNC)(unsigned char *wBuf);
    extern HID_WriteHID_FUNC       WriteHID;    // WriteHID
 
}


#endif
/*
OpenHID();
			unsigned char wBuf[9]={0};
			wBuf[1]=0xAD;
			wBuf[2]=0x01;
			WriteHID(wBuf);
			DestroyWindow(hWnd);

Linker : Input : hid.lib setupapi.lib
*/
