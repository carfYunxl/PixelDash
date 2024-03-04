#include "pch.h"
#include "HFST_DeviceManager.hpp"

#pragma comment (lib, "Setupapi.lib")

namespace HFST
{
	USB_Manager::USB_Manager()
		: m_hUSBDevNotify(NULL)
	{
	}

    bool USB_Manager::Register(HWND hWnd)
    {
		DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
		ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
		NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
		NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		NotificationFilter.dbcc_classguid = m_Guid;

		m_hUSBDevNotify = RegisterDeviceNotification(hWnd,
			&NotificationFilter,
			DEVICE_NOTIFY_WINDOW_HANDLE
		);

		return (m_hUSBDevNotify != NULL);
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
		auto pApi = HFST_API::GetAPI();
		if (!pApi)
			return false;

		int retNum = pApi->BULK.USBComm_InitEx(1500, device_index);
		if (retNum < 0) {
			pApi->BULK.USBComm_FinishEx();
			return false;
		}

		retNum = pApi->BULK.USBComm_CheckBulkInEx();
		if (retNum == 0) {
			pApi->BULK.USBComm_FinishEx();
			return false;
		}

		retNum = pApi->BULK.USBComm_CheckBulkOutEx();
		if (retNum == 0)
		{
			pApi->BULK.USBComm_FinishEx();
			return false;
		}
		return true;
	}

	HID_Manager::HID_Manager()
		: m_hHIDDevNotify{NULL}
	{

	}

	bool HID_Manager::SwicthToBULK()
	{
		auto api = HFST_API::GetAPI();
		if ( !api )
			return false;

		int ret = api->USB_HID.OpenHID(m_PID, m_VID);
		if ( ret <= 0 )
			return false;

		unsigned char buffer[16]{ 0 };
		buffer[0] = 0x00;
		buffer[1] = 0xAD;
		buffer[2] = 0x01;

		bool res = api->USB_HID.WriteHID(buffer);
		if (!ret)
			return false;

		api->USB_HID.CloseHID();

		return true;
	}

	bool HID_Manager::Register(HWND hWnd)
	{
		DEV_BROADCAST_DEVICEINTERFACE NotificationFilter2;

		ZeroMemory(&NotificationFilter2, sizeof(NotificationFilter2));
		NotificationFilter2.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
		NotificationFilter2.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		NotificationFilter2.dbcc_classguid = m_Guid;

		m_hHIDDevNotify = RegisterDeviceNotification(
			hWnd,
			&NotificationFilter2,
			DEVICE_NOTIFY_WINDOW_HANDLE
		);

		return (m_hHIDDevNotify != NULL);
	}

	ABT_Manager::ABT_Manager()
		: m_hABTDevNotify{ NULL }
	{

	}
	bool ABT_Manager::Register(HWND hWnd)
	{
		DEV_BROADCAST_DEVICEINTERFACE NotificationFilter2;

		ZeroMemory(&NotificationFilter2, sizeof(NotificationFilter2));
		NotificationFilter2.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
		NotificationFilter2.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		NotificationFilter2.dbcc_classguid = m_Guid;

		m_hABTDevNotify = RegisterDeviceNotification(
			hWnd,
			&NotificationFilter2,
			DEVICE_NOTIFY_WINDOW_HANDLE
		);

		return (m_hABTDevNotify != NULL);
	}
}
