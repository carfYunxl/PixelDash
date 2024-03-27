#pragma once
#include "XListBox.h"
/////////////////////////////////////////////////////////////////////////////
// COutputList 窗口

//class COutputList : public CXListBox
//{
//// 构造
//public:
//	COutputList() noexcept;
//
//// 实现
//public:
//	virtual ~COutputList();
//
//protected:
//	afx_msg void OnEditSelectAll();
//	afx_msg void OnEditCopy();
//	afx_msg void OnEditClear();
//	afx_msg void OnViewOutput();
//
//	DECLARE_MESSAGE_MAP()
//};

class COutputWnd : public CDockablePane
{
// 构造
public:
	COutputWnd() noexcept;

	void UpdateFonts();

// 特性
protected:
	CMFCTabCtrl	m_wndTabs;

	CXListBox m_wndOutputBuild;
	CXListBox m_wndOutputDebug;
	CXListBox m_wndOutputFind;

	CFont	  m_Font;

protected:
	void FillBuildWindow();
	void FillDebugWindow();
	void FillFindWindow();

	void AdjustHorzScroll(CListBox& wndListBox);

// 实现
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()

public:
	void AddOutputBuildString(const CString& str) {
		m_wndOutputBuild.AddString(str);
	}
};

