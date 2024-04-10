#pragma once
struct ErrorTypeSel {
	bool fgNoTouch;
	bool fgMoreTouch;
	bool fgPenUp;
	bool fgJitter;
	unsigned int uiJitterValue;
};
class CErrorTypeSelectDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CErrorTypeSelectDlg)

public:
	CErrorTypeSelectDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CErrorTypeSelectDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ERROR_TYPE_SEL_DIALOG };
#endif
	ErrorTypeSel  m_rErrTypeSel;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	BOOL  PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCheckJitter();
	afx_msg void OnEnKillfocusEditJitterValue();
};
