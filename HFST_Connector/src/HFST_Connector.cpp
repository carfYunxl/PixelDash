#include "pch.h"
#include "HFST_Connector.hpp"
#include "HFST_FuncLoader.hpp"
#include "HFST_Exception.hpp"
#include "HFST_DeviceManager.hpp"
#include "HFST_TouchDevice.hpp"

namespace HFST
{
    Connector::Connector()
        : m_nCurrentI2CAddr{ -1 }
        , m_dCurrentVDD{2.8}
        , m_dCurrentIOVDD{1.8}
        , m_ICStatus{ IC_Status::NOT_CONNECT }
    {
    }

    Connector::~Connector() { }

    bool Connector::Connect()
    {
        // 初始化API
        if ( !m_API.Init() )
            return false;
        std::cout << "Load dll success!\n";

        USB_Manager usbManager;
        SetGUID(usbManager.GetGUID());

        // 检查BULK的状态
        // detect usb connect, but bulk not ok, install the driver
        if ( !CheckAndInstallTLDriver(usbManager) )
            return false;

        // 获取及设置TouchLink信息     
        if ( !TL_GetInfomation() )
            return false;

        // 设置TouchLink电压
        if ( !TL_SetVoltage(2.8, 1.8) )
            return false;

        // 扫描I2C地址
        I2C_ScanAddr();

        // 设置I2C地址
        m_API.TTK.SetI2CAddr( m_nCurrentI2CAddr, 2, false );

        // 获取Protocol
        if ( !IC_GetProtocol() )
            return false;

        // 获取IC的状态码
        if ( !IC_GetStatus() )
            return false;

        switch (m_ICStatus)
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
                    return false;
            }
        }
        
        return true;
    }

    void Connector::DisConnect()
    {
        std::cout << "DisConnect Success!" << std::endl;
    }

    void Connector::I2C_ScanAddr()
    {
        if ( !m_vI2CAddr.empty() )
            m_vI2CAddr.clear();

        // 扫描I2C地址，并设置I2C地址
        unsigned char map{ 0 };
        int ret{ 0 };
        for ( int addr = 0x00; addr < 0x80; addr++ )
        {
            ret = m_API.TTK.SetI2CAddr(addr, 2, false);
            ret = m_API.TTK.ReadI2CReg(&map, 1, 1);
            if ( ret > 0 )
            {
                // for A8018 I2C polling, 5ms/addr, total 20ms
                if ( map == 0x86 )
                {
                    // send 0x84 0x5A 0xFB 0xF0 0x00 0x03
                    unsigned char uPassKey[6]{ 0x84, 0x5A, 0xFB, 0xF0, 0x00, 0x03 };
                    m_API.TTK.WriteCmd(uPassKey, sizeof(uPassKey));
                }
                // for A8018, check if is icp i2c addr
                else if (m_API.TTK.CheckIsA8018ICPI2CAddr != NULL && m_API.TTK.CheckIsA8018ICPI2CAddr(addr))
                {
                    m_vI2CAddr.emplace_back(true, addr);
                    continue;
                }

                m_vI2CAddr.emplace_back(false, addr);
                m_nCurrentI2CAddr = addr;
            }
        }
    }

    bool Connector::TL_SetVoltage( double vdd, double iovdd )
    {
        int ret{ -1 };
        if ( m_TLInfo.nHwVer > 5 )
        {
            ret = m_API.TTK.SetTouchLinkVoltage( (unsigned short)(vdd * 1000), (unsigned short)(iovdd * 1000) );
        }
        else
            ret = m_API.TTK.SetTouchLink3_Voltage( vdd, iovdd );

        if ( ret <= 0 )
        {
            return false;
        }

        m_dCurrentVDD = vdd;
        m_dCurrentIOVDD = iovdd;
        return true;
    }

    bool Connector::CheckAndInstallTLDriver( const USB_Manager& usb )
    {
        for (int i = 1; i <= 8; ++i)
        {
            if (m_API.USB_Driver.WinUSB_Driver_CheckIsConnected(i) == 0)
            {
                // check usb status
                if ((!usb.Check_USB_BULK_Status(i)) && (usb.DetectUSBConnectCount() <= 0))
                {
                    std::thread thread_install_TL_Driver([&]() -> bool{
                         
                        if (m_API.USB_Driver.WinUSB_Driver_Install(i))
                            return false;

                        // clear setup info files
                        m_API.USB_Driver.WinUSB_Driver_RemoveFiles();

                        return true;
                        });
                    thread_install_TL_Driver.join();
                    break;
                }
            }
        }

        return true;
    }

    bool Connector::TL_GetInfomation()
    {
        // 获取及设置TouchLink信息
        unsigned char buffer[64]{ 0 };
        int ret = m_API.TTK.GetI2CStatus_Bulk(buffer);
        if (ret <= 0)
            return false;

        m_TLInfo.nHwVer = (buffer[2] >> 5);
        m_TLInfo.nFwVerH = (buffer[2] & 0x1F);
        m_TLInfo.nFwVerL = buffer[3];
        return true;
    }

    bool Connector::SW_Reset()
    {
        unsigned char nReset{ 01 };

        int ret = m_API.TTK.WriteI2CReg( &nReset, 02, 01 );
        if ( ret <= 0 )
            return false;

        return true;
    }

    bool Connector::IC_GetStatus()
    {
        if ( !SW_Reset() )
            return false;

        unsigned char buffer{0};
        int ret = m_API.TTK.ReadI2CReg( &buffer, 0x01, 1 );
        if (ret <= 0)
        {
            m_ICStatus = IC_Status::NOT_CONNECT;
            return false;
        }

        int nStatus = (buffer & 0x0F );
        int nErrorCode = ( (buffer & 0xF0) >> 4 );

        if ( (nStatus & 0x0F) == 0x06 )
            m_ICStatus = IC_Status::BOOT_LOADER;
        else
        {
            if ( nErrorCode != 0 )
            {
                m_ICStatus = IC_Status::FW_ERROR;
            }
            else
            {
                m_ICStatus = IC_Status::CONNECT;
            }
        }

        return true;
    }

    bool Connector::IC_GetInformation()
    {
        if (!SW_Reset())
            return false;

        constexpr size_t READ_LEN = 256;
        unsigned char buffer[READ_LEN]{ 0 };

        int ret = m_API.TTK.ReadI2CReg( buffer, 0x00, READ_LEN );
        if ( ret <= 0 )
            return false;

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

        return true;
    }

    bool Connector::IC_GetProtocol()
    {
        constexpr size_t READ_LEN = 8;
        unsigned char buffer[READ_LEN]{ 0 };

        int ret = m_API.TTK.ReadI2CReg( buffer, ADDR_MAP::PAGE, READ_LEN );
        if ( ret <= 0 )
            return false;

        std::string strProtocol( (const char* const)&buffer[1], 4 );

        auto iterator = std::find_if(PROROCOL::g_sMapProtocol.begin(), PROROCOL::g_sMapProtocol.end(), [=]( const std::pair<const size_t, std::string>& pair ) {
            return pair.second == strProtocol;
        });

        if (iterator == PROROCOL::g_sMapProtocol.end())
            m_IcInfo.nProtocol = PROROCOL::PROTOCOL_UNKNOWN;
        else
            m_IcInfo.nProtocol = (*iterator).first;

        return true;
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

    std::string Connector::ReadRawData()
    {
#if 1
        std::string str;
        str.resize(20);
        m_API.TTK.ReadI2CReg((unsigned char*)str.data(), 0x40, 20);

        return str;
#else
        unsigned char buffer[20]{ 0 };
        m_API.TTK.ReadI2CReg( buffer, 0x40, 20 );

        int nValidSize = buffer[1];
        int nLength = nValidSize / 2;

        std::vector<short> back;

        for ( int i = 0; i < nLength; ++i )
        {
            short raw = buffer[2 + 2 * i] << 8 | buffer[2 + 2 * i + 1];
            back.emplace_back(raw);
        }

        return back;
#endif
    }    
}
