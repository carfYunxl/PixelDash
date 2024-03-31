#include "pch.h"
#include "CMNR.h"
#include "HF_SettingDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(HF_SettingDlg, CDialogEx)

HF_SettingDlg::HF_SettingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SETTING, pParent)
{

}

HF_SettingDlg::~HF_SettingDlg()
{
}

void HF_SettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(HF_SettingDlg, CDialogEx)
END_MESSAGE_MAP()
