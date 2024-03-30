#pragma once
#include "ChildView.h"
#include "FileView.h"
#include "GraphicsView.h"
#include "OutputWnd.h"
#include "PropertiesWnd.h"
#include "TestPanel.h"
#include "CLinearCtlPanel.h"

#define UM_PROGRESS WM_USER+1

class CMainFrame : public CFrameWndEx
{
	
public:
	CMainFrame() noexcept;
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// 特性
protected:
	//CSplitterWnd m_wndSplitter;
public:

// 操作
public:

// 重写
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);

// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;
	CChildView		  m_wndView;
	CFileView		  m_wndFileView;
	//CGraphicsView	  m_wndGraphicsView;
	COutputWnd        m_wndOutput;

	CMFCToolBar       m_NewToolBar;
	CPropertiesWnd	  m_wndProperty;
	TestPanel		  m_wndTest;
	LinearCtlPane	  m_wndLinearCtl;

	CFont			  m_Font;

// 生成的消息映射函数
protected:
	afx_msg int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnSetFocus(CWnd *pOldWnd);
	afx_msg void	OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void	OnButtonIc();
	afx_msg void	OnButtonCon();
	afx_msg void	OnButtonDiscon();
	afx_msg void	OnButtonTest1();
	afx_msg void	OnButtonTest2();
	afx_msg void	OnButtonTest3();

	afx_msg LRESULT	OnProgress(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};


