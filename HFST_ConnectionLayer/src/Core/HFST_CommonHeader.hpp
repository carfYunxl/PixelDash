#ifndef __HFST_COMMON_HEADER_HPP__
#define __HFST_COMMON_HEADER_HPP__

#include <map>
#include <string>
#include <vector>
#include "HFST_APICenter.hpp"

namespace HFST
{
    using uchar = unsigned char;

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

        double dVDD{0.0};
        double dIOVDD{0.0};
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
        int         nFwVersion  {-1};
        int         nChipID     {-1};
        int         nNumX       {-1};
        int         nNumY       {-1};
        int         nNumKey     {-1};
        int         nStatus     {-1};
        size_t      nProtocol   { 0};
        int         nResX       {-1};
        int         nResY       {-1};
        std::string strRevision;

        // for A8018
        bool        TagTypeSel                  {false};
        bool        bNoiseMergEn                {false};
        bool        Mutual_Axis                 {false};
        bool        Mutual_ChnType              {false};
        bool        Mutual_MultiLineEnable      {false};
        uchar       Mutual_MultiLineNum         {0};
        uchar       Mutual_MultiNoiseLineNum    {0};
        bool        Self_1_Axis                 {false};
        bool        Self_1_ChnType              {false};
        bool        Self_1_MultiLineEnable      {false};
        uchar       Self_1_MultiLineNum         {0};
        uchar       Self_1_MultiNoiseLineNum    {0};
        bool        Self_2_Axis                 {false};
        bool        Self_2_ChnType              {false};
        bool        Self_2_MultiLineEnable      {false};
        uchar       Self_2_MultiLineNum         {0};
        uchar       Self_2_MultiNoiseLineNum    {0};
        bool        Key_Axis                    {false};
        bool        Key_ChnType                 {false};
        bool        Key_MultiLineEnable         {false};
        uchar       Key_MultiLineNum            {0};
        uchar       Key_MultiNoiseLineNum       {0};
        bool        bSlfTP                      {false};
        bool        bSlfTP_Tri                  {false};
        uchar       nValidSelfLen               {0};
    };

    namespace PROROCOL
    {
        constexpr unsigned char PROTOCOL_UNKNOWN	= 0x00;	//Unknown
        constexpr unsigned char PROTOCOL_STPA		= 0x01;	//Sitronix Protocol A
        constexpr unsigned char PROTOCOL_REPL		= 0xFF;	//Replace Protocol
        constexpr unsigned char PROTOCOL_STPC		= 0x03;	//Sitronix Protocol C
        constexpr unsigned char PROTOCOL_STNA		= 0x02;	//Sitronix Protocol A with I2C clock non-stretch
        constexpr unsigned char PROTOCOL_STJ0		= 0x11;	//Sitronix Protocol A for ST1801
        constexpr unsigned char PROTOCOL_STI0		= 0xFE;	//Sitronix Protocol A for Ingenico POS 
        constexpr unsigned char PROTOCOL_STT0		= 0x12;	//Sitronix Procotol A for ST1802 high resolution (7680 x 3840)
        constexpr unsigned char PROTOCOL_STPD       = 0x13;	//Sitronix Procotol A for A8018
        constexpr unsigned char PROTOCOL_STND       = 0x14;	//Sitronix Procotol A for A8018 clock non-stretch

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
        constexpr unsigned char FW_VERSION = 0x00;
        constexpr unsigned char STATUS     = 0x01;
        constexpr unsigned char RES_X_H    = 0x03;
        constexpr unsigned char RES_X_L    = 0x04;
        constexpr unsigned char RES_Y_H    = 0x05;
        constexpr unsigned char RES_Y_L    = 0x06;
        constexpr unsigned char REV_3      = 0x0C;
        constexpr unsigned char REV_2      = 0x0D;
        constexpr unsigned char REV_1      = 0x0E;
        constexpr unsigned char REV_0      = 0x0F;
        constexpr unsigned char GESTURE    = 0x10;
        constexpr unsigned char KEYS       = 0x11;
        constexpr unsigned char COORD      = 0x12;
        constexpr unsigned char RAW        = 0x40;
        constexpr unsigned char CMDIO_PORT = 0xD0;
        constexpr unsigned char MISC_INFO  = 0xF0;
        constexpr unsigned char MISC_CTL   = 0xF1;
        constexpr unsigned char PAGE       = 0xFF;
        constexpr unsigned char CHIPID     = 0xF4;
        constexpr unsigned char NUM_X      = 0xF5;
        constexpr unsigned char NUM_Y      = 0xF6;
        constexpr unsigned char NUM_KEY    = 0xF7;
        constexpr unsigned char CMDIO_CTRL = 0xF8;
    }

    namespace RAW
    {
        enum class TAG_TYPE
        {
            AG = 0,
            AG_FLUSH,
            GND,
            GND_FLUSH
        };
        // some definition
        template<typename T>
        using Vec = std::vector<T>;

        template<typename T>
        struct ChannelRaw
        {
            ChannelRaw() 
                : nDataType(0)
                , nChannelIdx(0)
                , Type(TAG_TYPE::AG)
            {}
            uchar       nDataType;        // Raw type
            uchar       nChannelIdx;      // which channel
            TAG_TYPE    Type;             // tell the tag info
            Vec<T>      vecRaw;           // Raw
        };

        // some structure
        enum class RawMode {
            RAW = 0,
            DELTA,
            DIST
        };

        enum class PageType
        {
            REPORT = 0,
            DEVELOP
        };

        template<typename T>
        struct Frame
        {
            void Clear()
            {
                vctHeader.vecRaw.clear();
                vctKey.vecRaw.clear();
                vctKeyNs.vecRaw.clear();

                vctMutual.clear();
                vctMuNoise.clear();
                vctXSelf.clear();
                vctXSelfNs.clear();
                vctYSelf.clear();
                vctYSelfNs.clear();
            }

            ChannelRaw<T>       vctHeader;
            ChannelRaw<T>       vctKey;
            ChannelRaw<T>       vctKeyNs;
            Vec<ChannelRaw<T>>  vctMutual;
            Vec<ChannelRaw<T>>  vctMuNoise;
            Vec<ChannelRaw<T>>  vctXSelf;
            Vec<ChannelRaw<T>>  vctXSelfNs;
            Vec<ChannelRaw<T>>  vctYSelf;
            Vec<ChannelRaw<T>>  vctYSelfNs;
        };
    }

    namespace COMMAND_IO
    {
        constexpr size_t        CMDIO_PACK_SIZE = 30;
        constexpr unsigned char CMD_RW_OUTPUT   = 0x82;
        constexpr unsigned char CMD_INFO_OUTPUT = 0x84;

        enum class State
        {
            OK = 0x00,
            PROCESSING = 0x01,
            UNKNOWN_ID = 0x80,
            CHECKSUM_ERROR = 0x81
        };

        enum class CmdID
        {
            WRITE = 0x01,
            READ,
            GET_INFO = 0x04
        };

        enum class MEMORY_TYPE
        {
            MCU_MEM = 0,
            MCU_SFRs,
            MCU_ROM,
            AFE_MEM,
            AFE_REG
        };

        enum class INFO_ID
        {
            INFO_RAWDATA_FORMAT = 1,    // Not support yet
            INFO_CHANNEL = 2,           // Not support yet
            INFO_RAW_LINE = 3           // support
        };
    }

    // Chip ID
    enum class ChipID
    {
        A8008 = 0x06,
        A8015 = 0x0F,
        A8010 = 0x0A,
        A2152 = 0x0C,
        A8018 = 0x10
    };
}


#endif //__HFST_COMMON_HEADER_HPP__
