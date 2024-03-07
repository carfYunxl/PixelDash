/*****************************************************************//**
 * @file   HFST_Bridge.hpp
 * @brief  Abstract Class of multi protocol:
 *              BULK
 *              HID
 *              HID_OVER_I2C
 *              WIFI
 *              BlueTooth
 *              ADB
 * 
 * @author yun
 * @date   March 2024
 *********************************************************************/

#ifndef __HFST_BRIDGE_HPP__
#define __HFST_BRIDGE_HPP__

namespace HFST
{
    enum class CommunicationMode
    {
        BULK = 0,
        HID,
        WIFI,
        ADB
    };

    struct Impl
    {
        bool init() {}
        bool UnInit() {}
        void DoSomething() {};
    };

    struct BULK : public Impl
    {};

    struct HID : public Impl
    {};

    struct WIFI : public Impl
    {};

    struct ADB : public Impl
    {};

    class Bridge
    {
    public:
        explicit Bridge(CommunicationMode mode);
        virtual ~Bridge() {}

        void Attach();
        void Detach();

        void Switch(CommunicationMode mode);
    private:
        CommunicationMode m_CMode;
        Impl* pImpl;
    };
}

#endif //__HFST_BRIDGE_HPP__