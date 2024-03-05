#ifndef __HFST_IRAW_ANALYSIS_HPP__
#define __HFST_IRAW_ANALYSIS_HPP__

#include <memory>

namespace HFST
{
    struct IC_Info;
    class IRawFormat
    {
    public:
        IRawFormat(const IC_Info& info);
        virtual ~IRawFormat() {}

        virtual int  GetReadLength() = 0;
        virtual int  IndexOfDatatype()          { return 0; }
        virtual int  IndexOfValidDataSize()     { return 1; }
        virtual int  IndexOfTag() = 0;
        virtual int  IndexOfChannel() = 0;
        virtual int  IndexOfData() = 0;

        bool         HasTagInfomation()         { return m_bHasTag; }

    public:
        static std::unique_ptr<IRawFormat> CreateRawFormat(const IC_Info& info);
    protected:
        const       IC_Info& m_Info;
        bool        m_bHasTag{ false };
    };


    class A8008_RawFormat : public IRawFormat
    {
    public:
        A8008_RawFormat(const IC_Info& info)
            : IRawFormat(info)
        {}
        ~A8008_RawFormat() {}

        virtual int  GetReadLength() override { return 0; }
        virtual int  IndexOfTag() override { return 0; }
        virtual int  IndexOfChannel() override { return 0; }
        virtual int  IndexOfData() override { return 0; }
    };

    class A8010_RawFormat : public IRawFormat
    {
    public:
        A8010_RawFormat(const IC_Info& info)
            : IRawFormat(info)
        {}
        ~A8010_RawFormat() {}

        virtual int  GetReadLength() override { return 0; }
        virtual int  IndexOfTag() override { return 0; }
        virtual int  IndexOfChannel() override { return 0; }
        virtual int  IndexOfData() override { return 0; }
    };

    class A8015_RawFormat : public IRawFormat
    {
    public:
        A8015_RawFormat(const IC_Info& info)
            : IRawFormat(info)
        {}
        ~A8015_RawFormat() {}

        virtual int  GetReadLength() override { return 0; }
        virtual int  IndexOfTag() override { return 0; }
        virtual int  IndexOfChannel() override { return 0; }
        virtual int  IndexOfData() override { return 0; }
    };

    class A2152_RawFormat : public IRawFormat
    {
    public:
        A2152_RawFormat(const IC_Info& info)
            : IRawFormat(info)
        {}
        ~A2152_RawFormat() {}

        virtual int  GetReadLength() override { return 0; }
        virtual int  IndexOfTag() override { return 0; }
        virtual int  IndexOfChannel() override { return 0; }
        virtual int  IndexOfData() override { return 0; }
    };
}

#endif //__HFST_IRAW_ANALYSIS_HPP__