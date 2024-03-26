#include <windows.h>
#include <iostream>
#include <system_error>
#include "HFST_Error.hpp"
#include "HFST_Connector.hpp"

int main(int argc, char* argv[])
{
    HFST::Connector connector;
    std::error_code error = connector.Connect();

    std::cout << "\n" << error.message() << std::endl;
    system("pause");
    return 0;
}

