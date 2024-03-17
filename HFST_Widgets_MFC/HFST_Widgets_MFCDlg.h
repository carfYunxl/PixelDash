#pragma once
#include "HFST_LogBox.hpp"
#include "afxdialogex.h"
#include <memory>

class CHFSTWidgetsMFCDlg : public CDialogEx
{
public:
	CHFSTWidgetsMFCDlg(CWnd* pParent = nullptr);
	~CHFSTWidgetsMFCDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HFST_WIDGETS_MFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBtnClose();
	afx_msg void OnBtnShowMax();
	afx_msg void OnBtnShowNormal();
	afx_msg void OnBtnWinTitle();
	afx_msg void OnNMCustomdrawListDrawerContent(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

private:
	void CreateUIControls();
	void CreateGdiObjects();
	void ResizeUI(int cx, int cy);
	void InitToolBar();
private:
	// Controls
	std::unique_ptr<CMFCButton> m_pBtnClose;		// 关闭按钮
	std::unique_ptr<CMFCButton> m_pBtnMax;			// 最大化按钮
	std::unique_ptr<CMFCButton> m_pBtnNormal;		// 正常显示按钮
	std::unique_ptr<CMFCButton> m_pBtnWindowTitle;	// 窗口图标

	CFont			m_CtrlFont_Normal;
	CFont			m_CtrlFont_Middle;
	CFont			m_CtrlFont_Big;
	CBrush			m_GrayBrush;
	CBrush			m_WhiteBrush;
	CBrush			m_RedBrush;
	CPen			m_Pen;

	BOOL			m_bCreateCtrls{ FALSE };
	HFST::LogBox	m_LogBox;
	CListCtrl		m_ListDrawerContent;
	CImageList		m_DrawerImglist;
	CToolBar		m_ToolBar;
};
