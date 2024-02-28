#ifndef __HFST_COMMAND_IO_HPP__
#define __HFST_COMMAND_IO_HPP__

#include "HFST_CommonHeader.hpp"

namespace HFST
{
    struct HFST_API;

    class CommandIO
    {
    public:
        CommandIO(const HFST_API& api);

    private:
        bool SetCommandReady();
        bool IsCommandFinished();
    private:
        const HFST_API& m_Api;
    };
}

#endif //__HFST_COMMAND_IO_HPP__
