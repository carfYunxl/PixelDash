#include "pch.h"
#include "HFST_Error.hpp"
#include <system_error>

namespace HFST
{
    const char* HFST_ERROR::name() const noexcept
    {
        return "HFST_ERROR";
    }

    std::string HFST_ERROR::message(int ev) const
    {
        switch (static_cast<ErrorCode>(ev))
        {
            case ErrorCode::OK:
                return "Success";
            case ErrorCode::API_INIT:
                return "��ʼ��ʧ��";
            case ErrorCode::BULK_INIT:
                return "API ��ʼ��ʧ��";
            case ErrorCode::BULK_IN:
                return "BULK ����ʧ��";
            case ErrorCode::BULK_OUT:
                return "BULK ����ʧ��";
            case ErrorCode::I2C_RW_ERROR:
                return "BULK ����ʧ��";
            case ErrorCode::HID_INIT:
                return "HID ��ʼ��ʧ��";
            case ErrorCode::BRIDGE_CREATE:
                return "����Bridgeʧ��";
            case ErrorCode::SACN_I2C_ADDR:
                return "ɨ��I2C��ַʧ��";
            case ErrorCode::SET_I2C_ADDR:
                return "����I2C��ַʧ��";
            case ErrorCode::GET_CHIP_ID:
                return "��ȡChipIDʧ��";
            case ErrorCode::GET_PROTOCOL:
                return "��ȡProtocolʧ��";
            case ErrorCode::GET_STATUS:
                return "��ȡIC Statusʧ��";
            case ErrorCode::GET_IC_INFO:
                return "��ȡIC infoʧ��";
            case ErrorCode::GET_RAW_INFO:
                return "��ȡRaw Line Infoʧ��";
            case ErrorCode::SW_RESET:
                return "SW RESET ʧ��";
        }

        return "δ֪ϵͳ����";
    }
}
