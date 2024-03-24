#include "pch.h"
#include "framework.h"
#include "TestPanel.h"
#include "Resource.h"
#include "MainFrm.h"
#include "CMNR.h"

TestPanel::TestPanel()  noexcept {}
TestPanel::~TestPanel() {}

BEGIN_MESSAGE_MAP(TestPanel, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int TestPanel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();
	
	m_Dialog.Create(IDD_DIALOG_TEST_PAN, this);
	m_Dialog.ShowWindow(SW_SHOW);
	return 0;
}

void TestPanel::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	CRect rectClient;
	GetClientRect(rectClient);

	m_Dialog.SetWindowPos(nullptr, rectClient.left, rectClient.top , rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

	//((CMainFrame*)GetParent())->Invalidate();
	//((CMainFrame*)GetParent())->RecalcLayout();
}
