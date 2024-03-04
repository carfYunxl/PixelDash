#include "pch.h"
#include "HFST_FuncLoader.hpp"

namespace HFST
{
    TTK_Communication_API::TTK_Communication_API()
        : ReadI2CReg{nullptr}
        , WriteI2CReg{ nullptr }
        , ResetTP{ nullptr }
        , SetI2CAddr{ nullptr }
        , WriteCmd{ nullptr }
        , ReadCmd{ nullptr }
        , ReadI2CRegBeginINT{ nullptr }
        , ReadI2CRegDataINT{ nullptr }
        , ReadI2CRegEndINT{ nullptr }
        , GetI2CStatus_Bulk{ nullptr }
        , SetTouchLinkVoltage{ nullptr }
        , SetTouchLink3_Voltage{ nullptr }
        , CheckIsA8018ICPI2CAddr{ nullptr }
        , CommunicationModeSelect{ nullptr }
    {
    }

    bool TTK_Communication_API::Load_Function()
    {
        (FARPROC&)ReadI2CReg = GetProcAddress(m_hInstance, "ReadI2CReg");
        if (!ReadI2CReg) {
            return false;
        }

        (FARPROC&)WriteI2CReg = GetProcAddress(m_hInstance, "WriteI2CReg");
        if (!WriteI2CReg) {
            return false;
        }

        (FARPROC&)ResetTP = GetProcAddress(m_hInstance, "ResetTP");
        if (!ResetTP) {
            return false;
        }

        (FARPROC&)SetI2CAddr = GetProcAddress(m_hInstance, "SetI2CAddr");
        if (!SetI2CAddr) {
            return false;
        }

        (FARPROC&)WriteCmd = GetProcAddress(m_hInstance, "WriteCmd");
        if (!WriteCmd) {
            return false;
        }

        (FARPROC&)ReadCmd = GetProcAddress(m_hInstance, "ReadCmd");
        if (!ReadCmd) {
            return false;
        }

        (FARPROC&)ReadI2CRegBeginINT = GetProcAddress(m_hInstance, "ReadI2CRegBeginINT");
        if (!ReadI2CRegBeginINT) {
            return false;
        }

        (FARPROC&)ReadI2CRegDataINT = GetProcAddress(m_hInstance, "ReadI2CRegDataINT");
        if (!ReadI2CRegDataINT) {
            return false;
        }

        (FARPROC&)ReadI2CRegEndINT = GetProcAddress(m_hInstance, "ReadI2CRegEndINT");
        if (!ReadI2CRegEndINT) {
            return false;
        }

        (FARPROC&)GetI2CStatus_Bulk = GetProcAddress(m_hInstance, "GetI2CStatus_Bulk");
        if (!GetI2CStatus_Bulk) {
            return false;
        }

        (FARPROC&)SetTouchLinkVoltage = GetProcAddress(m_hInstance, "SetTouchLinkVoltage");
        if (!SetTouchLinkVoltage) {
            return false;
        }

        (FARPROC&)SetTouchLink3_Voltage = GetProcAddress(m_hInstance, "SetTouchLink3_Voltage");
        if (!SetTouchLink3_Voltage) {
            return false;
        }

        (FARPROC&)CheckIsA8018ICPI2CAddr = GetProcAddress(m_hInstance, "HFST_CheckIsA8018ICPI2CAddr");
        if (!CheckIsA8018ICPI2CAddr) {
            return false;
        }

        (FARPROC&)CommunicationModeSelect = GetProcAddress(m_hInstance, "CommunicationModeSelect");
        if (!CommunicationModeSelect) {
            return false;
        }

        return true;
    }

    Bulk_API::Bulk_API()
        : USBComm_FinishEx{ nullptr }
        , USBComm_InitEx{ nullptr }
        , USBComm_CheckBulkInEx{ nullptr }
        , USBComm_CheckBulkOutEx{ nullptr }
        , USBComm_ReadFromBulkEx{ nullptr }
        , USBComm_WriteToBulkEx{ nullptr }
    {
        
    }

    bool Bulk_API::Load_Function()
    {
        (FARPROC&)USBComm_FinishEx = GetProcAddress(m_hInstance, "USBComm_FinishEx");
        if (!USBComm_FinishEx) {
            return false;
        }

        (FARPROC&)USBComm_InitEx = GetProcAddress(m_hInstance, "USBComm_InitEx");
        if (!USBComm_InitEx) {
            return false;
        }

        (FARPROC&)USBComm_CheckBulkInEx = GetProcAddress(m_hInstance, "USBComm_CheckBulkInEx");
        if (!USBComm_CheckBulkInEx) {
            return false;
        }

        (FARPROC&)USBComm_CheckBulkOutEx = GetProcAddress(m_hInstance, "USBComm_CheckBulkOutEx");
        if (!USBComm_CheckBulkOutEx) {
            return false;
        }

        (FARPROC&)USBComm_ReadFromBulkEx = GetProcAddress(m_hInstance, "USBComm_ReadFromBulkEx");
        if (!USBComm_ReadFromBulkEx) {
            return false;
        }

        (FARPROC&)USBComm_WriteToBulkEx = GetProcAddress(m_hInstance, "USBComm_WriteToBulkEx");
        if (!USBComm_WriteToBulkEx) {
            return false;
        }
        return true;
    }


    WinUSB_Driver_API::WinUSB_Driver_API()
        : WinUSB_Driver_CheckIsConnected{ nullptr }
        , WinUSB_Driver_Install{ nullptr }
        , WinUSB_Driver_RemoveFiles{ nullptr }
    {

    }

    bool WinUSB_Driver_API::Load_Function()
    {
        (FARPROC&)WinUSB_Driver_CheckIsConnected = GetProcAddress(m_hInstance, "HFST_WinUSB_Driver_CheckIsConnected");
        if (!WinUSB_Driver_CheckIsConnected) {
            return false;
        }

        (FARPROC&)WinUSB_Driver_Install = GetProcAddress(m_hInstance, "HFST_WinUSB_Driver_Install");
        if (!WinUSB_Driver_Install) {
            return false;
        }

        (FARPROC&)WinUSB_Driver_RemoveFiles = GetProcAddress(m_hInstance, "HFST_WinUSB_Driver_RemoveFiles");
        if (!WinUSB_Driver_RemoveFiles) {
            return false;
        }
        return true;
    }

    USB_HID::USB_HID()
        : OpenHID{nullptr}
        , CloseHID{nullptr}
        , WriteHID{nullptr}
    {

    }

    bool USB_HID::Load_Function()
    {
        (FARPROC&)OpenHID = GetProcAddress(m_hInstance, "OpenHID");
        if (!OpenHID) {
            return false;
        }

        (FARPROC&)CloseHID = GetProcAddress(m_hInstance, "CloseHID");
        if (!CloseHID) {
            return false;
        }

        (FARPROC&)WriteHID = GetProcAddress(m_hInstance, "WriteHID");
        if (!WriteHID) {
            return false;
        }
        return true;
    }
}
