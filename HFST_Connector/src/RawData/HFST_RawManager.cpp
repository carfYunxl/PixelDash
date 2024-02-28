#include "pch.h"
#include "HFST_RawManager.hpp"

namespace HFST
{
    RawManager::RawManager(const IC_Info& info, const HFST_API& api)
        : m_IcInfo(info)
        , m_Api(api)
        , m_RawFormat(std::make_unique<IRawFormat>(info.nChipID)){
    }

    // read one line raw data
    void RawManager::ReadRaw( RAW::Vec<short>& vecRaw )
    {
        int nBufferLen = m_RawFormat->GetMaxBufferLength();
        unsigned char* arrBuffer = new unsigned char[nBufferLen];

        int ret = m_Api.TTK.ReadI2CReg(arrBuffer, ADDR_MAP::RAW, nBufferLen);
        if (ret <= 0)
        {
            // something goes wrong, TODO
        }

        delete[] arrBuffer;
    }

    void RawManager::ReadFrame( RAW::Frame<short>& frame )
    {

    }
}
