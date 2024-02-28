#ifndef __HFST_IRAW_ANALYSIS_HPP__
#define __HFST_IRAW_ANALYSIS_HPP__

namespace HFST
{
    class IRawFormat
    {
    public:
        IRawFormat() = default;
        IRawFormat(int nID);

        virtual int  GetMaxBufferLength() = 0;
        virtual int  IndexOfDatatype() { return 0; }
        virtual int  IndexOfValidDataSize() { return 1; }
        virtual bool HasTagInfomation() { return false; }
        virtual int  IndexOfData() = 0;
    private:
    };

    class A8008_RawFormat : public IRawFormat
    {};

    class A8018_RawFormat_Mutual : public IRawFormat
    {
    public:
        A8018_RawFormat_Mutual();

        virtual int  GetMaxBufferLength() override { return 25; }
        virtual bool HasTagInfomation() override { return true; }
        virtual int  IndexOfData() override;
    };

    class A8018_RawFormat_Self : public IRawFormat
    {
    public:
        A8018_RawFormat_Self();

        virtual int  GetMaxBufferLength() override;
        virtual int  IndexOfDatatype() override;
        virtual int  IndexOfValidDataSize() override;
        virtual bool HasTagInfomation() override;
        virtual int  IndexOfData() override;
    };
}

#endif //__HFST_IRAW_ANALYSIS_HPP__