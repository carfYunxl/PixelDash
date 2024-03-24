#pragma once
#include "ViewTree.h"

class CGraphicsViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CGraphicsView : public CDockablePane
{
	// 构造
public:
	CGraphicsView() noexcept;

	void AdjustLayout();
	void OnChangeVisualStyle();

	// 特性
protected:

	CViewTree m_wndGraphicsView;
	CImageList m_FileViewImages;
	CGraphicsViewToolBar m_wndToolBar;

protected:
	void FillGraphicsView();

	// 实现
public:
	virtual ~CGraphicsView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnProperties();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenWith();
	afx_msg void OnDummyCompile();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()
};

