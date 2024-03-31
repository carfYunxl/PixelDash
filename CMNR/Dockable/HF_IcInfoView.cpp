#include "pch.h"
#include "HF_IcInfoView.h"
#include "HF_MainFrm.h"
#include "CMNR.h"

HF_IcInfoView::HF_IcInfoView()  noexcept {}
HF_IcInfoView::~HF_IcInfoView() {}

BEGIN_MESSAGE_MAP(HF_IcInfoView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int HF_IcInfoView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	m_Dialog.Create(IDD_DIALOG_IC_VIEW, this);
	m_Dialog.ShowWindow(SW_SHOW);
	return 0;
}

void HF_IcInfoView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	CRect rectClient;
	GetClientRect(rectClient);

	m_Dialog.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}
