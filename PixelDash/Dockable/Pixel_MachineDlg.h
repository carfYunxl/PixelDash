#ifndef __Pixel_MACHINE_DLG_H__
#define __Pixel_MACHINE_DLG_H__

class Pixel_MachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Pixel_MachineDlg)

public:
	Pixel_MachineDlg(CWnd* pParent = nullptr);
	virtual ~Pixel_MachineDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_LINEAR_CONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
};

#endif //__Pixel_MACHINE_DLG_H__
