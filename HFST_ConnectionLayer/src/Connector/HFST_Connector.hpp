#ifndef __HFST_CONNECTOR_HPP__
#define __HFST_CONNECTOR_HPP__

#include <memory>
#include "DLL/HFST_APICenter.hpp"
#include "Core/HFST_CommonHeader.hpp"
#include "RawData/HFST_RawCollector.hpp"
#include "Connector/Bridge/HFST_DeviceManager.hpp"

namespace HFST
{
    class Connector
    {
    public:
        Connector();
        ~Connector();

        std::error_code Connect(double vdd, double iovdd);
        void DisConnect();

        std::error_code I2C_ScanAddr();
        std::error_code IC_GetStatus();
        std::error_code IC_GetInformation();
        std::error_code IC_GetProtocol();
        std::error_code IC_GetChipID();
        std::error_code IC_SetI2CAddr( int nAddr );
        std::error_code SW_Reset();
        bool RegisterDevice(HWND hWnd);

        std::string Protocol();
        const GUID GetGUID() const { return m_Guid; }

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
        std::error_code      IC_SwitchPage( RAW::PageType type );

        void                 SetCommunicationMode(CommunicationMode mode) { m_CommunicationMode = mode; }
        bool                 SetVoltage(double vdd, double iovdd);
        bool                 GetTLInfo();
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
        CommunicationMode m_CommunicationMode{ CommunicationMode::TOUCH_LINK };
    };
}

#endif //__HFST_CONNECTOR_HPP__
