#include "pch.h"
#include "HFST_RawFormatA2152.hpp"
#include "HFST_CommonHeader.hpp"

namespace HFST
{
    int  A2152_RawFormat::GetReadLength()
    {
        return max(m_Info.nNumX, m_Info.nNumY) * 2 + 5;;
    }
    int  A2152_RawFormat::IndexOfTag() { return 0; }
    int  A2152_RawFormat::IndexOfChannel() { return 2; }
    int  A2152_RawFormat::IndexOfData() { return 4; }
    bool A2152_RawFormat::IsHeader(unsigned char dataType) {
        return dataType == static_cast<unsigned char>(DataType::HEADER);
    }
    bool A2152_RawFormat::IsMutual(unsigned char dataType) {
        return dataType == static_cast<unsigned char>(DataType::MUTUAL);
    }
    bool A2152_RawFormat::IsSelfX(unsigned char dataType) {
        return dataType == static_cast<unsigned char>(DataType::SELF);
    }
    bool A2152_RawFormat::IsSelfXNs(unsigned char dataType) {
        return false;
    }
    bool A2152_RawFormat::IsSelfY(unsigned char dataType) {
        return false;
    }
    bool A2152_RawFormat::IsSelfYNs(unsigned char dataType) {
        return false;
    }
    bool A2152_RawFormat::IsKey(unsigned char dataType) {
        return false;
    }
    bool A2152_RawFormat::IsKeyNs(unsigned char dataType){
        return false;
    }
}
