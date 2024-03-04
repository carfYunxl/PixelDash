
#include "HFST_HID_SDK.h"
#include "HFST_HID_SDK_ErrorCodeDef.h"

#include <Windows.h>
#include <tchar.h>
#include <conio.h>

extern "C" {
#include "hidsdi.h"
#include "setupapi.h"
}

typedef struct _HFST_HID_CONTEXT_ {
	HANDLE	hDevHandle;
	USHORT	nReadSize;
	USHORT	nWriteSize;
	_HFST_HID_CONTEXT_() {
		hDevHandle = INVALID_HANDLE_VALUE;
		nReadSize = nWriteSize = 0;
	}
} HFST_HID_CONTEXT;

struct ERROR_DETAIL_MESSAGE {
	int		errCode;
	char	errMessage[250];
} err_msg[] = { { HFST_HID_ERROR_OK									,	"Success"											},
				{ HFST_HID_ERROR_PARAM								,	"Invalid input param"								},
				{ HFST_HID_ERROR_ALLOC_MEMMORY						,	"Can't alloc memory"								},
				{ HFST_HID_ERROR_DEVICE_NOT_FOUND					,	"Can't find device"									},
				{ HFST_HID_ERROR_GETDEVICE_COLLECTION				,	"Can't get device information collection"			},
				{ HFST_HID_ERROR_GETDEVICE_INFORMATION				,	"Can't get device information"						},
				{ HFST_HID_ERROR_GETDEVICE_INFORMATION_DETAIL_SIZE	,	"Can't get device information detail size"			},
				{ HFST_HID_ERROR_GETDEVICE_INFORMATION_DETAIL_MSG	,	"Can't get device information detail message"		},
				{ HFST_HID_ERROR_OPEN_DEVICE_NO_ACCESS				,	"Can't open device with no access"					},
				{ HFST_HID_ERROR_OPEN_DEVICE_ACCESS					,	"Can't open device with read/write access"			},
				{ HFST_HID_ERROR_GETDEVICE_ATTRIBUTE				,	"Can't get device attribute"						},
				{ HFST_HID_ERROR_GETDEVICE_PREPASED_DATA			,	"Can't get device prepare data"						},
				{ HFST_HID_ERROR_GETDEVICE_CAPS						,	"Can't get device caps"								},
				{ HFST_HID_ERROR_GET_INPUT_REPORT					,	"Get input report data error"						},
				{ HFST_HID_ERROR_GET_INPUT_REPORT_DATA_SIZE         ,   "Get input report data size not equal report size"  },
				{ HFST_HID_ERROR_SET_OUTPUT_REPORT					,	"Set output report data error"						},
				{ HFST_HID_ERROR_SET_OUTPUT_REPORT_DATA_SIZE        ,   "Set output report data size not equal report size" },
				{ HFST_HID_ERROR_DESCRIBE_REPORT_LENGTH				,	"Find device, but invalid report in/out length"		},	
				{ HFST_HID_ERROR_DEVICE_PAGE						,	"Find device, but invalid usage page"				} };

