#ifndef __HF_SETTING_DLG_H__
#define __HF_SETTING_DLG_H__


class HF_SettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(HF_SettingDlg)

public:
	HF_SettingDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~HF_SettingDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SETTING };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};

#endif //__HF_SETTING_DLG_H__
