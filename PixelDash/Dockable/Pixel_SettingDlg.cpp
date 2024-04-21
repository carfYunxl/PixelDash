#include "pch.h"
#include "PixelDash.h"
#include "Pixel_SettingDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(Pixel_SettingDlg, CDialogEx)

Pixel_SettingDlg::Pixel_SettingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SETTING, pParent)
{

}

Pixel_SettingDlg::~Pixel_SettingDlg()
{
}

void Pixel_SettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Pixel_SettingDlg, CDialogEx)
END_MESSAGE_MAP()
