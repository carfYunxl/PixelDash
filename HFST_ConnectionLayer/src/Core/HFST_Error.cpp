#include "pch.h"
#include "HFST_Error.hpp"
#include <system_error>

namespace HFST
{
    static HFST_ERROR g_sError;
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
        case ErrorCode::I2C_READ_ERROR:
            return "BULK 连接失败";
        }

        return "未知系统错误";
    }

    std::error_code TryError()
    {
        return make_error_code(-5);
    }

    std::error_code make_error_code(int e) {
        return { e, g_sError };
    }
}
