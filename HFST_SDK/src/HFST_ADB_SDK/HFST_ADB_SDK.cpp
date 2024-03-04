
#include "HFST_ADB_SDK.h"
#include "HFST_ADB_SDK_ErrorCodeDef.h"

#include <Windows.h>
#include <string>

#define SafeRelease_Ptr( p ) { if ( p ) { delete p; p = nullptr; } }
#define SafeCloseHandle( h ) { if ( h ) { CloseHandle( h ); h = NULL; } }

typedef struct _HFST_ADB_CONTEXT_ {
    HANDLE hWritePipe = INVALID_HANDLE_VALUE;
    HANDLE hReadPipe = INVALID_HANDLE_VALUE;
    HANDLE hProcess = INVALID_HANDLE_VALUE;
} HFST_ADB_CONTEXT;

int HFST_ADB_SDK_RunADB_Cmd( HFST_ADB_CONTEXT * pAdbCtx, const char * cmd, bool bWaitExit )
{
    SECURITY_ATTRIBUTES sat;
    sat.nLength = sizeof( SECURITY_ATTRIBUTES );
    sat.bInheritHandle = TRUE;
    sat.lpSecurityDescriptor = NULL;

    HANDLE tmpExeReadPipe = INVALID_HANDLE_VALUE, tmpExeWritePipe = INVALID_HANDLE_VALUE;
    HANDLE hWritePipe = INVALID_HANDLE_VALUE, hReadPipe = INVALID_HANDLE_VALUE;
    if ( !CreatePipe( &tmpExeReadPipe, &hWritePipe, &sat, NULL ) ) 
        return HFST_ADB_ERROR_CREATE_PIPE;

    if ( !CreatePipe( &hReadPipe, &tmpExeWritePipe, &sat, NULL ) ) {
        SafeCloseHandle( hWritePipe );
        return HFST_ADB_ERROR_CREATE_PIPE;
    }

    STARTUPINFOA startupinfo;
    startupinfo.cb = sizeof( STARTUPINFOA );
    GetStartupInfoA( &startupinfo );
    startupinfo.hStdError = tmpExeWritePipe;
    startupinfo.hStdOutput = tmpExeWritePipe;
    startupinfo.hStdInput = tmpExeReadPipe;
    startupinfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    startupinfo.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pinfo;
    if ( !CreateProcessA( NULL, const_cast<char *>(cmd), NULL, NULL, TRUE, 0, NULL, NULL, &startupinfo, &pinfo ) ) {
        SafeCloseHandle( hReadPipe );
        SafeCloseHandle( hWritePipe );
        return HFST_ADB_ERROR_CREATE_PROCESS;
    }

    SafeCloseHandle( tmpExeReadPipe );
    SafeCloseHandle( tmpExeWritePipe );
    SafeCloseHandle( pinfo.hThread );

    if ( bWaitExit ) {
        WaitForSingleObject( pinfo.hProcess, INFINITE );
        SafeCloseHandle( hReadPipe );
        SafeCloseHandle( hWritePipe );
    }

    if ( pAdbCtx ) {
        pAdbCtx->hReadPipe = hReadPipe;
        pAdbCtx->hWritePipe = hWritePipe;
        pAdbCtx->hProcess = pinfo.hProcess;
    }

    return HFST_ADB_ERROR_OK;
}

int HFST_ADB_SDK_Open( HANDLE * hHandle, bool bAdbRunAsRoot, const char * pAdbFilePath, const char * pExtraService )
{
    if ( !hHandle || !pAdbFilePath )
        return HFST_ADB_ERROR_INPUT_PARAM;

    HFST_ADB_CONTEXT * pAdbContext = new HFST_ADB_CONTEXT();
    if ( !pAdbContext ) return HFST_ADB_ERROR_ALLOC_MEMORY;

    int res = HFST_ADB_ERROR_OK;
    std::string strAdbFilePath( pAdbFilePath );

    // first start adb as root if some cmd need root authority
    if ( bAdbRunAsRoot ) {
        // ./platform-tools/adb.exe root
        res = HFST_ADB_SDK_RunADB_Cmd( nullptr, (strAdbFilePath + " root").c_str(), true );
        if ( HFST_ADB_ERROR_OK != res ) goto __exit;
    }
    
    // start adb shell
    res = HFST_ADB_SDK_RunADB_Cmd( pAdbContext, (strAdbFilePath + " shell").c_str(), false);
    if ( HFST_ADB_ERROR_OK != res ) goto __exit;

    // start user-defined service
    if ( pExtraService ) {
        res = HFST_ADB_SDK_Write( pAdbContext, pExtraService, strlen( pExtraService ) );
        if ( HFST_ADB_ERROR_OK != res ) goto __exit;
    }

    *hHandle = pAdbContext;
    return HFST_ADB_ERROR_OK;

__exit:
    SafeRelease_Ptr( pAdbContext );
    return res;
}

