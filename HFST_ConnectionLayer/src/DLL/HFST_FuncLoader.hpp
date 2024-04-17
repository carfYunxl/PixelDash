#ifndef __HFST_FUNCTION_LOADER__
#define __HFST_FUNCTION_LOADER__

#include "DLL/HFST_DllLoader.hpp"
#include "DLL/HFST_FunctionExport.hpp"

#include <string>

namespace HFST
{
    //! load TTK_Communication_API functions
    class TTK_Communication_API : private API_Loader_Base
    {
    public:
        TTK_Communication_API();
        ~TTK_Communication_API() {}

        void LoadDLL(const std::string& library_path) {
            API_Loader_Base::LoadDLL(library_path);
        }

    private:
        virtual bool Load_Function() override;
    public:
        pfn_ReadI2CReg                  ReadI2CReg;
        pfn_WriteI2CReg                 WriteI2CReg;
        pfn_ResetTP                     ResetTP;
        pfn_SetI2CAddr                  SetI2CAddr;
        pfn_WriteCmd                    WriteCmd;
        pfn_ReadCmd                     ReadCmd;
        pfn_ReadI2CRegBeginINT          ReadI2CRegBeginINT;
        pfn_ReadI2CRegDataINT           ReadI2CRegDataINT;
        pfn_ReadI2CRegEndINT            ReadI2CRegEndINT;
        pfn_GetI2CStatus_Bulk           GetI2CStatus_Bulk;
        pfn_SetTouchLinkVoltage         SetTouchLinkVoltage;
        pfn_SetTouchLink3_Voltage       SetTouchLink3_Voltage;
        pfn_HFST_CheckIsA8018ICPI2CAddr CheckIsA8018ICPI2CAddr;
        pfn_CommunicationModeSelect     CommunicationModeSelect;

        pfn_HID_Init                    HID_Init;
        pfn_HID_UnInit                  HID_UnInit;
        pfn_HID_GetLastErrorCode        HID_GetLastErrorCode;
        pfn_HID_ReadRawdata             HID_ReadRawdata;
        pfn_HID_GetLastErrorMessage     HID_GetLastErrorMessage;
        pfn_ADB_Init                    ADB_Init;
        pfn_ADB_UnInit                  ADB_UnInit;
        pfn_ADB_StartRecordRawData      ADB_StartRecordRawData;
        pfn_ADB_StopRecordRawData       ADB_StopRecordRawData;
    };

    //! load Bulk API functions
    class Bulk_API : private API_Loader_Base
    {
    public:
        Bulk_API();
        ~Bulk_API() {}

        void LoadDLL(const std::string& library_path) {
            API_Loader_Base::LoadDLL(library_path);
        }
    private:
        virtual bool Load_Function() override;
    public:
        pfn_USBComm_FinishEx        USBComm_FinishEx;
        pfn_USBComm_InitEx          USBComm_InitEx;
        pfn_USBComm_CheckBulkInEx   USBComm_CheckBulkInEx;
        pfn_USBComm_CheckBulkOutEx  USBComm_CheckBulkOutEx;
        pfn_USBComm_ReadFromBulkEx  USBComm_ReadFromBulkEx;
        pfn_USBComm_WriteToBulkEx   USBComm_WriteToBulkEx;
    };

    class WinUSB_Driver_API : private API_Loader_Base
    {
    public:
        WinUSB_Driver_API();
        ~WinUSB_Driver_API() {}

        void LoadDLL(const std::string& library_path) {
            API_Loader_Base::LoadDLL(library_path);
        }
    private:
        virtual bool Load_Function() override;
    public:
        pfn_WinUSB_Driver_CheckIsConnected  WinUSB_Driver_CheckIsConnected;
        pfn_WinUSB_Driver_Install           WinUSB_Driver_Install;
        pfn_WinUSB_Driver_RemoveFiles       WinUSB_Driver_RemoveFiles;
    };

    class USB_HID : private API_Loader_Base
    {
    public:
        USB_HID();
        ~USB_HID() {}

        void LoadDLL(const std::string& library_path) {
            API_Loader_Base::LoadDLL(library_path);
        }
    private:
        virtual bool Load_Function() override;
    public:
        pfn_OpenHID     OpenHID;
        pfn_CloseHID    CloseHID;
        pfn_WriteHID    WriteHID;
    };
};

#endif //__HFST_FUNCTION_LOADER__
