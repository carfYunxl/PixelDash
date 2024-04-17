#ifndef __HFST_API_CENTER__
#define __HFST_API_CENTER__

#include "DLL/HFST_FuncLoader.hpp"

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

        inline static bool m_bInit = false;

        HFST_API(const HFST_API& rhs) = delete;
        HFST_API& operator=(const HFST_API& rhs) = delete;
    private:
        bool Init();

        HFST_API() = default;
    };
}

#endif //__HFST_API_CENTER__
