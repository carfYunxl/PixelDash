#include <windows.h>
#include <ConsoleApi.h>
#include "HFST_Connector.hpp"
#include <iostream>
#include <thread>
#include <Dbt.h>
#include <string>
#include <atlstr.h>
#include "HFST_DeviceManager.hpp"
#include "HFST_DummyWindow.hpp"
#include <format>

int main(int argc, char* argv[])
{
    HFST::Connector connector;

    //HFST::DummyWindow window;
    //window.SetGUID(connector.GetGUID());
    //window.Run();
    //if (!connector.RegisterDevice(window.GetWnd()))
    //{
    //    std::cout << "Register Device Failed!\n";
    //}
    //std::cout << "Register USB/HID/ABT Device Success!\n";

    if (!connector.Connect())
    {
        std::cout << "Connected Failed!\n";
    }

    HFST::IC_Info info = connector.IC_GetInfo();
    HFST::RawCollector collector(info);
    HFST::RAW::ChannelRaw<short> raw;

    std::string strText;
    while (1)
    {
        strText.clear();
        raw.vecRaw.clear();
        int ret = collector.ReadChannelRaw(raw);
        if (ret == 1)
        {
            for (size_t i = 0; i < raw.vecRaw.size(); ++i)
            {
                if (raw.nDataType == 0x20)
                    strText.append(std::format("{:02X} ", raw.vecRaw.at(i)));
                else
                    strText.append(std::format("{} ", raw.vecRaw.at(i)));
            }

            std::cout << strText << std::endl;
        }
    }
    return 0;
}

