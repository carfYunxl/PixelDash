#include "pch.h"
#include "HFST_DeviceManager.hpp"

#pragma comment (lib, "Setupapi.lib")

namespace HFST
{
	//! ********************* Implementation of USB_Manager *********************!//
	//! 
	const GUID USB_Manager::m_Guid = 
							{ 
								0x8D98FC49,
								0x7A37,
								0x4B2D,
								{0xA1, 0xA3, 0x55, 0xEA, 0x7B, 0xB2, 0xAE, 0x60} 
							};
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

	//! ********************* Implementation of HID_Manager *********************!//
	//! 
	const GUID HID_Manager::m_Guid = 
							{
								0x4d1e55b2,
								0xf16f,
								0x11cf,
								{ 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}
							};
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

	//! ********************* Implementation of ABT_Manager *********************!//
	//! 
	const GUID HID_Manager::m_Guid =
	{
		0xf7fea3af,
		0x6d81,
		0x4cbf,
		{0xa5, 0xc1, 0x3b, 0x78, 0xd5, 0xa6, 0x99, 0xbb}
	};
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
