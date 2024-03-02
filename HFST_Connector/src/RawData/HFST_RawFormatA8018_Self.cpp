#include "pch.h"
#include "HFST_RawFormatA8018_Self.hpp"
#include "HFST_CommonHeader.hpp"

namespace HFST
{
    A8018_RawFormat_Self::A8018_RawFormat_Self(const IC_Info& info)
        : IRawFormat(info) {
        if (info.TagTypeSel != 0)
            m_bHasTag = true;
    }

    int  A8018_RawFormat_Self::GetReadLength()
    {
        int nReadLen = 0;

        int nMaxChanel = m_Info.nValidSelfLen;

        if (m_Info.TagTypeSel == 0)
            nReadLen = nMaxChanel * 2 + 5;
        else
            nReadLen = nMaxChanel * 2 + 6;

        return nReadLen;
    }

    int  A8018_RawFormat_Self::IndexOfTag()
    {
        if (m_bHasTag)
        {
            return 4;
        }

        return -1;
    }
    int  A8018_RawFormat_Self::IndexOfChannel()
    {
        return 2;
    }
    int  A8018_RawFormat_Self::IndexOfData()
    {
        return m_Info.TagTypeSel == 0 ? 4 : 5;
    }
}
