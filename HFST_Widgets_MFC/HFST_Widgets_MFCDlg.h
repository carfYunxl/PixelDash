#pragma once

#include <memory>

namespace ADV_RECORD
{
	constexpr int ID_CLOSE = 0x400;
	constexpr int ID_MAX = 0x401;
	constexpr int ID_NORMAL = 0x402;
	constexpr int ID_WIN_TITLE = 0x403;

	const COLORREF GRAY = RGB(0x3E, 0x3E, 0x3E);
	const COLORREF WHITE = RGB(0xFF, 0xFF, 0xFF);
	const COLORREF RED = RGB(0xFF, 0x00, 0x00);

	constexpr int ICON_SIZE = 40;
	constexpr int GAP = 5;
	constexpr int HEIGHT = ICON_SIZE + GAP * 2;

	constexpr int PEN_SIZE = 5;
	constexpr int LEVEL1_OFFSETX = 60;
	constexpr int LEVEL2_OFFSETX = 200;

	constexpr int OFFSETY = 30;

	constexpr int MCU_ROM = 0x02;
	constexpr int OFTV_AutoTrim_Offset = 0xBCA1;
}

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
	DECLARE_MESSAGE_MAP()

private:
	void CreateUIControls();
	void CreateGdiObjects();
	void ResizeUI(int cx, int cy);
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
public:
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
