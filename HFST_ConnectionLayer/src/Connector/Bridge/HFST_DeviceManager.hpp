#ifndef __HFST_DEVICE_MANAGER_HPP
#define __HFST_DEVICE_MANAGER_HPP

namespace HFST
{
    class USB_Manager
    {
    public:
        USB_Manager();
        bool Register(HWND hWnd);

        static const GUID GetGUID() {
            return m_Guid;
        }
    private:
        HDEVNOTIFY		        m_hUSBDevNotify;
        static const GUID       m_Guid;
    };

    class HID_Manager
    {
    public:
        HID_Manager();
        bool Register(HWND hWnd);

        static const GUID GetGUID() {
            return m_Guid;
        }

        bool SwicthToBULK();
    private:
        HDEVNOTIFY		        m_hHIDDevNotify;
        static const GUID       m_Guid;

        const int m_PID = 0x1403;
        const int m_VID = 0x6100;
    };

    class ABT_Manager
    {
    public:
        ABT_Manager();
        bool Register(HWND hWnd);

        static const GUID GetGUID() {
            return m_Guid;
        }
    private:
        HDEVNOTIFY		m_hABTDevNotify;
        static const GUID     m_Guid;
    };
}

#endif //__HFST_DEVICE_MANAGER_HPP
