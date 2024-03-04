#ifndef __HFST_TOUCHLINK_HPP__
#define __HFST_TOUCHLINK_HPP__

#include "HFST_CommonHeader.hpp"
#include "HFST_DeviceManager.hpp"
#include <memory>

namespace HFST
{
    class TouchLink
    {
    public:
        explicit TouchLink(const USB_Manager& manager);

        bool    CheckInstallDriver();
        bool    GetInfomation();
        bool    SetVoltage(double vdd, double iovdd);

        TL_Info GetInfo() const { return m_TLInfo; }
    private:
        TL_Info            m_TLInfo;
        const USB_Manager& m_UsbManager;
    };
}

#endif //__HFST_TOUCHLINK_HPP__
