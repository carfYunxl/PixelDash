#ifndef __HFST_RAWDATA_READER_HPP__
#define __HFST_RAWDATA_READER_HPP__

#include "HFST_CommonHeader.hpp"
#include <memory>

namespace HFST
{
    // virtual base class interface
    class RawReader
    {
    public:
        explicit RawReader(const IC_Info& info) : m_Info(info) {}
        virtual ~RawReader() {}

        virtual int ReadChannelRaw(RAW::ChannelRaw<short>& channel) = 0;
        virtual int ReadFrame(RAW::Frame<short>& frame) = 0;

        template<typename Stream>
        void PrintChannel( Stream& stream, RAW::ChannelRaw<short>& channel )
        {
            std::string str = std::format("{: <4X}  {: >4d}  {: >4d}  ", channel.nDataType, channel.nChannelIdx, static_cast<int>(channel.Type));
            for (const auto& data : channel.vecRaw)
            {
                str += std::format("{: >4d}  ", data);
            }
            stream << str << "\n";
        }

        template<typename Stream>
        void PrintFrame( Stream& stream, RAW::Frame<short>& frame )
        {
            
        }
    protected:
        const IC_Info& m_Info;
    };

    // CRTP base class
    template<typename Derived>
    class RawReaderBase : public RawReader
    {
    public:
        RawReaderBase(const IC_Info& info) :RawReader(info) {}
        ~RawReaderBase() {}

        virtual int ReadChannelRaw(RAW::ChannelRaw<short>& channel) override
        {
            auto pApi = HFST_API::GetAPI();
            if (!pApi)
                return -1;

            Derived* RawFormat = static_cast<Derived*>(this);

            int nBufferLen = RawFormat->GetReadLength();
            std::unique_ptr<unsigned char[]> pBuffer = std::make_unique<unsigned char[]>(nBufferLen);

            int ret = pApi->TTK.ReadI2CReg(pBuffer.get(), ADDR_MAP::RAW, nBufferLen);
            if (ret <= 0) {
                return ret;
            }

            if (pBuffer[RawFormat->IndexOfDatatype()] == 0)
                return -1;

            channel.nDataType = pBuffer[RawFormat->IndexOfDatatype()];

            if ( m_Info.TagTypeSel == 1 )
            {
                channel.Type = ConvertToTagType(pBuffer[RawFormat->IndexOfTag()]);
            }
            // 20 0E 01 00 01 11 22 33 44 55 66 77 88 99 aa xx
            int nDataSize = pBuffer[RawFormat->IndexOfValidDataSize()];     // 0x0E : 14
            channel.nChannelIdx = pBuffer[RawFormat->IndexOfChannel()];     // 01
            int nDataIdx = RawFormat->IndexOfData();                        // 5

            int nLen = nDataSize - nDataIdx + 1;

            if (RawFormat->IsHeader(channel.nDataType))
            {
                channel.vecRaw.resize(nLen);

                for (int i = 0; i < nLen; i++)
                {
                    channel.vecRaw[i] = pBuffer[nDataIdx + i];
                }
            }
            else
            {
                channel.vecRaw.resize(nLen / 2);

                for (int i = 0; i < nLen / 2; i++)
                {
                    channel.vecRaw[i] = ((pBuffer[nDataIdx + 2 * i] << 8) | pBuffer[nDataIdx + 2 * i + 1]);
                }
            }

            return 1;
        }

        RAW::TAG_TYPE ConvertToTagType(uchar nTagValue)
        {
            bool bAG = ((nTagValue & 0x10) == 1) ? true : false;
            bool bFlush = ((nTagValue & 0x20) == 1) ? true : false;

            if ((bAG & bFlush) == 1)
                return RAW::TAG_TYPE::AG_FLUSH;

            if ((bAG | bFlush) == 0)
                return RAW::TAG_TYPE::GND;

            if (bAG)
                return RAW::TAG_TYPE::AG;
            else
                return RAW::TAG_TYPE::GND_FLUSH;
        }

