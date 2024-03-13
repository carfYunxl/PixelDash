/*****************************************************************//**
 * @file   HFST_BulkController.hpp
 * @brief  Implementation of BULK initialize and BULK status check in Windows.
 *          
 *              For BULK initialize, call Initialize().
 *              For BULK status check, just call CheckState().
 *              Normally, you can first call Initialize(), then call CheckState() to make sure BULK now is avaliable.
 * 
 * @author River_Yun
 * @date   March 2024
 * @note   ONLY SUPPORT WINDOWS PLATFORM!!!
 * 
 * @log
 *          2024-03-11   first commits
 *********************************************************************/
#ifndef __HFST_BULK_CONTROLLER_HPP__
#define __HFST_BULK_CONTROLLER_HPP__

namespace HFST
{
    constexpr uint32_t PID_MIN = 1;
    constexpr uint32_t PID_MAX = 8;

    class BulkController
    {
    public:
        BulkController() = default;
        BulkController(uint32_t device_index);
        ~BulkController();

        //! Initialize the BULK
        bool Initialize();

        //! Check BULK status
        bool CheckState();

        //! Change device index
        void ChangeDeviceIndex(uint32_t device_index) { m_nDeviceIdx = device_index; }

        //! check USB connect count by GUID guid
        int  DetectUSBConnectCount(GUID guid) const;
    private:
        // pid index, default = 1
        // for TouchLink, it's ranges in 1 - 8
        uint32_t m_nDeviceIdx{ 1 };
    };
}

#endif //__HFST_BULK_CONTROLLER_HPP__
