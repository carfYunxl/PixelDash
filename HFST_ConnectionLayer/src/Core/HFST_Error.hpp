#ifndef __HFST_ERROR_HPP__
#define __HFST_ERROR_HPP__

namespace HFST
{
    class std::error_category;
    class HFST_ERROR;

    extern HFST_ERROR g_sError;

    enum class ErrorCode
    {
        OK              = 0,
        API_INIT        = -1,
        BULK_INIT       = -2,
        BULK_IN         = -3,
        BULK_OUT        = -4,
        I2C_READ_ERROR  = -5
        // ...etc
    };

    class HFST_ERROR : public std::error_category
    {
    public:
        const char* name() const noexcept override;

        std::string message(int ev) const override;
    };

    std::error_code make_error_code(int e);

    std::error_code TryError();
}

#endif //__HFST_ERROR_HPP__
