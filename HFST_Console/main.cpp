#include <windows.h>
#include <ConsoleApi.h>
#include "HFST_Connector.hpp"
#include <iostream>
#include <thread>
#include <Dbt.h>
#include <string>
#include <atlstr.h>
#include "HFST_DeviceManager.hpp"
#include "HFST_DummyWindow.hpp"
#include <format>

static LRESULT message_handler(HWND__* hwnd, UINT uint, WPARAM wparam, LPARAM lparam)
{
    static const GUID GUID_DEVINTERFACE_ATL = {
                            0x8D98FC49,
                            0x7A37,
                            0x4B2D,
                            {0xA1, 0xA3, 0x55, 0xEA, 0x7B, 0xB2, 0xAE, 0x60}
    };

    switch (uint)
    {
    case WM_NCCREATE: // before window creation
        return true;
        break;
    case WM_CREATE: // the actual creation of the window
    {
        LPCREATESTRUCT params = (LPCREATESTRUCT)lparam;
        GUID InterfaceClassGuid = *((GUID*)params->lpCreateParams);
        DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
        ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
        NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
        NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        memcpy(&(NotificationFilter.dbcc_classguid), &(GUID_DEVINTERFACE_ATL), sizeof(struct _GUID));
        HDEVNOTIFY dev_notify = RegisterDeviceNotification(hwnd, &NotificationFilter,
            DEVICE_NOTIFY_WINDOW_HANDLE);
        if (dev_notify == NULL)
        {
            throw std::runtime_error("Could not register for devicenotifications!");
        }
        break;
    }
    case WM_DEVICECHANGE:
    {
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lparam;
        PDEV_BROADCAST_DEVICEINTERFACE lpdbv = (PDEV_BROADCAST_DEVICEINTERFACE)lpdb;

        if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
        {
            USES_CONVERSION;
            std::string path(CT2A(CString(lpdbv->dbcc_name)));
            switch (wparam)
            {
            case DBT_DEVICEARRIVAL:
                std::cout << "new device connected: " << path << "\n";
                break;
            case DBT_DEVICEREMOVECOMPLETE:
                std::cout << "device disconnected: " << path << "\n";
                break;
            }
        }
        break;
    }
    }
    return 0;
}

int main(int argc, char* argv[])
{
    std::cout << std::format("{:02x}", (char)0xA3) << std::endl;
    return 0;
}

