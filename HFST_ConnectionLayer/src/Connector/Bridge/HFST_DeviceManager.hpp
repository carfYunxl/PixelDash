#ifndef __HFST_DEVICE_MANAGER_HPP
#define __HFST_DEVICE_MANAGER_HPP

namespace HFST
{
    class USB_Manager
    {
    public:
        USB_Manager();
        bool Register(HWND hWnd);

        int  DetectUSBConnectCount() const;
        bool Check_USB_BULK_Status( int device_index ) const;

        const GUID GetGUID() const {
            return m_Guid;
        }
    private:
        HDEVNOTIFY		m_hUSBDevNotify;
        GUID            m_Guid{
                            0x8D98FC49,
                            0x7A37,
                            0x4B2D,
                            {0xA1, 0xA3, 0x55, 0xEA, 0x7B, 0xB2, 0xAE, 0x60}
                        };
    };

    class HID_Manager
    {
    public:
        HID_Manager();
        bool Register(HWND hWnd);

        const GUID GetGUID() const {
            return m_Guid;
        }

        bool SwicthToBULK();
    private:
        HDEVNOTIFY		m_hHIDDevNotify;
        GUID            m_Guid{
                            0x4d1e55b2, 
                            0xf16f,
                            0x11cf,
                            { 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} 
                        };
        const int m_PID = 0x1403;
        const int m_VID = 0x6100;
    };

    class ABT_Manager
    {
    public:
        ABT_Manager();
        bool Register(HWND hWnd);

        const GUID GetGUID() const {
            return m_Guid;
        }
    private:
        HDEVNOTIFY		m_hABTDevNotify;
        GUID            m_Guid{ 
                            0xf7fea3af,
                            0x6d81, 
                            0x4cbf, 
                            {0xa5, 0xc1, 0x3b, 0x78, 0xd5, 0xa6, 0x99, 0xbb}
                        };
    };
}

#endif //__HFST_DEVICE_MANAGER_HPP
