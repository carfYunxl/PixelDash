#ifndef __HFST_RAW_FORMAT_A8018_SELF_HPP__
#define __HFST_RAW_FORMAT_A8018_SELF_HPP__

#include "HFST_IRawFormat.hpp"

namespace HFST
{
    struct IC_Info;
    class A8018_RawFormat_Self : public IRawFormat
    {
        enum class DataType {
            DATA_NOT_READY  = 0x00,
            HEADER          = 0x20,
            SELF_1          = 0X21,
            SELF_2          = 0X22,
            MUTUAL          = 0x23,
            KEY             = 0x24,
            SELF_1_NS       = 0x29,
            SELF_2_NS       = 0x2A,
            MUTUAL_NS       = 0x2B,
            KEY_NS          = 0x2C,

            ALG_SELF_1      = 0XA1,
            ALG_SELF_2      = 0XA2,
            ALG_MUTUAL      = 0xA3,
            ALG_KEY         = 0xA4,
            ALG_SELF_1_NS   = 0xA9,
            ALG_SELF_2_NS   = 0xAA,
            ALG_MUTUAL_NS   = 0xAB,
            ALG_KEY_NS      = 0xAC
        };
    public:
        A8018_RawFormat_Self(const IC_Info& info);
        ~A8018_RawFormat_Self() {}

        virtual int  GetReadLength() override;
        virtual int  IndexOfTag() override;
        virtual int  IndexOfChannel() override;
        virtual int  IndexOfData() override;

        virtual bool IsHeader(unsigned char dataType)   override { return dataType == static_cast<unsigned char>(DataType::HEADER   ); }
        virtual bool IsMutual(unsigned char dataType)   override { return dataType == static_cast<unsigned char>(DataType::MUTUAL   ) || dataType == static_cast<unsigned char>(DataType::ALG_MUTUAL    ); }
        virtual bool IsSelfX(unsigned char dataType)    override { return dataType == static_cast<unsigned char>(DataType::SELF_1   ) || dataType == static_cast<unsigned char>(DataType::ALG_SELF_1    ); }
        virtual bool IsSelfXNs(unsigned char dataType)  override { return dataType == static_cast<unsigned char>(DataType::SELF_1_NS) || dataType == static_cast<unsigned char>(DataType::ALG_SELF_1_NS ); }
        virtual bool IsSelfY(unsigned char dataType)    override { return dataType == static_cast<unsigned char>(DataType::SELF_2   ) || dataType == static_cast<unsigned char>(DataType::ALG_SELF_2    ); }
        virtual bool IsSelfYNs(unsigned char dataType)  override { return dataType == static_cast<unsigned char>(DataType::SELF_2_NS) || dataType == static_cast<unsigned char>(DataType::ALG_SELF_2_NS ); }
        virtual bool IsKey(unsigned char dataType)      override { return dataType == static_cast<unsigned char>(DataType::KEY      ) || dataType == static_cast<unsigned char>(DataType::ALG_KEY       ); }
        virtual bool IsKeyNs(unsigned char dataType)    override { return dataType == static_cast<unsigned char>(DataType::KEY_NS   ) || dataType == static_cast<unsigned char>(DataType::ALG_KEY_NS    ); }
    };
}


#endif //__HFST_RAW_FORMAT_A8018_SELF_HPP__