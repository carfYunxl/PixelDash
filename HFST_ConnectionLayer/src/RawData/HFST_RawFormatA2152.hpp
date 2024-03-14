#ifndef __HFST_RAW_FORMAT_A2152_HPP_
#define __HFST_RAW_FORMAT_A2152_HPP_

#include "HFST_IRawFormat.hpp"

namespace HFST
{
    class A2152_RawFormat : public IRawFormat
    {
        enum class DataType
        {
            HEADER  = 0x09,
            MUTUAL  = 0x06,
            SELF    = 0x04,
            NOISE   = 0x08
        };
    public:
        A2152_RawFormat(const IC_Info& info)
            : IRawFormat(info)
        {}
        ~A2152_RawFormat() {}

        virtual int  GetReadLength() override;
        virtual int  IndexOfTag() override;
        virtual int  IndexOfChannel() override;
        virtual int  IndexOfData() override;

        virtual bool IsHeader(unsigned char dataType)   override;
        virtual bool IsMutual(unsigned char dataType)   override;
        virtual bool IsSelfX(unsigned char dataType)    override;
        virtual bool IsSelfXNs(unsigned char dataType)  override;
        virtual bool IsSelfY(unsigned char dataType)    override;
        virtual bool IsSelfYNs(unsigned char dataType)  override;
        virtual bool IsKey(unsigned char dataType)      override;
        virtual bool IsKeyNs(unsigned char dataType)    override;
    };
}

#endif //__HFST_RAW_FORMAT_A2152_HPP_
