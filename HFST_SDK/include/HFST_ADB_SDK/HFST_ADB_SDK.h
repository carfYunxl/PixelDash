#ifndef __HFST_ADB_SDK_H__
#define __HFST_ADB_SDK_H__

/**
 * @brief HFST_ADB_SDK dll used to communication with linux phone by ADB(Android Debug Bridge) Tool
 * @date 2023/01/10
 */

#ifndef HANDLE
typedef void * HANDLE;
#endif

/**
 * @brief Open ADB shell to allow communication and alloc resource 
 * @param [out] hHandle         Receive device instance   
 * @param [in]  bAdbRunAsRoot   Determine whether run ADB services as root
 * @param [in]	pAdbFilePath	adb.exe file path, it can be absolute path or relative path, but can't have space
 * @param [in]  pExtraService   Extra user define service except open ADB shell
 * @note  -1. Usually this interface only open ADB shell run "adb.exe shell"
 *        -2. If bAdbRunAsRoot is true, it will run "adb.exe root" as first command
 *        -3. If pExtraService is assign, after "adb.exe shell" it will run pExtraService command like "adb.exe pExtraService"
 *            such as there is a user-defined service need run
 * @retval > 0 SUCCESS
 * @retval < 0 FAIL, refer to HFST_ADB_ERROR_XX as detailed error code
 */
int HFST_ADB_SDK_Open( HANDLE * hHandle, bool bAdbRunAsRoot, const char * pAdbFilePath, const char * pExtraService );

/**
 * @brief Close ADB shell and release resource
 * @param [in] hHandle  Device instance, return by HFST_ADB_SDK_Open
 * @retval > 0 SUCCESS
 * @retval < 0 FAIL
 */
int HFST_ADB_SDK_Close( HANDLE hHandle );

/*
 * @brief Read data from phone by ADB
 * @param [in] hHandle     Device instance, return by HFST_ADB_SDK_Open 
 * @param [in] pReadBuf    User alloc memory to receive data 
 * @param [in] nReadSize   The buffer size of pReadBuf
 * @note
 * @retval > 0 Current read data length
 * @retval < 0 FAIL
 */
int HFST_ADB_SDK_Read( HANDLE hHandle, unsigned char * pReadBuf, int nReadSize );

/**
 * @brief Write data to phone by ADB 
 * @param [in] hHandle      Device instance, return by HFST_ADB_SDK_Open
 * @param [in] pWriteBuf    The data write to ADB
 * @param [in] nWriteSize   The number of size to write in pReadBuf
 * @note
 * @retval > 0 SUCCESS
 * @retval < 0 FAIL
 */
int HFST_ADB_SDK_Write( HANDLE hHandle, const char * pWriteBuf, int nWriteSize );

/**
 * @brief Run adb command
 * @param [in] pAdbFilePath	adb.exe file path, it can be absolute path or relative path, but can't have space
 * @param [in] pAdbParam	Run adb.exe append param, each param can't have space
 * @example 
 *	if adb command like: adb.exe push d:\\test.txt /data/httu, you can use
 *	HFST_ADB_SDK_RunCmd( "adb\\adb.exe", "push d:\\test.txt /data/httu" );
 * @retval > 0 SUCCESS
 * @retval < 0 FAIL
 */
int HFST_ADB_SDK_RunCmd( const char * pAdbFilePath, const char * pAdbParam );

#endif // __HFST_ADB_SDK_H__
