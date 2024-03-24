#include "pch.h"
#include "CMNR.h"
#include "CLinearCtlDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CLinearCtlDlg, CDialogEx)

CLinearCtlDlg::CLinearCtlDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_LINEAR_CONTROL, pParent)
{

}

CLinearCtlDlg::~CLinearCtlDlg()
{
}

void CLinearCtlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLinearCtlDlg, CDialogEx)
END_MESSAGE_MAP()

