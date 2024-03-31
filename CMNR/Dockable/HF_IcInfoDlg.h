#ifndef __HF_IC_INFO_DLG_H__
#define __HF_IC_INFO_DLG_H__

class HF_IcInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(HF_IcInfoDlg)

public:
	HF_IcInfoDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~HF_IcInfoDlg();
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
	DECLARE_MESSAGE_MAP()
public:
	CMFCButton	m_btnIcInfoMore;
	int			m_nWinHeight{0};
	int			m_nWinWidth{0};
	BOOL		m_bMouseTrack{ FALSE };
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#endif //__HF_IC_INFO_DLG_H__
