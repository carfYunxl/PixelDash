#ifndef __HFST_TOUCHLINK_HPP__
#define __HFST_TOUCHLINK_HPP__

#include "HFST_CommonHeader.hpp"
#include "HFST_BulkController.hpp"
#include "HFST_BridgeImpl.hpp"
#include <memory>

namespace HFST
{
    class TouchLink : public BridgeImpl
    {
    public:
        TouchLink();

        virtual bool Init() override;

        bool    GetInfomation();
        bool    SetVoltage(double vdd, double iovdd);

        TL_Info GetInfo() const { return m_TLInfo; }

        BulkController& GetBulk() { return m_BulkController; }
    private:
        TL_Info            m_TLInfo;
        BulkController     m_BulkController;
    };
}

#endif //__HFST_TOUCHLINK_HPP__
