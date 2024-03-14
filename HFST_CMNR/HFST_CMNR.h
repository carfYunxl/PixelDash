#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"

class CHFSTCMNRApp : public CWinAppEx
{
public:
	CHFSTCMNRApp() noexcept;

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CHFSTCMNRApp theApp;
