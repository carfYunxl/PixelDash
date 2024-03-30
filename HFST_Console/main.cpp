#include <windows.h>
#include <iostream>
#include <system_error>
#include "HFST_Error.hpp"
#include "HFST_Connector.hpp"

int main(int argc, char* argv[])
{
    HFST::Connector connector;
    connector.SetCommunicationMode(HFST::CommunicationMode::TOUCH_LINK);

    std::error_code error = connector.Connect(3.3,3.3);

    std::cout << "\n" << error.message() << std::endl;

    std::cout << connector.GetChipID() << std::endl;
    std::cout << connector.GetRxTxCnt().first << " | " << connector.GetRxTxCnt().second << std::endl;

    HFST::IC_Info info = connector.IC_GetInfo();
    system("pause");
    return 0;
}

