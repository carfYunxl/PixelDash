#ifndef __HFST_DEVICE_MANAGER_HPP
#define __HFST_DEVICE_MANAGER_HPP

namespace HFST
{

    struct HFST_Library;
    class USB_Manager
    {
    public:
        USB_Manager();
        void Register();

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
}

#endif //__HFST_DEVICE_MANAGER_HPP
