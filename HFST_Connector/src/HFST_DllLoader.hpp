#ifndef __HFST_DLL_LOADER_HPP__
#define __HFST_DLL_LOADER_HPP__

#include <windows.h>
#include <string>

namespace HFST
{
    //! base class for load dll functions
    class API_Loader_Base
    {
    public:
        API_Loader_Base();
        virtual ~API_Loader_Base() {
            if (m_hInstance)
            {
                FreeLibrary(m_hInstance);
            }
        }
        void LoadDLL(const std::string& library_path);
    private:
        virtual bool Load_Function() = 0;

    protected:
        HINSTANCE m_hInstance{ nullptr };
    };
}


#endif //__HFST_DLL_LOADER_HPP__
