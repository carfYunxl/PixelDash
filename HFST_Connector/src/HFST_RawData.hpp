#ifndef __HFST_RAW_DATA_HPP__
#define __HFST_RAW_DATA_HPP__

#include "HFST_CommonHeader.hpp"

namespace HFST
{
    enum class RawMode {
        RAW = 0,
        DELTA,
        DIST
    };

    class RawData
    {
    public:
        explicit RawData( const IC_Info& info );



    private:
        RawMode         m_RawMode{ RawMode::DIST };
        const IC_Info&  m_IcInfo;
    };
}

#endif //__HFST_RAW_DATA_HPP__
