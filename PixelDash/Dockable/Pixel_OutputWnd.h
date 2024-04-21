#ifndef __Pixel_OUTPUT_WND_H__
#define __Pixel_OUTPUT_WND_H__

#include "Pixel_ListBox.h"
#include "Pixel_Header.h"

class Pixel_OutputWnd : public CDockablePane
{
	// 构造
public:
	Pixel_OutputWnd() noexcept;

	void UpdateFonts();

	// 特性
protected:
	CMFCTabCtrl	m_wndTabs;

	Pixel_ListBox m_wndOutputBuild;
	//Pixel_ListBox m_wndOutputDebug;
	//Pixel_ListBox m_wndOutputFind;

	CFont	  m_Font;

protected:
	void AdjustHorzScroll(CListBox& wndListBox);

	// 实现
public:
	virtual ~Pixel_OutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()

public:
	void AddString(LogType type, const CString& str);
};

#endif //__Pixel_OUTPUT_WND_H__

