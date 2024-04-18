# CommonModeNoiseRejection 开发文档

## 内容提要:
- 简介
- 主要功能
- 架构
- 开发环境
- 接口设计
- 使用方法
- 用户手册

### 一 简介
CommonModeNoiseRejection 是一款 windows 应用程序，设计目标是能够操控线性机台，并满足用户对于不同打点测试环境下的使用需求。
### 二 主要功能
- 支持驱动线性机台：
    - 连续运动
    - 定量驱动
- 支持CCD实时图像采集与处理：
    - 支持CCD自动定位与调试
- 支持多种通信协议：
    - BULK
    - HID
    - ADB
    - WIFI / BlueTooth ...
- 支持多种打点判定规则：
    - 精准度
    - 直线度
    - ...
- 支持多种打点图形：
    - 圆形
    - 螺旋形
    - 直线
    - 1000点
    - ...
### 三 架构
- 通信库
- 功能层
- UI层
### 四 开发环境
- *Windows*
- *VS2019 IDE / MSVC compiler*
- *C++20*
### 五 UI设计
### 六 功能介绍
#### <font color=cyan face="微软雅黑">6.1 Connection Layer</font>
连接层是独立于UI层之外的一层额外的功能层，该层是为了**降低功能模块**与**UI**的耦合度而设计。大体上，它主要提供以下功能：
- 支持连接 IC，并读取 IC 信息
- 支持读取 IC 报点
- 其他 ...

其实现细节如下：

- （1）异常处理：
    - 为了既能返回IC的连接状态，又能返回一些细节，此处提供统一错误码返回方式：*std::error_code*。
    - 不需要包含额外的头文件，标准库有提供。
~~~C++
/**
*
*
*/
#ifndef __HFST_ERROR_HPP__
#define __HFST_ERROR_HPP__

namespace HFST
{
    class std::error_category;
    class HFST_ERROR;

    enum class ErrorCode
    {
        OK              = 0,
        API_INIT        = -1,
        BULK_INIT       = -2,
        BULK_IN         = -3,
        BULK_OUT        = -4,
        I2C_RW_ERROR    = -5,
        HID_INIT        = -6,
        BRIDGE_CREATE   = -7,
        SACN_I2C_ADDR   = -8,
        SET_I2C_ADDR    = -9,
        GET_CHIP_ID     = -10,
        GET_PROTOCOL    = -11,
        GET_STATUS      = -12,
        GET_IC_INFO     = -13,
        GET_RAW_INFO    = -14,
        SW_RESET        = -15
        // ...etc
    };

    class HFST_ERROR : public std::error_category
    {
    public:
        const char* name() const noexcept override;

        std::string message(int ev) const override;
    };

    static HFST_ERROR& GetError()
    {
        static HFST_ERROR hError;
        return hError;
    }

    std::error_code make_error_code(int e);
}

#endif //__HFST_ERROR_HPP__
~~~

