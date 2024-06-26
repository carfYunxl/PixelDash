﻿#ifndef __Pixel_MAINFRAM_H__
#define __Pixel_MAINFRAM_H__

#include "Pixel_Header.h"
#include "Pixel_ChildView.h"
#include "Pixel_TestView.h"
#include "Pixel_OutputWnd.h"
#include "Pixel_PropertiesWnd.h"

class Pixel_MainFrame : public CFrameWndEx
{
public:
	const int  MAX_USR_TOOBARS = 10;
	const UINT USR_TOOLBAR_BG_ID = AFX_IDW_CONTROLBAR_FIRST + 40;
	const UINT USR_TOOLBAR_ED_ID = USR_TOOLBAR_BG_ID + MAX_USR_TOOBARS - 1;

	Pixel_MainFrame() noexcept;
	virtual ~Pixel_MainFrame();
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);

protected:
	DECLARE_DYNAMIC(Pixel_MainFrame)

	//CSplitterWnd m_wndSplitter;
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
	afx_msg int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnSetFocus(CWnd* pOldWnd);
	afx_msg void	OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void	OnButtonIc();
	afx_msg void	OnButtonCon();
	afx_msg void	OnButtonDiscon();
	afx_msg void	OnButtonTest1();
	afx_msg void	OnButtonTest2();
	afx_msg void	OnButtonTest3();
	afx_msg void	OnPaint();
	afx_msg void	OnViewTestItem();
	afx_msg void	OnViewProperty();
	afx_msg void	OnViewMachine();
	afx_msg void	OnViewOutput();
	afx_msg void	OnColor(UINT nID);
	afx_msg void	OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void	OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void	OnUpdateColorUI(CCmdUI* pCmdUI);
	afx_msg BOOL	OnDeviceChange(UINT nEventType, DWORD_PTR dwData);

	DECLARE_MESSAGE_MAP()
public:
	void Log(LogType type, const CString& message);

public:
	CMFCMenuBar       m_wndMenuBar;
	Pixel_ChildView	  m_wndView;
	Pixel_PropertiesWnd  m_wndProperty;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	Pixel_OutputWnd      m_wndOutput;
	Pixel_TestView		  m_wndTestView;

private:
	CMFCToolBarImages m_UserImages;
	CMFCToolBar       m_NewToolBar;
	CFont			  m_Font;

	UINT m_nCurrentColor{0};
};
#endif //__Pixel_MAINFRAM_H__



