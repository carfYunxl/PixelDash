#include "pch.h"
#include "CMNR.h"
#include "HF_IcInfoDlg.h"

namespace HF
{
	IMPLEMENT_DYNAMIC(HF_IcInfoDlg, CDialogEx)

		HF_IcInfoDlg::HF_IcInfoDlg(CWnd* pParent /*=nullptr*/)
		: CDialogEx(IDD_DIALOG_TEST_PAN, pParent)
	{

	}

	HF_IcInfoDlg::~HF_IcInfoDlg()
	{
	}

	void HF_IcInfoDlg::DoDataExchange(CDataExchange* pDX)
	{
		CDialogEx::DoDataExchange(pDX);
	}


	BEGIN_MESSAGE_MAP(HF_IcInfoDlg, CDialogEx)
	END_MESSAGE_MAP()
}
