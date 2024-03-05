#ifndef __HFST_COMMAND_IO_HPP__
#define __HFST_COMMAND_IO_HPP__

#include "HFST_CommonHeader.hpp"

namespace HFST
{
    struct CommandIO_Packet
    {
        unsigned char nCmdID;
        unsigned char nDataSize;
        unsigned char Data[COMMAND_IO::CMDIO_PACK_SIZE];
    };

    using COMMAND_IO::MEMORY_TYPE;

    class CommandIO
    {
    public:
        explicit CommandIO(IC_Info& info);

        int Read(
            MEMORY_TYPE nMemType, 
            int nAddr,
            unsigned char* buf,
            int len
        );

        int Write(
            MEMORY_TYPE nMemType,
            int nAddr,
            unsigned char* buf,
            int len
        );

        bool GetInfo();
    private:
        bool SetCommandReady();
        bool GetCommandReady();
        bool Read_Packet( CommandIO_Packet& packet );
        bool Write_Packet( const CommandIO_Packet& packet );
    private:
        IC_Info&    m_Info;
    };
}

#endif //__HFST_COMMAND_IO_HPP__
