/*****************************************************************//**
 * \file   HFST_BridgeImpl.hpp
 * \brief  a virtual base class for bridge interface.
 * 
 * \author yun
 * \date   March 2024
 *********************************************************************/

#ifndef __HFST_BRIDGE_IMPL_HPP__
#define __HFST_BRIDGE_IMPL_HPP__

#include "HFST_CommonHeader.hpp"
#include "HFST_BulkController.hpp"

namespace HFST
{
    // virtual base class
    class Bridge
    {
    public:
        virtual bool Attach() = 0;
        virtual bool Detach() = 0;
    };

    // Base CRTP class
    template<typename Derived>
    class BridgeBase : public Bridge
    {
    public:
        virtual bool Attach() override
        {
            Derived* pDerived = static_cast<Derived*>(this);
            bool success = pDerived->Init();
            success &= pDerived->GetInfo();
            success &= pDerived->SetVoltage();

            return success;
        }

        virtual bool Detach() override
        {
            return static_cast<Derived*>(this)->UnInit();
        }
    };

    //****** TouchLink Implementation ******//
    class TouchLink : public BridgeBase<TouchLink>
    {
    public:
        bool    Init() {
            HFST_API* pApi = HFST_API::GetAPI();
            if (!pApi)
                return false;

            for (int i = 1; i <= 8; ++i)
            {
                if (pApi->USB_Driver.WinUSB_Driver_CheckIsConnected(i) == 0)
                {
                    m_BulkController.ChangeDeviceIndex(i);
                    m_BulkController.Initialize();
                    bool state = m_BulkController.CheckState();
                    int cnt = m_BulkController.DetectUSBConnectCount(USB_Manager::GetGUID());
                    // check usb status
                    if (!state && cnt <= 0)
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
        bool    UnInit() { return true; }
        bool    GetInfo()   {
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
        bool    SetVoltage() {
            HFST_API* pApi = HFST_API::GetAPI();
            if (!pApi)
                return false;

            double vdd = 2.8;
            double iovdd = 1.8;

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

        TL_Info         GetTouchLinkInfo() const { return m_TLInfo; }
        BulkController& GetBulk() { return m_BulkController; }
    private:
        TL_Info            m_TLInfo;
        BulkController     m_BulkController;
    };

    //****** TouchPad Implementation ******//
    class TouchPad : public BridgeBase<TouchPad>
    {
    public:
        bool Init() {

        }
        bool UnInit() { return true; }
        bool GetInfo() { return true; }
        bool SetVoltage() { return true; }
    };

    class TL_HID : public BridgeBase<TL_HID>
    {
    public:
        bool Init() { return true; }
        bool UnInit() { return true; }
        bool GetInfo() { return true; }
        bool SetVoltage() { return true; }
    };

    class WIFI : public BridgeBase<WIFI>
    {
    public:
        bool Init() { return true; }
        bool UnInit() { return true; }
        bool GetInfo() { return true; }
        bool SetVoltage() { return true; }
    };

    class ADB : public BridgeBase<ADB>
    {
    public:
        bool Init() { return true; }
        bool UnInit() { return true; }
        bool GetInfo() { return true; }
        bool SetVoltage() { return true; }
    };
}

#endif //__HFST_BRIDGE_IMPL_HPP__
