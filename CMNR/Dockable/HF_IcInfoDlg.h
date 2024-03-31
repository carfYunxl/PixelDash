#ifndef __HF_IC_INFO_DLG_H__
#define __HF_IC_INFO_DLG_H__

namespace HF
{
	class HF_IcInfoDlg : public CDialogEx
	{
		DECLARE_DYNAMIC(HF_IcInfoDlg)

	public:
		HF_IcInfoDlg(CWnd* pParent = nullptr);   // 标准构造函数
		virtual ~HF_IcInfoDlg();

		// 对话框数据
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_DIALOG_TEST_PAN };
#endif

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

		DECLARE_MESSAGE_MAP()
	};
}
#endif //__HF_IC_INFO_DLG_H__
