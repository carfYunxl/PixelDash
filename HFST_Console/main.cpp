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
    connector.Connect();

    HFST::IC_Info info = connector.IC_GetInfo();
    HFST::RAW::ChannelRaw<short> raw;

    HFST::RawReader* pRawReader = new HFST::RawReader_A8018_Mutual(info);
    pRawReader->ReadChannelRaw(raw);

    delete pRawReader;
    
    return 0;
}

