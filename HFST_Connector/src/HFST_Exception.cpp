#include "pch.h"
#include "HFST_Exception.hpp"

namespace HFST
{
    HFST_Exception::HFST_Exception(const std::string& message)
        : m_Message(message)
    {

    }

    const char* HFST_Exception::what() const noexcept
    {
        return m_Message.c_str();
    }
}
