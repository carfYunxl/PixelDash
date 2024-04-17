#ifndef __HF_FILE_VIEW_H__
#define __HF_FILE_VIEW_H__

#include "HF_ViewTree.h"
#include "HF_Header.h"
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
	inline static std::map<const CString, DRAW_TYPE> m_sShapeMap{
		{ _T("直线"),	DRAW_TYPE::LINE},
		{ _T("矩形"),	DRAW_TYPE::RECTANGLE},
		{ _T("三角形"),	DRAW_TYPE::TRIANGLE},
		{ _T("圆形"),	DRAW_TYPE::CIRCLE},
		{ _T("椭圆"),	DRAW_TYPE::ELLIPSE},
		{ _T("米字形"),	DRAW_TYPE::MI},
		{ _T("口字形"),	DRAW_TYPE::KOU},
		{ _T("回字形"),	DRAW_TYPE::HUI},
		{ _T("螺旋形"),	DRAW_TYPE::LUO},
		{ _T("1000点"),	DRAW_TYPE::TEN}
	};

	HF_TestView() noexcept;
	virtual ~HF_TestView();

	void AdjustLayout();
	void OnChangeVisualStyle();
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
	DRAW_TYPE GetIndexByTitle(const CString& title);
private:

};

#endif //__HF_FILE_VIEW_H__

