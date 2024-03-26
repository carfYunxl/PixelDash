#include "pch.h"
#include "HFST_Connector.hpp"
#include "HFST_FuncLoader.hpp"
#include "HFST_Exception.hpp"
#include "HFST_DeviceManager.hpp"
#include "HFST_TouchDevice.hpp"
#include "HFST_CommandIO.hpp"
#include "HFST_Bridge.hpp"
#include "HFST_BulkController.hpp"
#include "HFST_Error.hpp"
namespace HFST
{
    Connector::Connector()
        : m_nCurrentI2CAddr{ -1 }
        , m_dCurrentVDD{2.8}
        , m_dCurrentIOVDD{1.8}
        , m_IcStatus{ IC_Status::NOT_CONNECT }
        , m_UsbManager(std::make_unique<USB_Manager>())
        , m_HidManager(std::make_unique<HID_Manager>())
        , m_AbtManager(std::make_unique<ABT_Manager>())
        , m_Guid{m_UsbManager->GetGUID()} {
    }

    Connector::~Connector() { }

    std::error_code Connector::Connect()
    {
        std::unique_ptr<Bridge> pBridge = CreateBridge(m_CommunicationMode);
        if ( !pBridge || !pBridge->Attach() )
        {
            return make_error_code(-7);
        }

        //if ( m_TouchLink->GetBulk().DetectUSBConnectCount( USB_Manager::GetGUID() ) <= 0 ) {
        //    m_HidManager->SwicthToBULK();
        //    int nRetry = 10;
        //    while ( nRetry > 0 )
        //    {
        //        if ( m_TouchLink->GetBulk().DetectUSBConnectCount(USB_Manager::GetGUID()) > 0  )
        //            break;

        //        m_HidManager->SwicthToBULK();
        //        nRetry--;
        //    }
        //    return false;
        //}

        if ( !I2C_ScanAddr() )
            return make_error_code(-8);

        if ( !IC_SetI2CAddr(m_nCurrentI2CAddr) )
            return make_error_code(-9);

        if ( !IC_GetChipID() )
            return make_error_code(-10);

        if ( !IC_GetProtocol() )
            return make_error_code(-11);

        if ( m_IcInfo.nProtocol == PROROCOL::PROTOCOL_STNA || m_IcInfo.nProtocol == PROROCOL::PROTOCOL_STND ) {
            m_I2cFlag = true;

            if (!IC_SetI2CAddr(m_nCurrentI2CAddr))
                return make_error_code(-9);
        }

        if ( !IC_GetStatus() )
            return make_error_code(-12);

        switch (m_IcStatus)
        {
            case IC_Status::NOT_CONNECT:
                break;
            case IC_Status::BOOT_LOADER:
                break;
            case IC_Status::FW_ERROR:
                break;
            case IC_Status::CONNECT:
            {
                if ( !IC_GetInformation() )
                    return make_error_code(-13);

                if ( m_IcInfo.nChipID == static_cast<int>(ChipID::A8018) )
                {
                    CommandIO cmd_io(m_IcInfo);
                    if (!cmd_io.GetInfo())
                        return make_error_code(-14);
                }
                break;
            }
        }
        
        return make_error_code(0);
    }

    void Connector::DisConnect()
    {
        std::cout << "DisConnect Success!" << std::endl;
    }

    std::error_code Connector::I2C_ScanAddr()
    {
        auto* pApi = HFST_API::GetAPI();
        if (!pApi)
            return make_error_code(-1);;

        if ( !m_vI2CAddr.empty() )
            m_vI2CAddr.clear();


        // 扫描I2C地址，并设置I2C地址
        unsigned char map{ 0 };
        int ret{ 0 };
        for ( int addr = 0x00; addr < 0x80; addr++ )
        {
            ret = pApi->TTK.SetI2CAddr(addr, 2, false);
            ret = pApi->TTK.ReadI2CReg(&map, 1, 1);
            if ( ret > 0 )
            {
                // for A8018 I2C polling, 5ms/addr, total 20ms
                if ( map == 0x86 )
                {
                    // send 0x84 0x5A 0xFB 0xF0 0x00 0x03
                    unsigned char uPassKey[6]{ 0x84, 0x5A, 0xFB, 0xF0, 0x00, 0x03 };
                    pApi->TTK.WriteCmd(uPassKey, sizeof(uPassKey));
                }
                // for A8018, check if is icp i2c addr
                else if (pApi->TTK.CheckIsA8018ICPI2CAddr != NULL && pApi->TTK.CheckIsA8018ICPI2CAddr(addr))
                {
                    m_vI2CAddr.emplace_back(true, addr);
                    continue;
                }

                m_vI2CAddr.emplace_back(false, addr);
                m_nCurrentI2CAddr = addr;
            }
        }

        return make_error_code(0);;
    }

    std::error_code Connector::SW_Reset()
    {
        auto* pApi = HFST_API::GetAPI();
        if (!pApi)
            return make_error_code(-1);;

        unsigned char buffer = 0x01;
        int ret = pApi->TTK.WriteI2CReg( &buffer, 0x02, 1 );
        if (ret <= 0)
            return make_error_code(-5);;

        return make_error_code(0);;
    }

    std::error_code Connector::IC_GetStatus()
    {
        auto* pApi = HFST_API::GetAPI();
        if (!pApi)
            return make_error_code(-1);;

        if ( !SW_Reset() )
            return make_error_code(-15);;

        unsigned char buffer{0};
        int ret = pApi->TTK.ReadI2CReg( &buffer, 0x01, 1 );
        if (ret <= 0)
        {
            m_IcStatus = IC_Status::NOT_CONNECT;
            return make_error_code(-5);;
        }

        int nStatus = (buffer & 0x0F );
        int nErrorCode = ( (buffer & 0xF0) >> 4 );

        if ( (nStatus & 0x0F) == 0x06 )
            m_IcStatus = IC_Status::BOOT_LOADER;
        else
        {
            if ( nErrorCode != 0 )
            {
                m_IcStatus = IC_Status::FW_ERROR;
            }
            else
            {
                m_IcStatus = IC_Status::CONNECT;
            }
        }

        return make_error_code(0);;
    }

