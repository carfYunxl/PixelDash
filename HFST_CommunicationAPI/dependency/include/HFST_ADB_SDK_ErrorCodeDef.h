#ifndef __HFST_ADB_SDK_ERRORCODEDEF_H__
#define __HFST_ADB_SDK_ERRORCODEDEF_H__

// error code base on -200
constexpr short HFST_ADB_ERROR_OK                   = 1;
constexpr short HFST_ADB_ERROR_START				= -200;											// ADB SDK error code begin
constexpr short HFST_ADB_ERROR_INPUT_PARAM			= ( HFST_ADB_ERROR_START			- 1 );		// input param error
constexpr short HFST_ADB_ERROR_ALLOC_MEMORY         = ( HFST_ADB_ERROR_INPUT_PARAM		- 1 );		// can't alloc memory
constexpr short HFST_ADB_ERROR_CREATE_PIPE          = ( HFST_ADB_ERROR_ALLOC_MEMORY		- 1 );		// can't create pipe
constexpr short HFST_ADB_ERROR_CREATE_PROCESS       = ( HFST_ADB_ERROR_CREATE_PIPE		- 1 );		// can't create adb process
constexpr short HFST_ADB_ERROR_READ_DATA            = ( HFST_ADB_ERROR_CREATE_PROCESS	- 1 );		// read data from adb shell error
constexpr short HFST_ADB_ERROR_WRITE_DATA           = ( HFST_ADB_ERROR_READ_DATA		- 1 );		// write data to adb shell error
constexpr short HFST_ADB_ERROR_TIMEOUT				= ( HFST_ADB_ERROR_WRITE_DATA		- 1 );		// adb timeout
constexpr short HFST_ADB_ERROR_WAIT_EXIT			= ( HFST_ADB_ERROR_TIMEOUT			- 1 );		// wait process exit error

#endif // __HFST_ADB_SDK_ERRORCODEDEF_H__
