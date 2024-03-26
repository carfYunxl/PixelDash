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
                return "初始化失败";
            case ErrorCode::BULK_INIT:
                return "API 初始化失败";
            case ErrorCode::BULK_IN:
                return "BULK 连接失败";
            case ErrorCode::BULK_OUT:
                return "BULK 连接失败";
            case ErrorCode::I2C_RW_ERROR:
                return "BULK 连接失败";
            case ErrorCode::HID_INIT:
                return "HID 初始化失败";
            case ErrorCode::BRIDGE_CREATE:
                return "创建Bridge失败";
            case ErrorCode::SACN_I2C_ADDR:
                return "扫描I2C地址失败";
            case ErrorCode::SET_I2C_ADDR:
                return "设置I2C地址失败";
            case ErrorCode::GET_CHIP_ID:
                return "获取ChipID失败";
            case ErrorCode::GET_PROTOCOL:
                return "获取Protocol失败";
            case ErrorCode::GET_STATUS:
                return "获取IC Status失败";
            case ErrorCode::GET_IC_INFO:
                return "获取IC info失败";
            case ErrorCode::GET_RAW_INFO:
                return "获取Raw Line Info失败";
            case ErrorCode::SW_RESET:
                return "SW RESET 失败";
        }

        return "未知系统错误";
    }
}
