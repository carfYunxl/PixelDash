#include "pch.h"
#include "HFST_IRawFormat.hpp"
#include "HFST_CommonHeader.hpp"
#include "HFST_RawFormatA8018_Self.hpp"
#include "HFST_RawFormatA8018_Mutual.hpp"
namespace HFST
{
    IRawFormat::IRawFormat( const IC_Info& info )
        : m_Info(info)
    {}

    std::unique_ptr<IRawFormat> IRawFormat::CreateRawFormat(const IC_Info& info)
    {
        switch (info.nChipID )
        {
            case 0x10:
            {
                if (info.bSlfTP)
                    return std::make_unique<A8018_RawFormat_Self>(info);
                else
                    return std::make_unique<A8018_RawFormat_Mutual>(info);
            }
            case 0x06:
                return std::make_unique<A8008_RawFormat>(info);
            case 0x0A:
                return std::make_unique<A8010_RawFormat>(info);
            case 0x0F:
                return std::make_unique<A8015_RawFormat>(info);
            case 0x0C:
                return std::make_unique<A2152_RawFormat>(info);
        }

        return nullptr;
    }
}
