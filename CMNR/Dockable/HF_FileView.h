#ifndef __HF_FILE_VIEW_H__
#define __HF_FILE_VIEW_H__

#include "HF_ViewTree.h"

namespace HF
{
	class HF_FileViewToolBar : public CMFCToolBar
	{
		virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
		{
			CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
		}

		virtual BOOL AllowShowOnList() const { return FALSE; }
	};

	class HF_FileView : public CDockablePane
	{
	public:
		HF_FileView() noexcept;

		void AdjustLayout();
		void OnChangeVisualStyle();
	protected:

		CViewTree			m_wndFileView;
		CImageList			m_FileViewImages;
		HF_FileViewToolBar	m_wndToolBar;

	protected:
		void FillFileView();

	public:
		virtual ~HF_FileView();

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
}

#endif //__HF_FILE_VIEW_H__

