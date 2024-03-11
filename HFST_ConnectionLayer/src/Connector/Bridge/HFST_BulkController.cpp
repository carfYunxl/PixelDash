#include "pch.h"
#include "HFST_BulkController.hpp"

namespace HFST
{
    BulkController::BulkController( uint32_t device_index )
        : m_nDeviceIdx( device_index ) {
    }

    //! close BULK transfer
    BulkController::~BulkController() {
        HFST_API::GetAPI()->BULK.USBComm_FinishEx();
    }

    //! Initialize the BULK
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

    //! Check BULK status
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

    //! check USB connect count by GUID guid
    int BulkController::DetectUSBConnectCount(GUID guid) const
    {
        HDEVINFO hardwareDeviceInfoSet;
        SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
        HANDLE deviceHandle = INVALID_HANDLE_VALUE;
        DWORD result;
        UCHAR deviceIndex = 0;
        hardwareDeviceInfoSet = SetupDiGetClassDevs(&guid,
            NULL,
            NULL,
            (DIGCF_PRESENT |
                DIGCF_DEVICEINTERFACE));
        do {
            deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
            result = SetupDiEnumDeviceInterfaces(hardwareDeviceInfoSet,
                NULL,
                &guid,
                deviceIndex,
                &deviceInterfaceData);
            if (result == FALSE) {
                //TRACE("NOT a target USB\n");
            }
            else {
                //TRACE("FOUND a target USB\n");
                deviceIndex++;
            }
            SetupDiDestroyDeviceInfoList(hardwareDeviceInfoSet);
        } while (result == TRUE);
        return deviceIndex;
    }
}
