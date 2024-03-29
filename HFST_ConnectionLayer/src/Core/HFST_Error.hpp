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
