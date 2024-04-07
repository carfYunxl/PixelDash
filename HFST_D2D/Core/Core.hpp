#ifndef __HFST_CORE_HPP__
#define __HFST_CORE_HPP__

#ifndef HFST_D2D_EXPORT

    #ifndef BUILD_LIB
        #define HFST_D2D_EXPORT __declspec(dllimport)
    #else
        #define HFST_D2D_EXPORT __declspec(dllexport)
    #endif

#endif

#include <memory>

#endif //__HFST_CORE_HPP__
