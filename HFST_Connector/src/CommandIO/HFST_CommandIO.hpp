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

    class CommandIO
    {
    public:
        CommandIO(const HFST_API& api);

        void Read(int type, int address, unsigned char* buf, int len, unsigned char ChipId);
        void Write(int type, int address, unsigned char* buf, int len, unsigned char ChipId);
    private:
        bool SetCommandReady();
        bool GetCommandReady();
        bool Read_Packet( CommandIO_Packet& packet );
        bool Write_Packet( const CommandIO_Packet& packet );
    private:
        const HFST_API& m_Api;
    };
}

#endif //__HFST_COMMAND_IO_HPP__
