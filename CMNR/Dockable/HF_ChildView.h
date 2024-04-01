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
	afx_msg void	OnPaint();
	afx_msg int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void	OnButtonViewLeft();
	afx_msg void	OnButtonViewRight();
	afx_msg void	OnButtonViewUp();
	afx_msg void	OnButtonViewFront();
	afx_msg void	OnButtonViewDown();
	afx_msg void	OnButtonViewBack();
	virtual BOOL	PreTranslateMessage(MSG* pMsg);
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
};

#endif //__HF_CHILD_VIEW_H__

