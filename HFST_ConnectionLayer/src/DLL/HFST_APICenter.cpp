#include "pch.h"
#include "HFST_APICenter.hpp"
#include "HFST_Exception.hpp"

#include <filesystem>
#include <iostream>

namespace HFST
{
    bool HFST_API::m_bInit = false;

    bool HFST_API::Init()
    {
        auto dll_path = std::filesystem::current_path() / "dll";
        SetDllDirectoryA(dll_path.string().c_str());

        try {

            auto dll_path_bulk = dll_path / "BulkDll.dll";
            BULK.LoadDLL(dll_path_bulk.string());

            auto dll_path_ttk = dll_path / "TTK_Communication_API_A8018.dll";
            TTK.LoadDLL(dll_path_ttk.string());

            auto dll_path_USB = dll_path / "HFST_WinUSB_Driver.dll";
            USB_Driver.LoadDLL(dll_path_USB.string());

            auto dll_path_usbhid = dll_path / "USB_HID.dll";
            USB_HID.LoadDLL(dll_path_usbhid.string());
        }
        catch (const HFST::HFST_Exception& exception)
        {
            std::cout << exception.what() << std::endl;
            return false;
        }

        return true;
    }
}
