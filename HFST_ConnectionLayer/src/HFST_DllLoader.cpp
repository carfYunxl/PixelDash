#include "pch.h"
#include "HFST_DllLoader.hpp"
#include "HFST_Exception.hpp"

namespace HFST
{
    API_Loader_Base::API_Loader_Base() {
        //! TODO
    }

    void API_Loader_Base::LoadDLL(const std::string& library_path)
    {
        if (library_path.empty()) {
            throw HFST_Exception("Empty dll path");
        }

        //! 在当前路径下寻找该文件是否存在
        std::filesystem::path path(library_path);
        if (!std::filesystem::exists(path)) {
            std::cout << library_path << std::endl;
            throw HFST_Exception("Not Exist dll path");
        }

        if (m_hInstance) {
            FreeLibrary(m_hInstance);
            m_hInstance = nullptr;
        }

        m_hInstance = LoadLibraryA(path.string().c_str());
        DWORD dwError = GetLastError();

        if (!m_hInstance) {
            std::cout << "error code " << dwError << std::endl;
            throw HFST_Exception(" Load dll File failed !");
        }

        if (!Load_Function())
        {
            throw HFST_Exception(" Load dll functions failed !");
        };
    }
}
