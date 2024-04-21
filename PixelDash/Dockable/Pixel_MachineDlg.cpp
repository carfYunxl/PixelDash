#include "pch.h"
#include "PixelDash.h"
#include "Pixel_MachineDlg.h"

IMPLEMENT_DYNAMIC(Pixel_MachineDlg, CDialogEx)

Pixel_MachineDlg::Pixel_MachineDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MACHINE_VIEW, pParent) {

}

Pixel_MachineDlg::~Pixel_MachineDlg() {
}

void Pixel_MachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Pixel_MachineDlg, CDialogEx)
END_MESSAGE_MAP()

