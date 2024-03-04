#ifndef __HFST_HID_SDK_H__
#define __HFST_HID_SDK_H__

// James 2020/10/28 HID interface

#ifndef HANDLE
typedef void * HANDLE;
#endif

#ifndef HFST_HID_ERROR_OK
#define HFST_HID_ERROR_OK 0
#endif

/**
 * HFST_HID_SDK_Open : Open HID Device with special pid, vid and usage page
 * @param	[out]	hHandle		Receive device instance
 * @param	[in]	pid			Device ProductID
 * @param	[in]	vid			Device VendorID
 * @param	[in]	page		Device Usage Page
 * return :	
 *			OK	 : return HFST_HID_ERROR_OK and set 'hHandle'
 *			Fail : Call HFST_HID_SDK_GetErrorDetailMessage to get detail error message
 */
int HFST_HID_SDK_Open( HANDLE * hHandle, int pid, int vid, int page );

/**
 * HFST_HID_SDK_Close : Close HID Device and release memory
 * @param	[in]   hHandle		Device handle, return by 'HFST_HID_SDK_Open'
 * @return :	
 *			OK	 : HFST_HID_ERROR_OK
 *			Fail : Call HFST_HID_SDK_GetErrorDetailMessage to get detail error message
 */
int HFST_HID_SDK_Close( HANDLE hHandle );

/**
 * HFST_HID_SDK_GetCommunicationMaxDataLength : Get HID communication max data length
 *		if communication data more than max data length, you should repeat call 
 *		Read/Write interface to make sure data is all be received or send
 * @param	[in]	hHandle			Device handle, return by 'HFST_HID_SDK_Open'
 * @param	[out]	nReadMaxSize	Maximum length specified in the 'InputReportLength' protocol
 * @param	[out]	nWriteMaxSize	Maximum length specified in the 'OutputReportLength' protocol
 * @return :
 *			OK	 : HFST_HID_ERROR_OK
 *			Fail : Call HFST_HID_SDK_GetErrorDetailMessage to get detail error message
 */
int HFST_HID_SDK_GetCommunicationMaxDataLength( HANDLE hHandle, int * nReadMaxSize, int * nWriteMaxSize );

/*
 * HFST_HID_SDK_Read : Read HID Device data
 * @param	[in]	hHandle		Device handle, return by 'HFST_HID_SDK_Open'
 * @param	[out]	pReadBuf	User alloc memory to receive data
 * @param	[in]	nReadSize	The buffer size of 'pReadBuf', must less nReadMaxSize, see 'remark'
 * @return :	
 *			OK	 : HFST_HID_ERROR_OK
 *			Fail : Call HFST_HID_SDK_GetErrorDetailMessage to get detail error message
 * @remark :
 *			In order to compatible with variable length, we don't check read size, so
 *			if HID not support read variable data, make sure the 'nReadSize' is equal
 *			to the size of 'HFST_HID_SDK_GetCommunicationMaxDataLength' return, if 
 *			nReadSize less nReadMaxSize, it will return error, if nReadSize large
 *			nReadMaxSize, it return success, but only nReadMaxSize data will be read
 */
int HFST_HID_SDK_Read( HANDLE hHandle, unsigned char * pReadBuf, int nReadSize );

/**
 * HFST_HID_SDK_Write : Write HID Device data
 * @param	[in]	hHandle		Device handle, return by 'HFST_HID_SDK_Open'
 * @param	[in]	pWriteBuf	The data write to hid
 * @param	[in]	nWriteSize	The number of size to write in 'pReadBuf', must less nWriteMaxSize, see 'remark'
 * @return :	
 *			OK	 : HFST_HID_ERROR_OK
 *			Fail : Call HFST_HID_SDK_GetErrorDetailMessage to get detail error message
 * @remark :
 *			In order to compatible with variable length, we don't check write size, so 
 *			if HID not support write variable data, make sure the 'nWriteSize' is equal 
 *			to the size of 'HFST_HID_SDK_GetCommunicationMaxDataLength' return, if nWriteSize
 *			less nWriteMaxSize, it will return error, if nWriteSize large nWriteMaxSize,
 *			it return success, but only nWriteMaxSize data will be write 
 */
int HFST_HID_SDK_Write( HANDLE hHandle, const unsigned char * pWriteBuf, int nWriteSize );

/**
 * HFST_HID_SDK_GetErrorDetailMessage : Get detail error message by error code return 'HFST_HID_SDK_xxx'
 * @param	[in]	nErrorCode		The error code return 'HFST_HID_SDK_xxx' interface
 * @param	[out]	pErrorBuf		User alloc memory to receive error message
 * @param	[in]	nErrorBufSize	The buffer size of 'pErrorBuf'
 */
void HFST_HID_SDK_GetErrorDetailMessage( int nErrorCode, char * pErrorBuf, int nErrorBufSize );

#endif // __HFST_HID_SDK_H__
