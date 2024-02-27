#ifndef __HFST_TOUCHDEVICE_HPP__
#define __HFST_TOUCHDEVICE_HPP__

#include <iostream>

namespace HFST
{
    template<size_t ID>
    struct TouchDevice {
    };

    template<>
    struct TouchDevice<0x06>
    {
        static constexpr size_t ChipID          = 0x06;
        static constexpr size_t ISP_PageSize    = 0x400;
        static constexpr size_t FW_Addr         = 0x00;
        static constexpr size_t CFG_Addr        = 0x3F00;
        static constexpr size_t FW_Length       = 16 * 0x400 - 0x100;
        static constexpr size_t CFG_Length      = 0x100 - 2;
        static constexpr size_t Flash_Length    = 17 * 0x400;

        bool GetCFG(unsigned char* cfg) {
            // TO DO
            return true;
        }

        bool IsFwChecksumOK(unsigned char* fw, int protocol)
        {
            // TO DO
            return true;
        }

        bool JumpToSWISP(int protocol)
        {
            // TO DO
            return true;
        }
    };

    template<>
    struct TouchDevice<0x0F>
    {
        static constexpr size_t ChipID          = 0x0F;
        static constexpr size_t ISP_PageSize    = 0x400;
        static constexpr size_t FW_Addr         = 0x00;
        static constexpr size_t CFG_Addr        = 0x3F00;
        static constexpr size_t FW_Length       = 0x7E80;
        static constexpr size_t CFG_Length      = 0x100 - 2;
        static constexpr size_t Flash_Length    = 32 * 0x400;

        bool GetCFG(unsigned char* cfg)
        {
            std::cout << "Touch Device 0x0F" << std::endl;
            return true;
        }

        bool IsFwChecksumOK(unsigned char* fw, int protocol)
        {
            return true;
        }

        bool JumpToSWISP(int protocol)
        {
            // TO DO
            return true;
        }
    };

    template<>
    struct TouchDevice<0x0A>
    {
        static constexpr size_t ChipID          = 0x0A;
        static constexpr size_t ISP_PageSize    = 0x400;
        static constexpr size_t FW_Addr         = 0x00;
        static constexpr size_t CFG_Addr        = 0xBC00;
        static constexpr size_t FW_Length       = 47 * 0x400;
        static constexpr size_t CFG_Length      = 704;
        static constexpr size_t Flash_Length    = 64 * 0x400;

        bool GetCFG(unsigned char* cfg)
        {
            std::cout << "Touch Device 0x0A" << std::endl;
            return true;
        }

        bool IsFwChecksumOK(unsigned char* fw, int protocol)
        {
            return true;
        }

        bool JumpToSWISP(int protocol)
        {
            // TO DO
            return true;
        }
    };

    template<>
    struct TouchDevice<0x0C>
    {
        static constexpr size_t ChipID          = 0x0C;
        static constexpr size_t ISP_PageSize    = 4 * 0x400;
        static constexpr size_t FW_Addr         = 0x00;
        static constexpr size_t CFG_Addr        = 0xBC00;
        static constexpr size_t FW_Length       = 0xF000;
        static constexpr size_t CFG_Length      = 2110;
        static constexpr size_t Flash_Length    = 128 * 0x400;

        bool GetCFG(unsigned char* cfg)
        {
            std::cout << "Touch Device 0x0C" << std::endl;
            return true;
        }

        bool IsFwChecksumOK(unsigned char* fw, int protocol)
        {
            return true;
        }

        bool JumpToSWISP(int protocol)
        {
            // TO DO
            return true;
        }
    };

    template<>
    struct TouchDevice<0x10>
    {
        static constexpr size_t ChipID = 0x10;
        static constexpr size_t ISP_PageSize = 0x400;
        static constexpr size_t FW_Addr = 0x00;
        static constexpr size_t CFG_Addr = 0xBA00;
        static constexpr size_t FW_Length = 0xC000;
        static constexpr size_t CFG_Length = 0x600;
        static constexpr size_t Flash_Length = 0xFC00;

        bool GetCFG(unsigned char* cfg)
        {
            std::cout << "Touch Device 0x10" << std::endl;
            return true;
        }

        bool IsFwChecksumOK(unsigned char* fw, int protocol)
        {
            return true;
        }

        bool JumpToSWISP(int protocol)
        {
            // TO DO
            return true;
        }
    };
}

#endif //__HFST_TOUCHDEVICE_HPP__
