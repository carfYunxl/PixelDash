#include "pch.h"
#include "CMNR.h"
#include "HF_MachineDlg.h"

namespace HF
{
	IMPLEMENT_DYNAMIC(HF_MachineDlg, CDialogEx)

		HF_MachineDlg::HF_MachineDlg(CWnd* pParent /*=nullptr*/)
		: CDialogEx(IDD_DIALOG_LINEAR_CONTROL, pParent) {

	}

	HF_MachineDlg::~HF_MachineDlg() {
	}

	void HF_MachineDlg::DoDataExchange(CDataExchange* pDX)
	{
		CDialogEx::DoDataExchange(pDX);
	}

	BEGIN_MESSAGE_MAP(HF_MachineDlg, CDialogEx)
	END_MESSAGE_MAP()
}

