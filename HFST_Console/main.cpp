#include <windows.h>
#include <iostream>
#include <format>
#include "HFST_Connector.hpp"
#include "HFST_RawReader.hpp"
#include "HFST_CommonHeader.hpp"
#include "HFST_Bridge.hpp"

int main(int argc, char* argv[])
{
    HFST::Connector connector;
    if (!connector.Connect())
    {
        std::cout << "Connect failed!" << std::endl;
        system("pause");
        return 0;
    }

    HFST::IC_Info info = connector.IC_GetInfo();
    HFST::RAW::ChannelRaw<short> raw;

    HFST::RawReader* pRawReader = new HFST::RawReader_A8018_Mutual(info);

    std::string str;
    while (1)
    {
        pRawReader->ReadChannelRaw(raw);

        for (const auto& data : raw.vecRaw)
        {
            str += std::format("{:02d}  ", data);
        }

        std::cout << str << std::endl;
        str.clear();
    }

    delete pRawReader;
    
    return 0;
}

