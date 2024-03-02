#ifndef __HFST_CORE_HPP__
#define __HFST_CORE_HPP__

#include <stdexcept>
#include <exception>
#include <string>

namespace HFST
{
    class HFST_Exception : public std::exception
    {
    public:
        explicit HFST_Exception(const std::string& message);

        const char* what() const noexcept override;
    private:
        std::string m_Message;
    };
}

#endif //__HFST_CORE_HPP__
