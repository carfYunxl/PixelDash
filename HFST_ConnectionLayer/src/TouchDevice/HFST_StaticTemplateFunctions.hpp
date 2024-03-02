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

    unsigned short CalculateChecksum( unsigned char* pInData, unsigned long Len )
    {
        unsigned char LowByteChecksum;
        unsigned short nCheckSum = 0;
        for (unsigned long i = 0; i < Len; i++)
        {
            nCheckSum += (unsigned short)pInData[i];
            LowByteChecksum = (unsigned char)(nCheckSum & 0xFF);
            LowByteChecksum = (LowByteChecksum) >> 7 | (LowByteChecksum) << 1;
            nCheckSum = (nCheckSum & 0xFF00) | LowByteChecksum;
        }

        return nCheckSum;
    }
}


#endif //__HFST_STATIC_TEMPLATE_FUNCTIONS_HPP__