        virtual int ReadFrame(RAW::Frame<short>& frame) override
        {
            Derived* RawFormat = static_cast<Derived*>(this);
            RAW::ChannelRaw<short> Raw;
            while (1)
            {
                Raw.vecRaw.clear();
                int ret = ReadChannelRaw(Raw);
                if (ret <= 0) {
                    return ret;
                }

                if (RawFormat->IsHeader(Raw.nDataType))
                {
                    if ( !frame.vctHeader.vecRaw.empty() )
                        break;

                    frame.vctHeader = Raw;
                }
                else if (RawFormat->IsMutual(Raw.nDataType))
                {
                    if ( !frame.vctMutual.empty() )
                        break;

                    frame.vctMutual.push_back(Raw);
                }
                else if (RawFormat->IsSelfX(Raw.nDataType))
                {
                    if ( !frame.vctXSelf.empty() )
                        break;
                    frame.vctXSelf.push_back(Raw);
                }
                else if (RawFormat->IsSelfY(Raw.nDataType))
                {
                    if (!frame.vctYSelf.empty())
                        break;
                    frame.vctYSelf.push_back(Raw);
                }
                else if (RawFormat->IsSelfXNs(Raw.nDataType))
                {
                    if (!frame.vctXSelfNs.empty())
                        break;
                    frame.vctXSelfNs.push_back(Raw);
                }
                else if (RawFormat->IsSelfYNs(Raw.nDataType))
                {
                    if (!frame.vctYSelfNs.empty())
                        break;
                    frame.vctYSelfNs.push_back(Raw);
                }
                else if (RawFormat->IsKey(Raw.nDataType))
                {
                    if (!frame.vctKey.vecRaw.empty())
                        break;
                    frame.vctKey = Raw;
                }
                else if (RawFormat->IsKeyNs(Raw.nDataType))
                {
                    if (!frame.vctKeyNs.vecRaw.empty())
                        break;
                    frame.vctKeyNs = Raw;
                }
                else
                    continue;
            }

            return 1;
        }
    };

    class RawReader_A8018_Mutual final : public RawReaderBase<RawReader_A8018_Mutual>
    {
        enum class DataType {
            DATA_NOT_READY = 0x00,
            HEADER = 0x20,
            SELF_1 = 0X21,
            SELF_2 = 0X22,
            MUTUAL = 0x23,
            KEY = 0x24,
            SELF_1_NS = 0x29,
            SELF_2_NS = 0x2A,
            MUTUAL_NS = 0x2B,
            KEY_NS = 0x2C,

            ALG_SELF_1 = 0XA1,
            ALG_SELF_2 = 0XA2,
            ALG_MUTUAL = 0xA3,
            ALG_KEY = 0xA4,
            ALG_SELF_1_NS = 0xA9,
            ALG_SELF_2_NS = 0xAA,
            ALG_MUTUAL_NS = 0xAB,
            ALG_KEY_NS = 0xAC
        };
    public:
        RawReader_A8018_Mutual(const IC_Info& info) :RawReaderBase<RawReader_A8018_Mutual>(info) {}
        int GetReadLength() {
            int nReadLen = 0;

            int nMaxChanel = max(m_Info.nNumX, m_Info.nNumY);
            if (m_Info.TagTypeSel == 0)
                nReadLen = nMaxChanel * 2 + 5;
            else
                nReadLen = nMaxChanel * 2 + 6;

            return nReadLen;
        }
        int  IndexOfTag() {
            if (m_Info.TagTypeSel == 0)
            {
                return 4;
            }

            return -1;
        }
        int  IndexOfChannel() { return 2; }

        int  IndexOfData() {
            if (m_Info.TagTypeSel == 0)
                return 4;
            else
                return 5;
        }

        int  IndexOfDatatype() { return 0; }
        int  IndexOfValidDataSize() { return 1; }

        bool IsHeader(unsigned char dataType)  { return dataType == static_cast<unsigned char>(DataType::HEADER); }
        bool IsMutual(unsigned char dataType)  { return dataType == static_cast<unsigned char>(DataType::MUTUAL) || dataType == static_cast<unsigned char>(DataType::ALG_MUTUAL); }
        bool IsSelfX(unsigned char dataType)   { return dataType == static_cast<unsigned char>(DataType::SELF_1) || dataType == static_cast<unsigned char>(DataType::ALG_SELF_1); }
        bool IsSelfXNs(unsigned char dataType) { return dataType == static_cast<unsigned char>(DataType::SELF_1_NS) || dataType == static_cast<unsigned char>(DataType::ALG_SELF_1_NS); }
        bool IsSelfY(unsigned char dataType)   { return dataType == static_cast<unsigned char>(DataType::SELF_2) || dataType == static_cast<unsigned char>(DataType::ALG_SELF_2); }
        bool IsSelfYNs(unsigned char dataType) { return dataType == static_cast<unsigned char>(DataType::SELF_2_NS) || dataType == static_cast<unsigned char>(DataType::ALG_SELF_2_NS); }
        bool IsKey(unsigned char dataType)     { return dataType == static_cast<unsigned char>(DataType::KEY) || dataType == static_cast<unsigned char>(DataType::ALG_KEY); }
        bool IsKeyNs(unsigned char dataType)   { return dataType == static_cast<unsigned char>(DataType::KEY_NS) || dataType == static_cast<unsigned char>(DataType::ALG_KEY_NS); }
    };

    class RawReader_A8018_Self : public RawReaderBase<RawReader_A8018_Self>
    {
        enum class DataType {
            DATA_NOT_READY = 0x00,
            HEADER = 0x20,
            SELF_1 = 0X21,
            SELF_2 = 0X22,
            MUTUAL = 0x23,
            KEY = 0x24,
            SELF_1_NS = 0x29,
            SELF_2_NS = 0x2A,
            MUTUAL_NS = 0x2B,
            KEY_NS = 0x2C,

