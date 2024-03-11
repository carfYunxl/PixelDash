/*****************************************************************//**
 * \file   HFST_BridgeImpl.hpp
 * \brief  a virtual base class for bridge interface.
 * 
 * \author yun
 * \date   March 2024
 *********************************************************************/

#ifndef __HFST_BRIDGE_IMPL_HPP__
#define __HFST_BRIDGE_IMPL_HPP__

namespace HFST
{
    class BridgeImpl
    {
    public:
        virtual bool Init() = 0;
    };
}

#endif //__HFST_BRIDGE_IMPL_HPP__
