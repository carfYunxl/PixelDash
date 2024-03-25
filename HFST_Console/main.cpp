#include <windows.h>
#include <iostream>
#include <system_error>
#include "HFST_Error.hpp"

int main(int argc, char* argv[])
{
    std::error_code code = HFST::TryError();
    std::cout << code.message() << std::endl;
    std::cout << code.category().name() << std::endl;
    std::cout << code.value() << std::endl;
    return 0;
}