            ALG_SELF_1 = 0XA1,
            ALG_SELF_2 = 0XA2,
            ALG_MUTUAL = 0xA3,
            ALG_KEY = 0xA4,
            ALG_SELF_1_NS = 0xA9,
            ALG_SELF_2_NS = 0xAA,
            ALG_MUTUAL_NS = 0xAB,
            ALG_KEY_NS = 0xAC
        };
    public:
        RawReader_A8018_Self(const IC_Info& info) :RawReaderBase<RawReader_A8018_Self>(info) {}

        int  GetReadLength() {
            return m_Info.TagTypeSel == 0 ? (m_Info.nValidSelfLen * 2 + 5) : (m_Info.nValidSelfLen * 2 + 6);
        }
        int  IndexOfTag() {
            return m_Info.TagTypeSel == 0 ? 4 : -1;
        }
        int  IndexOfChannel() { return 2; }

        int  IndexOfData() { return m_Info.TagTypeSel == 0 ? 4 : 5; }

        int  IndexOfDatatype() { return 0; }

        int  IndexOfValidDataSize() { return 1; }

        bool IsHeader(unsigned char dataType)  { return dataType == static_cast<unsigned char>(DataType::HEADER); }
        bool IsMutual(unsigned char dataType)  { return dataType == static_cast<unsigned char>(DataType::MUTUAL) || dataType == static_cast<unsigned char>(DataType::ALG_MUTUAL); }
        bool IsSelfX(unsigned char dataType)   { return dataType == static_cast<unsigned char>(DataType::SELF_1) || dataType == static_cast<unsigned char>(DataType::ALG_SELF_1); }
        bool IsSelfXNs(unsigned char dataType) { return dataType == static_cast<unsigned char>(DataType::SELF_1_NS) || dataType == static_cast<unsigned char>(DataType::ALG_SELF_1_NS); }
        bool IsSelfY(unsigned char dataType)   { return dataType == static_cast<unsigned char>(DataType::SELF_2) || dataType == static_cast<unsigned char>(DataType::ALG_SELF_2); }
        bool IsSelfYNs(unsigned char dataType) { return dataType == static_cast<unsigned char>(DataType::SELF_2_NS) || dataType == static_cast<unsigned char>(DataType::ALG_SELF_2_NS); }
        bool IsKey(unsigned char dataType)     { return dataType == static_cast<unsigned char>(DataType::KEY) || dataType == static_cast<unsigned char>(DataType::ALG_KEY); }
        bool IsKeyNs(unsigned char dataType)   { return dataType == static_cast<unsigned char>(DataType::KEY_NS) || dataType == static_cast<unsigned char>(DataType::ALG_KEY_NS); }
    };

    class RawReader_A2152 final : public RawReaderBase<RawReader_A2152>
    {
        enum class DataType
        {
            HEADER = 0x09,
            MUTUAL = 0x06,
            SELF = 0x04,
            NOISE = 0x08
        };
    public:
        RawReader_A2152(const IC_Info& info)
            : RawReaderBase<RawReader_A2152>(info)
        {}
        ~RawReader_A2152() {}

        int  GetReadLength() {
            return max(m_Info.nNumX, m_Info.nNumY) * 2 + 5;
        }
        int  IndexOfTag() { return 0; }
        int  IndexOfChannel() { return 2; }
        int  IndexOfData() { return 4; }

        int  IndexOfDatatype() { return 0; }
        int  IndexOfValidDataSize() { return 1; }

        bool IsHeader(unsigned char dataType)   { return dataType == static_cast<unsigned char>(DataType::HEADER); }
        bool IsMutual(unsigned char dataType)   { return dataType == static_cast<unsigned char>(DataType::MUTUAL); }
        bool IsSelfX(unsigned char dataType)    { return dataType == static_cast<unsigned char>(DataType::SELF); }
        bool IsSelfXNs(unsigned char dataType)  { return false; }
        bool IsSelfY(unsigned char dataType)    { return false; }
        bool IsSelfYNs(unsigned char dataType)  { return false; }
        bool IsKey(unsigned char dataType)      { return false; }
        bool IsKeyNs(unsigned char dataType)    { return false; }
    };

    static std::unique_ptr<RawReader> CreateRawReader(const IC_Info& info)
    {
        switch ( static_cast<ChipID>(info.nChipID) )
        {
            case ChipID::A8018:
            {
                if (info.bSlfTP)
                    return std::make_unique<RawReader_A8018_Self>(info);

                return std::make_unique<RawReader_A8018_Mutual>(info);
            }
            case ChipID::A2152:
            {
                return std::make_unique<RawReader_A2152>(info);
            }
        }

        return nullptr;
    }
}

#endif //__HFST_RAWDATA_READER_HPP__