    std::error_code Connector::IC_GetInformation()
    {
        auto* pApi = HFST_API::GetAPI();
        if (!pApi)
            return make_error_code(-1);;

        if (!SW_Reset())
            return make_error_code(-15);

        constexpr size_t READ_LEN = 256;
        unsigned char buffer[READ_LEN]{ 0 };

        int ret = pApi->TTK.ReadI2CReg( buffer, 0x00, READ_LEN );
        if ( ret <= 0 )
            return make_error_code(-5);

        m_IcInfo.nResX = (buffer[ADDR_MAP::RES_X_H] << 8) | buffer[ADDR_MAP::RES_X_L];
        m_IcInfo.nResY = (buffer[ADDR_MAP::RES_Y_H] << 8) | buffer[ADDR_MAP::RES_Y_L];

        m_IcInfo.strRevision = std::format(
            "{:02x} {:02x} {:02x} {:02x}",
            buffer[ADDR_MAP::REV_3],
            buffer[ADDR_MAP::REV_2],
            buffer[ADDR_MAP::REV_1],
            buffer[ADDR_MAP::REV_0]
            );

        m_IcInfo.nNumX = buffer[ADDR_MAP::NUM_X];
        m_IcInfo.nNumY = buffer[ADDR_MAP::NUM_Y];
        m_IcInfo.nNumKey = buffer[ADDR_MAP::NUM_KEY] & 0x0F;
        m_IcInfo.nStatus = buffer[ADDR_MAP::STATUS];
        m_IcInfo.nChipID = buffer[ADDR_MAP::CHIPID];
        m_IcInfo.nFwVersion = buffer[ADDR_MAP::FW_VERSION];

        return make_error_code(0);
    }

    std::error_code Connector::IC_GetProtocol()
    {
        auto* pApi = HFST_API::GetAPI();
        if (!pApi)
            return make_error_code(-1);

        constexpr size_t READ_LEN = 8;
        unsigned char buffer[READ_LEN]{ 0 };

        int ret = pApi->TTK.ReadI2CReg( buffer, ADDR_MAP::PAGE, READ_LEN );
        if ( ret <= 0 )
            return make_error_code(-5);

        std::string strProtocol( (const char* const)&buffer[1], 4 );

        auto iterator = std::find_if(PROROCOL::g_sMapProtocol.begin(), PROROCOL::g_sMapProtocol.end(), [=]( const std::pair<const size_t, std::string>& pair ) {
            return pair.second == strProtocol;
        });

        if (iterator == PROROCOL::g_sMapProtocol.end())
            m_IcInfo.nProtocol = PROROCOL::PROTOCOL_UNKNOWN;
        else
            m_IcInfo.nProtocol = (*iterator).first;

        return make_error_code(0);
    }

    std::error_code Connector::IC_GetChipID()
    {
        auto* pApi = HFST_API::GetAPI();
        if (!pApi)
            return make_error_code(-1);

        constexpr size_t READ_LEN = 1;
        unsigned char buffer[READ_LEN]{ 0 };

        int ret = pApi->TTK.ReadI2CReg( buffer, ADDR_MAP::CHIPID, READ_LEN );
        if (ret <= 0)
            return make_error_code(-5);

        int nChipID = buffer[0];

        m_IcInfo.nChipID = buffer[0];
        return make_error_code(0);
    }

    std::string Connector::Protocol()
    {
        switch ( m_IcInfo.nProtocol )
        {
            case PROROCOL::PROTOCOL_UNKNOWN:
            {
                return "UnKnown";
            }
            case PROROCOL::PROTOCOL_STPA:
            {
                return "Sitronix";
            }
            case PROROCOL::PROTOCOL_REPL:
            {
                return "Replace";
            }
            case PROROCOL::PROTOCOL_STPC:
            {
                return "Sitronix C";
            }
            case PROROCOL::PROTOCOL_STNA:
            case PROROCOL::PROTOCOL_STND:
            {
                return "Sitronix A(non-stretch)";
            }
            case PROROCOL::PROTOCOL_STJ0:
            case PROROCOL::PROTOCOL_STI0:
            case PROROCOL::PROTOCOL_STT0:
            case PROROCOL::PROTOCOL_STPD:
            {
                return "Sitronix A";
            }
        }
        
        return "UnKnown";
    }

    std::error_code Connector::IC_SetI2CAddr(int nAddr)
    {
        auto* pApi = HFST_API::GetAPI();
        if (!pApi)
            return make_error_code(-1);

        int ret = pApi->TTK.SetI2CAddr( m_nCurrentI2CAddr, 2, m_I2cFlag );
        if (ret < 0)
            return make_error_code(-9);

        return make_error_code(0);
    }

    std::error_code Connector::IC_SwitchPage( RAW::PageType type )
    {
        switch ( type )
        {
        case RAW::PageType::REPORT:
            break;
        case RAW::PageType::DEVELOP:
            break;
        }

        return make_error_code(0);
    }

    bool Connector::RegisterDevice(HWND hWnd)
    {
        bool success = m_UsbManager->Register(hWnd);
        success &= m_HidManager->Register(hWnd);
        success &= m_AbtManager->Register(hWnd);

        return success;
    }
}
