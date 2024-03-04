#ifndef __HFST_API_CENTER__
#define __HFST_API_CENTER__

#include "HFST_FuncLoader.hpp"

namespace HFST
{
    class HFST_API
    {
    public:
        TTK_Communication_API   TTK;
        Bulk_API                BULK;
        WinUSB_Driver_API       USB_Driver;
        USB_HID                 USB_HID;

        static HFST_API* GetAPI()
        {
            static HFST_API m_API;

            if ( !m_bInit )
            {
                if (!m_API.Init())
                    return nullptr;

                m_bInit = true;
            }

            return &m_API;
        }

        static bool m_bInit;
    private:
        bool Init();
    private:
        HFST_API() = default;
        HFST_API(const HFST_API& rhs) = default;
        HFST_API& operator=(const HFST_API& rhs) = default;
    };
}

#endif //__HFST_API_CENTER__
