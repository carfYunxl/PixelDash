#ifndef __HFST_RAW_COLLECTOR_HPP__
#define __HFST_RAW_COLLECTOR_HPP__

// User header
#include "HFST_CommonHeader.hpp"
#include "HFST_IRawFormat.hpp"

// C++ standard library header
#include <memory>

namespace HFST
{
    struct HFST_API;

    class RawCollector
    {
    public:
        RawCollector( const IC_Info& info, const HFST_API& api);

        int ReadChannelRaw( RAW::ChannelRaw<short>& channel );
        int ReadFrame(  RAW::Frame<short>& frame );

    private:
        RAW::TAG_TYPE ConvertToTagType(uchar nTagValue);

    private:
        RAW::RawMode                    m_RawMode{ RAW::RawMode::DIST };
        const IC_Info&                  m_IcInfo;
        const HFST_API&                 m_Api;

        std::unique_ptr<IRawFormat>     m_RawFormat;
    };
}

#endif //__HFST_RAW_COLLECTOR_HPP__
