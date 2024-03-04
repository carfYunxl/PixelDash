#include "pch.h"
#include "HFST_RawCollector.hpp"

namespace HFST
{
    RawCollector::RawCollector(const IC_Info& info)
        : m_IcInfo(info)
        , m_RawFormat(IRawFormat::CreateRawFormat(info)){
    }

    int RawCollector::ReadChannelRaw( RAW::ChannelRaw<short>& channel )
    {
        auto pApi = HFST_API::GetAPI();
        if (!pApi)
            return -1;

        int nBufferLen = m_RawFormat->GetReadLength();
        std::unique_ptr<unsigned char[]> pBuffer = std::make_unique<unsigned char[]>(nBufferLen);

        int ret = pApi->TTK.ReadI2CReg( pBuffer.get(), ADDR_MAP::RAW, nBufferLen );
        if (ret <= 0)
        {
            return ret;
        }

        channel.nDataType = pBuffer[m_RawFormat->IndexOfDatatype()];

        if ( m_RawFormat->HasTagInfomation() )
        {
            channel.Type = ConvertToTagType(pBuffer[m_RawFormat->IndexOfTag()] );
        }
        // 20 0E 01 00 01 11 22 33 44 55 66 77 88 99 aa xx
        int nDataSize = pBuffer[m_RawFormat->IndexOfValidDataSize()];     // 0x0E : 14
        channel.nChannelIdx = pBuffer[m_RawFormat->IndexOfChannel()];     // 01
        int nDataIdx = m_RawFormat->IndexOfData();                        // 5

        int nLen = nDataSize - nDataIdx + 1;
        channel.vecRaw.resize( nLen );
        for ( int i=0; i < nLen; i+=2 )
        {
            channel.vecRaw[i] = ((pBuffer[nDataSize+i] << 8) | pBuffer[nDataSize+i+1]);
        }

        return 1;
    }

    int RawCollector::ReadFrame( RAW::Frame<short>& frame )
    {
        return 1;
    }

    RAW::TAG_TYPE ConvertToTagType(uchar nTagValue)
    {
        bool bAG =      ( (nTagValue & 0x10) == 1 ) ? true : false;
        bool bFlush =   ( (nTagValue & 0x20) == 1 ) ? true : false;

        if ( (bAG & bFlush) == 1)
            return RAW::TAG_TYPE::AG_FLUSH;

        if ( (bAG | bFlush) == 0)
            return RAW::TAG_TYPE::GND;

        if (bAG)
            return RAW::TAG_TYPE::AG;
        else
            return RAW::TAG_TYPE::GND_FLUSH;
    }
}
