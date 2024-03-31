#ifndef __HF_FILE_VIEW_H__
#define __HF_FILE_VIEW_H__

#include "HF_ViewTree.h"

class HF_TestViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class HF_TestView : public CDockablePane
{
public:
	HF_TestView() noexcept;

	void AdjustLayout();
	void OnChangeVisualStyle();
protected:

	CViewTree			m_wndTestView;
	CImageList			m_FileViewImages;
	HF_TestViewToolBar	m_wndToolBar;
	CFont				m_Font;

protected:
	void FillFileView();

public:
	virtual ~HF_TestView();

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

#endif //__HF_FILE_VIEW_H__

