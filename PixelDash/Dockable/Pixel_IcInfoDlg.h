#ifndef __Pixel_IC_INFO_DLG_H__
#define __Pixel_IC_INFO_DLG_H__

class Pixel_IcInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Pixel_IcInfoDlg)

public:
	typedef    BOOL(WINAPI* fnSetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);

	Pixel_IcInfoDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Pixel_IcInfoDlg();
	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_IC_VIEW };
#endif

protected:
	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL	OnInitDialog();
	afx_msg void	OnPaint();
	afx_msg HBRUSH	OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg void	OnBnClickedMfcbuttonIcInfoMore();
	afx_msg void	OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void	OnMouseLeave();
	afx_msg void	OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	CMFCButton	m_btnIcInfoMore;
	int			m_nWinHeight{0};
	int			m_nWinWidth{0};
	BOOL		m_bMouseTrack{ FALSE };
	HINSTANCE   m_hInst{NULL};
	CString		m_strI2cClock;

	fnSetLayeredWindowAttributes    m_pFun{ NULL };
private:
	CMFCButton m_btnInfoStatus;
	CFont		m_Font;
};

#endif //__Pixel_IC_INFO_DLG_H__
