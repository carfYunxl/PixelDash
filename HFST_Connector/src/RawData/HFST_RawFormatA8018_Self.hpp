#ifndef __HFST_RAW_FORMAT_A8018_SELF_HPP__
#define __HFST_RAW_FORMAT_A8018_SELF_HPP__

#include "HFST_IRawFormat.hpp"

namespace HFST
{
    struct IC_Info;
    class A8018_RawFormat_Self : public IRawFormat
    {
    public:
        A8018_RawFormat_Self(const IC_Info& info);
        ~A8018_RawFormat_Self() {}

        virtual int  GetReadLength() override;
        virtual int  IndexOfTag() override;
        virtual int  IndexOfChannel() override;
        virtual int  IndexOfData() override;
    };
}


#endif //__HFST_RAW_FORMAT_A8018_SELF_HPP__