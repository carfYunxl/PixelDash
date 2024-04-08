#ifndef __HF_FILE_VIEW_H__
#define __HF_FILE_VIEW_H__

#include "HF_ViewTree.h"
#include <map>

class HF_PropertiesWnd;

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

	inline static std::map<const CString, int> m_sShapeMap{
		{ _T("直线"),	0},
		{ _T("矩形"),	1},
		{ _T("三角形"),	2},
		{ _T("圆形"),	3},
		{ _T("椭圆"),	4},
		{ _T("米字形"),	5},
		{ _T("口字形"),	6},
		{ _T("回字形"),	7},
		{ _T("螺旋形"),	8},
		{ _T("字形"),	9},
	};

	HF_TestView() noexcept;
	virtual ~HF_TestView();

	void AdjustLayout();
	void OnChangeVisualStyle();
	void SetPropertyWnd(HF_PropertiesWnd* pWndProperty) { m_pWndProperty = pWndProperty; }
protected:

	CViewTree			m_wndTestView;
	CImageList			m_FileViewImages;
	HF_TestViewToolBar	m_wndToolBar;
	CFont				m_Font;

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
	afx_msg void OnTvnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

	void FillFileView();

private:
	int GetIndexByTitle(const CString& title);
private:
	HF_PropertiesWnd* m_pWndProperty{ nullptr };
};

#endif //__HF_FILE_VIEW_H__

