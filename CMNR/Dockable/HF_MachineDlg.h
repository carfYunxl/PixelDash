#ifndef __HF_MACHINE_DLG_H__
#define __HF_MACHINE_DLG_H__

class HF_MachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(HF_MachineDlg)

public:
	HF_MachineDlg(CWnd* pParent = nullptr);
	virtual ~HF_MachineDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_LINEAR_CONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
};

#endif //__HF_MACHINE_DLG_H__
