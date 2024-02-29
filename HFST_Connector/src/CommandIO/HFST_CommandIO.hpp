#ifndef __HFST_COMMAND_IO_HPP__
#define __HFST_COMMAND_IO_HPP__

#include "HFST_CommonHeader.hpp"

namespace HFST
{
    struct HFST_API;

    struct CommandIO_Packet
    {
        int nCmdID;
        int nDataSize;
        unsigned char Data[COMMAND_IO::CMDIO_PACK_SIZE];
    };

    using COMMAND_IO::MEMORY_TYPE;

    class CommandIO
    {
    public:
        CommandIO(const HFST_API& api, ChipID nChipID);

        bool Read(
            MEMORY_TYPE nMemType, 
            int nAddr,
            unsigned char* buf,
            int len
        );
        bool Write(
            MEMORY_TYPE nMemType,
            int nAddr,
            unsigned char* buf,
            int len
        );
    private:
        bool SetCommandReady();
        bool GetCommandReady();
        bool Read_Packet( CommandIO_Packet& packet );
        bool Write_Packet( const CommandIO_Packet& packet );

        void BuildPacket();
    private:
        const HFST_API& m_Api;
        ChipID          m_ChipID;
    };
}

#endif //__HFST_COMMAND_IO_HPP__
