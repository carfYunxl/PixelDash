/*****************************************************************//**
 * \file   HFST_BulkController.hpp
 * \brief  Implementation of BULK initialize and BULK status check in Windows.
 *          
 *          For BULK initialize, call Initialize().
 *          For BULK status check, just call CheckState().
 *          Normally, you can first call Initialize(), then call CheckState() to make sure BULK now is avaliable.
 * 
 * \author River_Yun
 * \date   March 2024
 * \sa     ONLY SUPPORT WINDOWS PLATFORM!!!
 *********************************************************************/
#ifndef __HFST_BULK_CONTROLLER_HPP__
#define __HFST_BULK_CONTROLLER_HPP__

namespace HFST
{
    class BulkController
    {
    public:
        BulkController() = default;
        BulkController(uint32_t device_index);
        ~BulkController();

        // Initialize the BULK
        bool Initialize();

        // Check BULK status
        bool CheckState();
    private:
        uint32_t m_nDeviceIdx{ 1 };
    };
}

#endif //__HFST_BULK_CONTROLLER_HPP__
