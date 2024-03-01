#include "pch.h"
#include "HFST_RawFormatA8018_Mutual.hpp"
#include "HFST_CommonHeader.hpp"

namespace HFST
{
    A8018_RawFormat_Mutual::A8018_RawFormat_Mutual(IC_Info* info)
        : IRawFormat(info) {
        auto pInfo = dynamic_cast<A8018_IC_Info*>(m_Info);
        if (pInfo == nullptr)
        {
            if (pInfo->TagTypeSel != 0)
                m_bHasTag = true;
        }
    }

    int  A8018_RawFormat_Mutual::GetMaxBufferLength()
    {

    }

    int  A8018_RawFormat_Mutual::IndexOfTag()
    {

    }
    int  A8018_RawFormat_Mutual::IndexOfChannel()
    {
        
    }
    int  A8018_RawFormat_Mutual::IndexOfData()
    {

    }
}
