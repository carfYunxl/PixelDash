#include "pch.h"
#include "Pixel_MachineView.h"
#include "Pixel_MainFrm.h"
#include "PixelDash.h"

Pixel_MachineView::Pixel_MachineView()  noexcept {}
Pixel_MachineView::~Pixel_MachineView() {}

BEGIN_MESSAGE_MAP(Pixel_MachineView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int Pixel_MachineView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_dlgCtl.Create(IDD_DIALOG_MACHINE_VIEW, this);
	m_dlgCtl.ShowWindow(SW_SHOW);
	return 0;
}

void Pixel_MachineView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	CRect rectClient;
	GetClientRect(rectClient);

	m_dlgCtl.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}