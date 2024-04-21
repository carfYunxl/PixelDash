#ifndef __Pixel_SETTING_DLG_H__
#define __Pixel_SETTING_DLG_H__


class Pixel_SettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Pixel_SettingDlg)

public:
	Pixel_SettingDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Pixel_SettingDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SETTING };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};

#endif //__Pixel_SETTING_DLG_H__
