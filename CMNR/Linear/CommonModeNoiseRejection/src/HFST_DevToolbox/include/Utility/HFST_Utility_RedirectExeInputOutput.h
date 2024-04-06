
#ifndef __HFST_UTILITY_REDIRECTEXEINPUTOUTPUT_H__
#define __HFST_UTILITY_REDIRECTEXEINPUTOUTPUT_H__

#include "HFST_CommDef.h"

typedef void (*pfnOutputCallback)(const char *, int, void *);

_HFST_BEGIN

/*
 * @brief Redirect .exe input output
 *
 * For example, there is an exist exe application, you need catch application 
 * output in your application code to do something, so you can use this interface
 * 
 * After you call RunExe and appoint receive exe output callback function, if there 
 * is have any exe output data, the callback function will be called with current
 * exe output data and data size, if have error, data will be null and size is 0
 * 
 */
class EXPORT_CLASS HFST_Utility_RedirectExeInputOutput
{
public:
	HFST_Utility_RedirectExeInputOutput();

	/**
	 * Run current exe application
	 * @param	[in]	exe			The exe application file path, allow run with param, eg: "adb.exe root"
	 * @param	[in]	bWaitExit	Wait execution completed, if next operation relate exe result, you will wait execution complete
	 * @param	[in]	fnCallback	Callback function that receive exe output data
	 * @param	[in]	pUserData	User data that callback will append
	 * @return	> 0 success
	 *			< 0 fail
	 */
	int		RunExe( const char * exe, bool bWaitExit = false, pfnOutputCallback fnCallback = NULL, void * pUserData = NULL );

	/**
	 * Run exe support command, call this interface must make sure call RunExe first
	 * @param	[in]	cmd			The exe support command
	 * @param	[in]	bExit		Whether exit exe
	 * @return	> 0 success
	 *			< 0 fail
	 */
	int		RunCmd( const char * cmd, bool bExit = false );

private:
	void	ThreadReadExeOutput();

private:
	HANDLE				m_hReadPipe;
	HANDLE				m_hWritePipe;

	HANDLE				m_hProcess;
	pfnOutputCallback	m_fnCallback;
	void			  * m_pUserData;
};

_HFST_END
#endif // __HFST_UTILITY_REDIRECTEXEINPUTOUTPUT_H__