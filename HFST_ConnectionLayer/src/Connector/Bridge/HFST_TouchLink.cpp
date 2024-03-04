#include "pch.h"
#include "HFST_TouchLink.hpp"

namespace HFST
{
    TouchLink::TouchLink(const USB_Manager& manager)
    : m_UsbManager(manager){
    }

    bool TouchLink::GetInfomation()
    {
        HFST_API* pApi = HFST_API::GetAPI();

        if (!pApi)
            return false;

        unsigned char buffer[64]{ 0 };
        int ret = pApi->TTK.GetI2CStatus_Bulk(buffer);
        if (ret <= 0)
            return false;

        m_TLInfo.nHwVer = (buffer[2] >> 5);
        m_TLInfo.nFwVerH = (buffer[2] & 0x1F);
        m_TLInfo.nFwVerL = buffer[3];
        return true;
    }
    bool TouchLink::SetVoltage(double vdd, double iovdd)
    {
        HFST_API* pApi = HFST_API::GetAPI();

        if (!pApi)
            return false;

        int ret{ -1 };
        if (m_TLInfo.nHwVer > 5)
        {
            ret = pApi->TTK.SetTouchLinkVoltage((unsigned short)(vdd * 1000), (unsigned short)(iovdd * 1000));
        }
        else
            ret = pApi->TTK.SetTouchLink3_Voltage(vdd, iovdd);

        if (ret <= 0)
        {
            return false;
        }

        m_TLInfo.dVDD = vdd;
        m_TLInfo.dIOVDD = iovdd;
        return true;
    }

    bool TouchLink::CheckInstallDriver()
    {
        HFST_API* pApi = HFST_API::GetAPI();

        if (!pApi)
            return false;

        for (int i = 1; i <= 8; ++i)
        {
            if (pApi->USB_Driver.WinUSB_Driver_CheckIsConnected(i) == 0)
            {
                // check usb status
                if ((!m_UsbManager.Check_USB_BULK_Status(i)) && (m_UsbManager.DetectUSBConnectCount() <= 0))
                {
                    std::thread thread_install_TL_Driver([&]() -> bool {

                        if (pApi->USB_Driver.WinUSB_Driver_Install(i))
                            return false;

                        // clear setup info files
                        pApi->USB_Driver.WinUSB_Driver_RemoveFiles();

                        return true;
                        });
                    thread_install_TL_Driver.join();
                    break;
                }
            }
        }

        return true;
    }
}