int HFST_HID_SDK_Open( HANDLE * hHandle, int pid, int vid, int page )
{
	if ( nullptr == hHandle || pid <= 0 || vid <= 0 || page <= 0 )
		return HFST_HID_ERROR_PARAM;

	HFST_HID_CONTEXT * pHidContext = new HFST_HID_CONTEXT();
	if ( nullptr == pHidContext ) return HFST_HID_ERROR_ALLOC_MEMMORY;

	GUID HidGuid;
	HidD_GetHidGuid( &HidGuid );

	// Get device collection
	HDEVINFO hDevInfoSet = SetupDiGetClassDevs( &HidGuid,
												NULL,
												NULL,
												DIGCF_DEVICEINTERFACE | DIGCF_PRESENT );
	if ( INVALID_HANDLE_VALUE == hDevInfoSet ) 
		return HFST_HID_ERROR_GETDEVICE_COLLECTION;

	SP_DEVICE_INTERFACE_DATA devInterfaceData;
	devInterfaceData.cbSize = sizeof( devInterfaceData );

	int nErrorCode = HFST_HID_ERROR_OK;
	DWORD dwDeviceIndex = 0, dwRequireSize = 0;

	TCHAR szDevivePath[MAX_PATH] = { 0 };
	while ( 1 )
	{
		// Get device information
		if ( FALSE == SetupDiEnumDeviceInterfaces( hDevInfoSet,
												   NULL,
												   &HidGuid,
												   dwDeviceIndex++,
												   &devInterfaceData ) )
		{
			if ( HFST_HID_ERROR_OK == nErrorCode )
				nErrorCode = HFST_HID_ERROR_GETDEVICE_INFORMATION;
			break;
		}

		// Get device information detail buffer size
		if ( FALSE == SetupDiGetDeviceInterfaceDetail( hDevInfoSet,
													   &devInterfaceData,
													   NULL,
													   NULL,
													   &dwRequireSize,
													   NULL ) )
		{
			//nErrorCode = HFST_HID_ERROR_GETDEVICE_INFORMATION_DETAIL_SIZE;
			//break;
		}

		// Alloc memory according to read buffer size
		PSP_DEVICE_INTERFACE_DETAIL_DATA pDevDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc( dwRequireSize );
		if ( nullptr == pDevDetailData ) {
			nErrorCode = HFST_HID_ERROR_ALLOC_MEMMORY;
			break;
		}
		pDevDetailData->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA );

		// Get device information detail message
		if ( FALSE == SetupDiGetDeviceInterfaceDetail( hDevInfoSet,
													   &devInterfaceData,
													   pDevDetailData,
													   dwRequireSize,
													   NULL,
													   NULL ) ) 
		{
			nErrorCode = HFST_HID_ERROR_GETDEVICE_INFORMATION_DETAIL_MSG;
			free( pDevDetailData );
			break;
		}
		
		memset( szDevivePath, 0, MAX_PATH * sizeof( TCHAR ) );
		_tcscpy_s( szDevivePath, MAX_PATH, pDevDetailData->DevicePath );
		free( pDevDetailData );

		// Open device
		HANDLE hDevHandle = CreateFile( szDevivePath,
										NULL,
										FILE_SHARE_READ | FILE_SHARE_WRITE,
										NULL,
										OPEN_EXISTING,
										FILE_ATTRIBUTE_NORMAL,
										NULL );	
		if ( INVALID_HANDLE_VALUE == hDevHandle ) {
			nErrorCode = HFST_HID_ERROR_OPEN_DEVICE_NO_ACCESS;
			break;
		}
		
		// Get device attribute, don't care result
		HIDD_ATTRIBUTES devAttributes;
		devAttributes.Size = sizeof( devAttributes );
		HidD_GetAttributes( hDevHandle, &devAttributes );

		// Compare device
		if ( (vid != devAttributes.VendorID) || (pid != devAttributes.ProductID) ) {
			if ( HFST_HID_ERROR_DEVICE_PAGE != nErrorCode )
				nErrorCode = HFST_HID_ERROR_DEVICE_NOT_FOUND;
			continue;
		}

		// Get device attribute, don't care result
		PHIDP_PREPARSED_DATA pd;
		HidD_GetPreparsedData( hDevHandle, &pd );
		CloseHandle( hDevHandle );

		HIDP_CAPS caps;
		HidP_GetCaps( pd, &caps );
		HidD_FreePreparsedData( pd );
					
		// Find device, if pass page, compare whether this device is we want
		if ( page >= 0 && page != caps.UsagePage ) {
			nErrorCode = HFST_HID_ERROR_DEVICE_PAGE;
			continue;
		}

		if ( caps.InputReportByteLength <= 0 || caps.OutputReportByteLength <= 0 ) {
			nErrorCode = HFST_HID_ERROR_DESCRIBE_REPORT_LENGTH;
			break;
		}

		hDevHandle = CreateFile( szDevivePath,
								 GENERIC_READ | GENERIC_WRITE,
								 FILE_SHARE_READ | FILE_SHARE_WRITE,
								 NULL,
								 OPEN_EXISTING,
								 FILE_ATTRIBUTE_NORMAL,
								 NULL );
		if ( INVALID_HANDLE_VALUE == hDevHandle ) {
			nErrorCode = HFST_HID_ERROR_OPEN_DEVICE_ACCESS;
			break;
		}

		nErrorCode = HFST_HID_ERROR_OK;
		pHidContext->hDevHandle = hDevHandle;
		pHidContext->nReadSize = caps.FeatureReportByteLength;
		pHidContext->nWriteSize = caps.FeatureReportByteLength;

		*hHandle = pHidContext;
		break;
	}

	SetupDiDestroyDeviceInfoList( hDevInfoSet );

	if ( HFST_HID_ERROR_OK != nErrorCode )
		HFST_HID_SDK_Close( pHidContext );

	return nErrorCode;
}

