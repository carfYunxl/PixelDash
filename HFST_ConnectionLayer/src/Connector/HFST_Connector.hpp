#ifndef __HFST_CONNECTOR_HPP__
#define __HFST_CONNECTOR_HPP__

#include <memory>
#include "HFST_APICenter.hpp"
#include "HFST_CommonHeader.hpp"
#include "HFST_RawCollector.hpp"
#include "HFST_DeviceManager.hpp"

namespace HFST
{
    class Connector
    {
    public:
        Connector();
        ~Connector();

        bool Connect();
        void DisConnect();

        const GUID GetGUID() const { return m_Guid; }

        bool I2C_ScanAddr();

        bool IC_GetStatus();
        bool IC_GetInformation();
        bool IC_GetProtocol();
        bool IC_GetChipID();
        bool IC_SetI2CAddr( int nAddr );

        std::string Protocol();

        bool SW_Reset();
        bool RegisterDevice(HWND hWnd);
    public: // Get infomation
        int                  GetChipID()        const { return m_IcInfo.nChipID;                  }
        std::pair<int, int>  GetRxTxCnt()       const { return { m_IcInfo.nNumX,m_IcInfo.nNumY };   }
        int                  GetStatus()        const { return m_IcInfo.nStatus;                  }
        std::vector<I2CAddr> GetI2CAddr()       const { return m_vI2CAddr;                        }
        IC_Info              IC_GetInfo()       const { return m_IcInfo; }
        std::pair<int, int>  GetResXY()         const { return { m_IcInfo.nResX, m_IcInfo.nResY};}
        std::string          GetFwRevision()    const { return m_IcInfo.strRevision; }
        int                  GetFwVersion()     const { return m_IcInfo.nFwVersion; }
        int                  GetNumKey()        const { return m_IcInfo.nNumKey; }
    public: // Set method
        bool                 IC_SwitchPage( RAW::PageType type );
    private:
        std::vector<I2CAddr>    m_vI2CAddr;

    private:

        int        m_nCurrentI2CAddr;
        double     m_dCurrentVDD;
        double     m_dCurrentIOVDD;
        TL_Info    m_TLInfo;
        IC_Status  m_IcStatus;
        IC_Info    m_IcInfo;
        bool       m_I2cFlag{ false };

        std::unique_ptr<USB_Manager>    m_UsbManager;
        std::unique_ptr<HID_Manager>    m_HidManager;
        std::unique_ptr<ABT_Manager>    m_AbtManager;
        GUID       m_Guid;
        CommunicationMode m_CommunicationMode{ CommunicationMode::TOUCH_PAD };
    };
}

#endif //__HFST_CONNECTOR_HPP__
