#ifndef __HFST_BRIDGE_HPP__
#define __HFST_BRIDGE_HPP__

namespace HFST
{
    enum class CommunicationMode
    {
        BULK = 0,
        ADB,
        HID_OVER_I2C
    };

    class Bridge
    {
    public:
        explicit Bridge(CommunicationMode mode);
        virtual ~Bridge() {}
    private:

    };
}

#endif //__HFST_BRIDGE_HPP__