int HFST_HID_SDK_Close( HANDLE hHandle )
{
	HFST_HID_CONTEXT * pHidContext = static_cast<HFST_HID_CONTEXT *>(hHandle);
	if ( !pHidContext ) return HFST_HID_ERROR_PARAM;

	if ( INVALID_HANDLE_VALUE != pHidContext->hDevHandle ) {
		CloseHandle( pHidContext->hDevHandle );
		pHidContext->hDevHandle = INVALID_HANDLE_VALUE;
	}

	delete pHidContext;
	return HFST_HID_ERROR_OK;
}

int HFST_HID_SDK_GetCommunicationMaxDataLength( HANDLE hHandle, int * nReadMaxSize, int * nWriteMaxSize )
{
	HFST_HID_CONTEXT * pHidContext = static_cast<HFST_HID_CONTEXT *>(hHandle);
	if ( !pHidContext || !nReadMaxSize || !nWriteMaxSize ) return HFST_HID_ERROR_PARAM;

	*nReadMaxSize = pHidContext->nReadSize;
	*nWriteMaxSize = pHidContext->nWriteSize;

	return HFST_HID_ERROR_OK;
}

int HFST_HID_SDK_Read( HANDLE hHandle, unsigned char * pReadBuf, int nReadSize )
{
	HFST_HID_CONTEXT * pHidContext = static_cast<HFST_HID_CONTEXT *>(hHandle);
	if ( !pHidContext || pHidContext->nReadSize <= 0 ) return HFST_HID_ERROR_PARAM;
	if ( !pReadBuf || nReadSize <= 0 ) return HFST_HID_ERROR_PARAM;

	// In order to compatible with variable length, don't check write size
	if ( HidD_GetFeature( pHidContext->hDevHandle, pReadBuf, nReadSize ) )
		return HFST_HID_ERROR_OK;

	DWORD dwError = GetLastError();
	_cprintf( "HFST_HID_SDK_Read error, readSize = %d maxReadSize = %d code = %d\r\n", nReadSize, pHidContext->nReadSize, dwError );

	if ( nReadSize != pHidContext->nReadSize )
		return ((HFST_HID_ERROR_GET_INPUT_REPORT_DATA_SIZE << 24) | (dwError & 0x00FFFFFF));
	else
		return ((HFST_HID_ERROR_GET_INPUT_REPORT << 24) | (dwError & 0x00FFFFFF));
}

int HFST_HID_SDK_Write( HANDLE hHandle, const unsigned char * pWriteBuf, int nWriteSize )
{
	HFST_HID_CONTEXT * pHidContext = static_cast<HFST_HID_CONTEXT *>(hHandle);
	if ( !pHidContext || pHidContext->nWriteSize <= 0 ) return HFST_HID_ERROR_PARAM;
	if ( !pWriteBuf || nWriteSize <= 0 ) return HFST_HID_ERROR_PARAM;

	// In order to compatible with variable length, don't check write size
	if ( HidD_SetFeature( pHidContext->hDevHandle, (PVOID)pWriteBuf, nWriteSize ) )
		return HFST_HID_ERROR_OK;

	if ( nWriteSize != pHidContext->nWriteSize )
		return ((HFST_HID_ERROR_SET_OUTPUT_REPORT_DATA_SIZE << 24) | (GetLastError() & 0x00FFFFFF));
	else
		return ((HFST_HID_ERROR_SET_OUTPUT_REPORT << 24) | (GetLastError() & 0x00FFFFFF));
}

void HFST_HID_SDK_GetErrorDetailMessage( int nErrorCode, char * pErrorBuf, int nErrorBufSize )
{
	if ( !pErrorBuf || nErrorBufSize <= 0 )
		return;

	int nSystemCode = 0;
	if ( nErrorCode > 0xFF ) {
		nSystemCode = nErrorCode & 0x00FFFFFF;
		nErrorCode = (nErrorCode >> 24);
	}
	for ( const auto & err : err_msg ) {
		if ( nErrorCode != err.errCode )
			continue;

		strcpy_s( pErrorBuf, nErrorBufSize, err.errMessage );
		if ( nSystemCode > 0 ) {
			LPSTR pSysError = nullptr;
			if ( FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
								 FORMAT_MESSAGE_FROM_SYSTEM |
								 FORMAT_MESSAGE_IGNORE_INSERTS,
								 NULL,
								 nSystemCode,
								 0,
								 (LPSTR)&pSysError,
								 0,
								 NULL ) )
			{
				strcat_s( pErrorBuf, nErrorBufSize, "\r\nSystemError:" );
				strcat_s( pErrorBuf, nErrorBufSize, pSysError );
				LocalFree( pSysError );
			}
		}
		return;
	}

	strcpy_s( pErrorBuf, nErrorBufSize, "Can't find error detail" );
}