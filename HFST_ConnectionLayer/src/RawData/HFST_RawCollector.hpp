#ifndef __HFST_RAW_COLLECTOR_HPP__
#define __HFST_RAW_COLLECTOR_HPP__

// User header
#include "HFST_CommonHeader.hpp"
#include "HFST_IRawFormat.hpp"

// C++ standard library header
#include <memory>

namespace HFST
{
    class RawCollector
    {
    public:
        explicit RawCollector( const IC_Info& info);

        int ReadChannelRaw( RAW::ChannelRaw<short>& channel );
        int ReadFrame(  RAW::Frame<short>& frame );

    private:
        RAW::TAG_TYPE ConvertToTagType(uchar nTagValue);

    private:
        RAW::RawMode                    m_RawMode{ RAW::RawMode::DIST };
        const IC_Info&                  m_IcInfo;

        std::unique_ptr<IRawFormat>     m_RawFormat;
    };
}

#endif //__HFST_RAW_COLLECTOR_HPP__
