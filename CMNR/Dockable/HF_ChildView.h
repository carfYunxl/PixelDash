#ifndef __HF_CHILD_VIEW_H__
#define __HF_CHILD_VIEW_H__

#include "HF_IcInfoDlg.h"

class HF_ChildView : public CWnd
{
public:
	HF_ChildView();

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
	virtual ~HF_ChildView();

protected:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

private:
	CFont		m_Font;
	CMFCButton	m_BtnLeft;
	CMFCButton	m_BtnRight;
	CMFCButton	m_BtnFront;
	CMFCButton	m_BtnBack;
	CMFCButton	m_BtnUp;
	CMFCButton	m_BtnDown;
	CMFCButton	m_BtnCenter;

	CStatic		m_StaI2cClk;
	CEdit		m_EdtI2cClk;
	CStatic		m_StaI2cAddr;
	CComboBox	m_ComboI2cAddr;
	CStatic		m_StaBridge;
	CComboBox	m_ComboBridge;

	HF_IcInfoDlg m_IcInfoDlg;
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#endif //__HF_CHILD_VIEW_H__

