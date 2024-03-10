#include "pch.h"
#include "HFST_BulkController.hpp"

namespace HFST
{
    BulkController::BulkController( uint32_t device_index )
        : m_nDeviceIdx( device_index ) {
    }

    // close BULK transfer
    BulkController::~BulkController() {
        HFST_API::GetAPI()->BULK.USBComm_FinishEx();
    }

    // Initialize the BULK
    bool BulkController::Initialize()
    {
        auto* api = HFST_API::GetAPI();
        if ( !api )
        {
            return false;
        }

        int ret = api->BULK.USBComm_InitEx( 1500, m_nDeviceIdx );
        if ( ret <= 0 )
            return false;

        return true;
    }

    // Check BULK status
    bool BulkController::CheckState()
    {
        auto* api = HFST_API::GetAPI();
        if (!api)
        {
            return false;
        }

        int ret = api->BULK.USBComm_CheckBulkInEx();
        if (ret <= 0)
            return false;

        ret = api->BULK.USBComm_CheckBulkOutEx();
        if (ret <= 0)
            return false;

        return true;
    }
}
