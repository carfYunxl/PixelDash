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
#include <thread>
#include "HFST_DeviceManager.hpp"
#include <memory>

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
        explicit BridgeBase(CommunicationMode mode) : m_CMode(mode) {}
        ~BridgeBase() {}
        virtual bool Attach() override
        {
            HFST_API* pApi = HFST_API::GetAPI();
            if (!pApi)
                return false;

            pApi->TTK.CommunicationModeSelect(static_cast<int>(m_CMode));

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
    protected:
        CommunicationMode m_CMode;
    };

    //****** TouchLink Implementation ******//
    class TouchLink final: public BridgeBase<TouchLink>
    {
    public:
        explicit TouchLink(CommunicationMode mode) : BridgeBase<TouchLink>(mode) {}

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
    class TouchPad final : public BridgeBase<TouchPad>
    {
        inline static int USER_PAGE = 0xFF55;
    public:
        explicit TouchPad(CommunicationMode mode) : BridgeBase<TouchPad>(mode) {}

        bool Init() {
            auto api = HFST_API::GetAPI();
            if (!api)
                return false;

            if ( !api->TTK.HID_Init(m_nPid, m_nVid, m_nReportid, USER_PAGE) )
            {
                return false;
            }

            return true;
        }
        bool UnInit() {
            auto api = HFST_API::GetAPI();
            if (!api)
                return false;

            api->TTK.HID_UnInit();
            return true;
        }
        bool GetInfo() { return true; }
        bool SetVoltage() { return true; }

    private:
        int m_nPid{0x8148};
        int m_nVid{0x1200};
        int m_nReportid{0x09};
    };

    class TL_HID : public BridgeBase<TL_HID>
    {
    public:
        explicit TL_HID(CommunicationMode mode) : BridgeBase<TL_HID>(mode) {}

        bool Init() { return true; }
        bool UnInit() { return true; }
        bool GetInfo() { return true; }
        bool SetVoltage() { return true; }
    };

    class WIFI : public BridgeBase<WIFI>
    {
    public:
        explicit WIFI(CommunicationMode mode) : BridgeBase<WIFI>(mode) {}

        bool Init() { return true; }
        bool UnInit() { return true; }
        bool GetInfo() { return true; }
        bool SetVoltage() { return true; }
    };

    class ADB : public BridgeBase<ADB>
    {
    public:
        explicit ADB(CommunicationMode mode) : BridgeBase<ADB>(mode) {}

        bool Init() { return true; }
        bool UnInit() { return true; }
        bool GetInfo() { return true; }
        bool SetVoltage() { return true; }
    };

    static std::unique_ptr<Bridge> CreateBridge(CommunicationMode mode)
    {
        switch (mode)
        {
            case CommunicationMode::TOUCH_LINK:
            {
                return std::make_unique<TouchLink>(mode);
            }
            case CommunicationMode::TOUCH_PAD:
            {
                return std::make_unique<TouchPad>(mode);
            }
            case CommunicationMode::HID:
            {
                return std::make_unique<TL_HID>(mode);
            }
            case CommunicationMode::WIFI:
            {
                return std::make_unique<WIFI>(mode);
            }
            case CommunicationMode::ADB:
            {
                return std::make_unique<ADB>(mode);
            }
        }

        return nullptr;
    }
}

#endif //__HFST_BRIDGE_IMPL_HPP__
