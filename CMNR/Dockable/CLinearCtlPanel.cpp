#include "pch.h"
#include "framework.h"
#include "CLinearCtlPanel.h"
#include "Resource.h"
#include "MainFrm.h"
#include "CMNR.h"

LinearCtlPane::LinearCtlPane()  noexcept {}
LinearCtlPane::~LinearCtlPane() {}

BEGIN_MESSAGE_MAP(LinearCtlPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int LinearCtlPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_dlgCtl.Create(IDD_DIALOG_LINEAR_CONTROL, this);
	m_dlgCtl.ShowWindow(SW_SHOW);
	return 0;
}

void LinearCtlPane::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	CRect rectClient;
	GetClientRect(rectClient);

	m_dlgCtl.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

	//((CMainFrame*)GetParent())->Invalidate();
	//((CMainFrame*)GetParent())->RecalcLayout();
}