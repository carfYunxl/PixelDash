#ifndef __HFST_IRAW_ANALYSIS_HPP__
#define __HFST_IRAW_ANALYSIS_HPP__

namespace HFST
{
    struct IC_Info;
    class IRawFormat
    {
    public:
        IRawFormat() = default;
        IRawFormat(IC_Info* info);

        virtual int  GetMaxBufferLength() = 0;
        virtual int  IndexOfDatatype()          { return 0; }
        virtual int  IndexOfValidDataSize()     { return 1; }
        virtual int  IndexOfTag() = 0;
        virtual int  IndexOfChannel() = 0;
        virtual int  IndexOfData() = 0;

        bool HasTagInfomation()         { return m_bHasTag; }
    protected:
        IC_Info* m_Info{ nullptr };
        bool     m_bHasTag{ false };
    };
}

#endif //__HFST_IRAW_ANALYSIS_HPP__