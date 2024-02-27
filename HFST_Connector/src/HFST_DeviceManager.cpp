#include "pch.h"
#include "HFST_DeviceManager.hpp"

#pragma comment (lib, "Setupapi.lib")

namespace HFST
{
	USB_Manager::USB_Manager()
		: m_hUSBDevNotify(NULL)
	{
	}

    void USB_Manager::Register()
    {
        
    }

	int USB_Manager::DetectUSBConnectCount() const
	{
		HDEVINFO hardwareDeviceInfoSet;
		SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
		GUID myGUID = m_Guid;//GUID_DEVINTERFACE_ATL;
		HANDLE deviceHandle = INVALID_HANDLE_VALUE;
		DWORD result;
		UCHAR deviceIndex = 0;
		hardwareDeviceInfoSet = SetupDiGetClassDevs(&myGUID,
			NULL,
			NULL,
			(DIGCF_PRESENT |
				DIGCF_DEVICEINTERFACE));
		do {
			deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
			result = SetupDiEnumDeviceInterfaces(hardwareDeviceInfoSet,
				NULL,
				&myGUID,
				deviceIndex,
				&deviceInterfaceData);
			if (result == FALSE) {
				//TRACE("NOT a target USB\n");
			}
			else {
				//TRACE("FOUND a target USB\n");
				deviceIndex++;
			}
			SetupDiDestroyDeviceInfoList(hardwareDeviceInfoSet);
		} while (result == TRUE);
		return deviceIndex;
	}

	bool USB_Manager::Check_USB_BULK_Status(int device_index) const
	{
		auto& API = HFST_Library::GetAPI();
		int retNum = API.BULK.USBComm_InitEx(1500, device_index);
		if (retNum < 0) {
			API.BULK.USBComm_FinishEx();
			return false;
		}

		retNum = API.BULK.USBComm_CheckBulkInEx();
		if (retNum == 0) {
			API.BULK.USBComm_FinishEx();
			return false;
		}

		retNum = API.BULK.USBComm_CheckBulkOutEx();
		if (retNum == 0)
		{
			API.BULK.USBComm_FinishEx();
			return false;
		}
		return true;
	}
}