int HFST_ADB_SDK_Close( HANDLE hHandle )
{
    HFST_ADB_CONTEXT * pAdbContext = static_cast<HFST_ADB_CONTEXT *>(hHandle);
    if ( !pAdbContext ) return HFST_ADB_ERROR_INPUT_PARAM;

    HFST_ADB_SDK_Write( pAdbContext, "exit", 4 );

    SafeCloseHandle( pAdbContext->hReadPipe );
    SafeCloseHandle( pAdbContext->hWritePipe );

    // Force terminate adb process
    TerminateProcess( pAdbContext->hProcess, 0 );
    SafeCloseHandle( pAdbContext->hProcess );

    SafeRelease_Ptr( pAdbContext );
    return HFST_ADB_ERROR_OK;
}

int HFST_ADB_SDK_Read( HANDLE hHandle, unsigned char * pReadBuf, int nReadSize )
{
	HFST_ADB_CONTEXT * pAdbContext = static_cast<HFST_ADB_CONTEXT *>(hHandle);
	if ( !pAdbContext || !pReadBuf || nReadSize <= 0 ) return HFST_ADB_ERROR_INPUT_PARAM;

    DWORD dwCurrentReadSize = 0;
	if ( !ReadFile( pAdbContext->hReadPipe, pReadBuf, nReadSize - 1, &dwCurrentReadSize, NULL ) )
        return HFST_ADB_ERROR_READ_DATA;
	 
    pReadBuf[dwCurrentReadSize] = '\0';
    return dwCurrentReadSize;
}

int HFST_ADB_SDK_Write( HANDLE hHandle, const char * pWriteBuf, int nWriteSize )
{
    HFST_ADB_CONTEXT * pAdbContext = static_cast<HFST_ADB_CONTEXT *>(hHandle);
    if ( !pAdbContext || !pWriteBuf || nWriteSize <= 0 ) return HFST_ADB_ERROR_INPUT_PARAM;

    // Write data must have CRLF that can trigger shell run this command
    std::string tmp_cmd( (const char *)pWriteBuf, nWriteSize );
    if ( '\n' != tmp_cmd.back() ) tmp_cmd += "\r\n";

    DWORD dwWriteSize = 0;
    BOOL res = WriteFile( pAdbContext->hWritePipe, tmp_cmd.c_str(), tmp_cmd.length(), &dwWriteSize, NULL );

    if ( !res || dwWriteSize != tmp_cmd.length() )
        return HFST_ADB_ERROR_WRITE_DATA;

    return HFST_ADB_ERROR_OK;
}

int HFST_ADB_SDK_RunCmd( const char * pAdbFilePath, const char * pAdbParam )
{
    if ( !pAdbFilePath || !pAdbParam )
        return HFST_ADB_ERROR_INPUT_PARAM;

	SHELLEXECUTEINFOA  ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof( SHELLEXECUTEINFO );
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = pAdbFilePath;
	ShExecInfo.lpParameters = pAdbParam;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	BOOL res = ShellExecuteExA( &ShExecInfo );

	DWORD dwRet = WaitForSingleObject( ShExecInfo.hProcess, INFINITE );
    if ( WAIT_TIMEOUT == dwRet ) return HFST_ADB_ERROR_TIMEOUT;
    if ( WAIT_OBJECT_0 != dwRet ) return HFST_ADB_ERROR_WAIT_EXIT;
    
    return HFST_ADB_ERROR_OK;
}
