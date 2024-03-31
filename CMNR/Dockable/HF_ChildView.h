#ifndef __HF_CHILD_VIEW_H__
#define __HF_CHILD_VIEW_H__

namespace HF
{
	class CChildView : public CWnd
	{
	public:
		CChildView();

	protected:
		virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	public:
		virtual ~CChildView();

	protected:
		afx_msg void OnPaint();
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		DECLARE_MESSAGE_MAP()

	public:
		CMFCButton	m_BtnLeft;
		CMFCButton	m_BtnRight;
		CMFCButton	m_BtnFront;
		CMFCButton	m_BtnBack;
		CMFCButton	m_BtnUp;
		CMFCButton	m_BtnDown;
		CMFCButton	m_BtnCenter;
	};
}
#endif //__HF_CHILD_VIEW_H__

