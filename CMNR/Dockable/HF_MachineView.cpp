#include "pch.h"
#include "HF_MachineView.h"
#include "HF_MainFrm.h"
#include "CMNR.h"

namespace HF
{
	HF_MachineView::HF_MachineView()  noexcept {}
	HF_MachineView::~HF_MachineView() {}

	BEGIN_MESSAGE_MAP(HF_MachineView, CDockablePane)
		ON_WM_CREATE()
		ON_WM_SIZE()
	END_MESSAGE_MAP()

	int HF_MachineView::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		if (CDockablePane::OnCreate(lpCreateStruct) == -1)
			return -1;

		m_dlgCtl.Create(IDD_DIALOG_LINEAR_CONTROL, this);
		m_dlgCtl.ShowWindow(SW_SHOW);
		return 0;
	}

	void HF_MachineView::OnSize(UINT nType, int cx, int cy)
	{
		CDockablePane::OnSize(nType, cx, cy);

		CRect rectClient;
		GetClientRect(rectClient);

		m_dlgCtl.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}
}