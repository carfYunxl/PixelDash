#include "pch.h"
#include "Pixel_IcInfoView.h"
#include "Pixel_MainFrm.h"
#include "PixelDash.h"

Pixel_IcInfoView::Pixel_IcInfoView()  noexcept {}
Pixel_IcInfoView::~Pixel_IcInfoView() {}

BEGIN_MESSAGE_MAP(Pixel_IcInfoView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int Pixel_IcInfoView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	m_Dialog.Create(IDD_DIALOG_IC_VIEW, this);
	m_Dialog.ShowWindow(SW_SHOW);
	return 0;
}

void Pixel_IcInfoView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	CRect rectClient;
	GetClientRect(rectClient);

	m_Dialog.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}
