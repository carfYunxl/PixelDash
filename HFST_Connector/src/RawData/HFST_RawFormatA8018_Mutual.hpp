#ifndef __HFST_RAW_FORMAT_A8018_MUTUAL_HPP__
#define __HFST_RAW_FORMAT_A8018_MUTUAL_HPP__
#include "HFST_IRawFormat.hpp"

namespace HFST
{
    struct IC_Info;
    class A8018_RawFormat_Mutual : public IRawFormat
    {
    public:
        A8018_RawFormat_Mutual(IC_Info* info);

        virtual int  GetMaxBufferLength() override;
        virtual int  IndexOfTag() override;
        virtual int  IndexOfChannel() override;
        virtual int  IndexOfData() override;
    };
}


#endif //__HFST_RAW_FORMAT_A8018_MUTUAL_HPP__
