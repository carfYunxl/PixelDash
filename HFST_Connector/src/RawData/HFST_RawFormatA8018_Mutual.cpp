#include "pch.h"
#include "HFST_RawFormatA8018_Mutual.hpp"
#include "HFST_CommonHeader.hpp"

namespace HFST
{
    A8018_RawFormat_Mutual::A8018_RawFormat_Mutual(const IC_Info& info)
        : IRawFormat(info)
    {
        if ( info.TagTypeSel != 0 )
            m_bHasTag = true;
    }

    int  A8018_RawFormat_Mutual::GetReadLength()
    {
        int nReadLen = 0;

        int nMaxChanel = max( m_Info.nNumX, m_Info.nNumY );
        if (m_Info.TagTypeSel == 0)
            nReadLen = nMaxChanel * 2 + 5;
        else
            nReadLen = nMaxChanel * 2 + 6;

        return nReadLen;
    }

    int  A8018_RawFormat_Mutual::IndexOfTag()
    {
        if ( m_bHasTag )
        {
            return 4;
        }

        return -1;
    }

    int  A8018_RawFormat_Mutual::IndexOfChannel()
    {
        return 2;
    }

    int  A8018_RawFormat_Mutual::IndexOfData()
    {
        if (m_Info.TagTypeSel == 0)
            return 4;
        else
            return 5;
    }
}
