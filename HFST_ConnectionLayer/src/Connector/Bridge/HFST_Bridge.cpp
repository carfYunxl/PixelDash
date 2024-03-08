#include "pch.h"
#include "HFST_Bridge.hpp"

namespace HFST
{
    Bridge::Bridge(CommunicationMode mode) : m_CMode(mode), pImpl(nullptr){

    }

    void Bridge::Attach()
    {
        switch (m_CMode)
        {
            case CommunicationMode::BULK:
            {
                pImpl = new BULK();
                break;
            }
            case CommunicationMode::HID:
            {
                pImpl = new HID();
                break;
            }
            case CommunicationMode::WIFI:
            {
                pImpl = new WIFI();
                break;
            }
            case CommunicationMode::ADB:
            {
                pImpl = new ADB();
                break;
            }
        }

        pImpl->init();
    }

    void Bridge::Detach()
    {
        pImpl->UnInit();
    }

    void Bridge::Switch(CommunicationMode mode)
    {
        if (pImpl)
            delete pImpl;

        switch (m_CMode)
        {
            case CommunicationMode::BULK:
            {
                pImpl = new BULK();
                break;
            }
            case CommunicationMode::HID:
            {
                pImpl = new HID();
                break;
            }
            case CommunicationMode::WIFI:
            {
                pImpl = new WIFI();
                break;
            }
            case CommunicationMode::ADB:
            {
                pImpl = new ADB();
                break;
            }
        }

        pImpl->init();
    }
}
