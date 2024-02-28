#ifndef __HFST_CONNECTOR_HPP__
#define __HFST_CONNECTOR_HPP__

#include <memory>
#include "HFST_APICenter.hpp"
#include "HFST_CommonHeader.hpp"
#include "HFST_RawManager.hpp"

namespace HFST
{
    class USB_Manager;
    class Connector
    {
    public:
        Connector();
        ~Connector();

        bool Connect();
        void DisConnect();

        void SetGUID(GUID guid) { m_Guid = guid; }
        const GUID GetGUID() const { return m_Guid; }

        void I2C_ScanAddr();
        bool TL_SetVoltage( double vdd, double iovdd );
        bool CheckAndInstallTLDriver( const USB_Manager& usb );

        bool TL_GetInfomation();
        bool IC_GetStatus();
        bool IC_GetInformation();
        bool IC_GetProtocol();

        std::string Protocol();

        bool SW_Reset();

        std::string  ReadRawData();
    public:
        int                  GetChipID()        const { return m_IcInfo.nChipID;                  }
        std::pair<int, int>  GetRxTxCnt()       const { return { m_IcInfo.nNumX,m_IcInfo.nNumY };   }
        int                  GetStatus()        const { return m_IcInfo.nStatus;                  }
        std::vector<I2CAddr> GetI2CAddr()       const { return m_vI2CAddr;                        }
        IC_Info              IC_GetInfo()       const { return m_IcInfo; }
        std::pair<int, int>  GetResXY()         const { return { m_IcInfo.nResX, m_IcInfo .nResY};}
        std::string          GetFwRevision()    const { return m_IcInfo.strRevision; }
        int                  GetFwVersion()     const { return m_IcInfo.nFwVersion; }
        int                  GetNumKey()        const { return m_IcInfo.nNumKey; }
    private:
        std::vector<I2CAddr>    m_vI2CAddr;

    private:
        HFST_API  m_API;
        GUID      m_Guid;
        int       m_nCurrentI2CAddr;
        double    m_dCurrentVDD;
        double    m_dCurrentIOVDD;
        TL_Info   m_TLInfo;
        IC_Status m_ICStatus;
        IC_Info   m_IcInfo;

        RawData*  m_Raw;
    };
}

#endif //__HFST_CONNECTOR_HPP__
