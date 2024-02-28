#ifndef __HFST_RAW_DATA_HPP__
#define __HFST_RAW_DATA_HPP__

// User header
#include "HFST_CommonHeader.hpp"
#include "HFST_IRawFormat.hpp"

// C++ standard library header
#include <memory>

namespace HFST
{
    struct HFST_API;

    class RawManager
    {
    public:
        RawManager( const IC_Info& info, const HFST_API& api);

        void ReadRaw( RAW::Vec<short>& vecRaw );
        void ReadFrame(  RAW::Frame<short>& frame );

    private:
        RAW::RawMode                    m_RawMode{ RAW::RawMode::DIST };
        const IC_Info&                  m_IcInfo;
        const HFST_API&                 m_Api;

        std::unique_ptr<IRawFormat>     m_RawFormat;
    };
}

#endif //__HFST_RAW_DATA_HPP__
