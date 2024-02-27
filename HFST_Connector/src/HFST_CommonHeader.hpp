#ifndef __HFST_COMMON_HEADER_HPP__
#define __HFST_COMMON_HEADER_HPP__

#include <map>
#include <string>

namespace HFST
{
    struct I2CAddr
    {
        bool    bICP{ false };
        size_t  nAddr{ 0 };
    };

    struct TL_Info
    {
        int nHwVer { -1 };
        int nFwVerH{ -1 };
        int nFwVerL{ -1 };
    };

    enum class IC_Status
    {
        NOT_CONNECT = 0,
        BOOT_LOADER,
        FW_ERROR,
        CONNECT
    };

    struct IC_Info
    {
        int         nFwVersion;
        int         nChipID{ -1 };
        int         nNumX{ -1 };
        int         nNumY{ -1 };
        int         nNumKey{ -1 };
        int         nStatus{ -1 };
        size_t      nProtocol;
        int         nResX;
        int         nResY;
        std::string strRevision;
    };

    namespace PROROCOL
    {
        constexpr size_t PROTOCOL_UNKNOWN	 = 0x00;	//Unknown
        constexpr size_t PROTOCOL_STPA		 = 0x01;	//Sitronix Protocol A
        constexpr size_t PROTOCOL_REPL		 = 0xFF;	//Replace Protocol
        constexpr size_t PROTOCOL_STPC		 = 0x03;	//Sitronix Protocol C
        constexpr size_t PROTOCOL_STNA		 = 0x02;	//Sitronix Protocol A with I2C clock non-stretch
        constexpr size_t PROTOCOL_STJ0		 = 0x11;	//Sitronix Protocol A for ST1801
        constexpr size_t PROTOCOL_STI0		 = 0xFE;	//Sitronix Protocol A for Ingenico POS 
        constexpr size_t PROTOCOL_STT0		 = 0x12;	//Sitronix Procotol A for ST1802 high resolution (7680 x 3840)
        constexpr size_t PROTOCOL_STPD       = 0x13;	//Sitronix Procotol A for A8018
        constexpr size_t PROTOCOL_STND       = 0x14;	//Sitronix Procotol A for A8018 clock non-stretch

        static const std::map<size_t, std::string> g_sMapProtocol{
            { PROTOCOL_STPA ,"STPA"},
            { PROTOCOL_REPL ,"REPL"},
            { PROTOCOL_STPC ,"STPC"},
            { PROTOCOL_STNA ,"STNA"},
            { PROTOCOL_STJ0 ,"STJ0"},
            { PROTOCOL_STI0 ,"STI0"},
            { PROTOCOL_STT0 ,"STT0"},
            { PROTOCOL_STPD ,"STPD"},
            { PROTOCOL_STND ,"STnD"}
        };
    }

    // I2C register map
    namespace ADDR_MAP
    {
        constexpr size_t FW_VERSION = 0x00;
        constexpr size_t STATUS     = 0x01;

        constexpr size_t RES_X_H    = 0x03;
        constexpr size_t RES_X_L    = 0x04;
        constexpr size_t RES_Y_H    = 0x05;
        constexpr size_t RES_Y_L    = 0x06;

        constexpr size_t REV_3      = 0x0C;
        constexpr size_t REV_2      = 0x0D;
        constexpr size_t REV_1      = 0x0E;
        constexpr size_t REV_0      = 0x0F;

        constexpr size_t GESTURE    = 0x10;
        constexpr size_t KEYS       = 0x11;
        constexpr size_t COORD      = 0x12;

        constexpr size_t MISC_INFO  = 0xF0;
        constexpr size_t MISC_CTL   = 0xF1;

        constexpr size_t PAGE       = 0xFF;
        constexpr size_t CHIPID     = 0xF4;
        constexpr size_t NUM_X      = 0xF5;
        constexpr size_t NUM_Y      = 0xF6;
        constexpr size_t NUM_KEY    = 0xF7;
    }
}


#endif //__HFST_COMMON_HEADER_HPP__
