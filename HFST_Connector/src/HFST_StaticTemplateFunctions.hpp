#ifndef __HFST_STATIC_TEMPLATE_FUNCTIONS_HPP__
#define __HFST_STATIC_TEMPLATE_FUNCTIONS_HPP__

#include "HFST_TouchDevice.hpp"

//! 构建一批模板函数，用于实现静态多态
namespace HFST
{
    template<typename T>
    concept ConvertibleToDevice = std::is_convertible_v<T, TouchDevice<0x0F>>;

    template<ConvertibleToDevice TouchDevice>
    inline bool GetCFG(TouchDevice& device, unsigned char* cfg)
    {
        return device.GetCFG(cfg);
    }

    template<typename TouchDevice>
    inline bool IsFwChecksumOK(TouchDevice& device, unsigned char* fw, unsigned int size)
    {
        return device.IsFwChecksumOK(fw, size);
    }

    template<typename TouchDevice>
    inline bool JumpToSWISP(TouchDevice& device, int protocol)
    {
        return device.JumpToSWISP(protocol);
    }
}


#endif //__HFST_STATIC_TEMPLATE_FUNCTIONS_HPP__
