#include "pch.h"
#include "CMNR.h"
#include "HF_ChildView.h"
#include "HF_MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HF_ChildView::HF_ChildView()
{
}

HF_ChildView::~HF_ChildView()
{

}


BEGIN_MESSAGE_MAP(HF_ChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

BOOL HF_ChildView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		::LoadCursor(nullptr, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH), nullptr);
	return TRUE;
}

void HF_ChildView::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);

	constexpr int WIDTH = 180;
	constexpr int BTN_WIDTH = 40;
	constexpr int GAP = 10;
	int nCenterX = rect.right - WIDTH / 2;
	int nCenterY = rect.bottom - WIDTH / 2;

	CRect recLeft(
		nCenterX - BTN_WIDTH / 2 - BTN_WIDTH,	// left
		nCenterY - BTN_WIDTH / 2,				// top
		nCenterX - BTN_WIDTH / 2,				// right
		nCenterY - BTN_WIDTH / 2 + BTN_WIDTH	// bootm
	);

	CRect recRight(
		nCenterX + BTN_WIDTH / 2,				// left
		nCenterY - BTN_WIDTH / 2,				// top
		nCenterX + BTN_WIDTH / 2 + BTN_WIDTH,	// right
		nCenterY - BTN_WIDTH / 2 + BTN_WIDTH	// bootm
	);

	CRect recFront(
		nCenterX - BTN_WIDTH / 2,				// left
		nCenterY - BTN_WIDTH / 2 - BTN_WIDTH,	// top
		nCenterX - BTN_WIDTH / 2 + BTN_WIDTH,	// right
		nCenterY - BTN_WIDTH / 2				// bootm
	);

	CRect recBack(
		nCenterX - BTN_WIDTH / 2,				// left
		nCenterY + BTN_WIDTH / 2,				// top
		nCenterX - BTN_WIDTH / 2 + BTN_WIDTH,	// right
		nCenterY + BTN_WIDTH / 2 + BTN_WIDTH	// bootm
	);

	CRect recUp(
		nCenterX - BTN_WIDTH * 2,				// left
		nCenterY - BTN_WIDTH * 2,				// top
		nCenterX - BTN_WIDTH,					// right
		nCenterY - BTN_WIDTH					// bootm
	);

	CRect recDown(
		nCenterX + BTN_WIDTH,					// left
		nCenterY - BTN_WIDTH * 2,				// top
		nCenterX + BTN_WIDTH * 2,				// right
		nCenterY - BTN_WIDTH					// bootm
	);

	CRect recCenter(
		nCenterX - 16,				// left
		nCenterY - 16,				// top
		nCenterX + 16,				// right
		nCenterY + 16				// bootm
	);

	if (m_BtnLeft.m_hWnd == NULL)
	{
		m_BtnLeft.Create(_T(""), WS_CHILD | WS_VISIBLE, recLeft, this, IDC_BUTTON_VIEW_LEFT);
		m_BtnLeft.SetImage(IDB_PNG_LEFT);

		m_BtnRight.Create(_T(""), WS_CHILD | WS_VISIBLE, recRight, this, IDC_BUTTON_VIEW_RIGHT);
		m_BtnRight.SetImage(IDB_PNG_RIGHT);

		m_BtnFront.Create(_T(""), WS_CHILD | WS_VISIBLE, recFront, this, IDC_BUTTON_VIEW_FRONT);
		m_BtnFront.SetImage(IDB_PNG_FRONT);

		m_BtnBack.Create(_T(""), WS_CHILD | WS_VISIBLE, recBack, this, IDC_BUTTON_VIEW_BACK);
		m_BtnBack.SetImage(IDB_PNG_BACK);

		m_BtnUp.Create(_T(""), WS_CHILD | WS_VISIBLE, recUp, this, IDC_BUTTON_VIEW_UP);
		m_BtnUp.SetImage(IDB_PNG_UP);

		m_BtnDown.Create(_T(""), WS_CHILD | WS_VISIBLE, recDown, this, IDC_BUTTON_VIEW_DOWN);
		m_BtnDown.SetImage(IDB_PNG_DOWN);

		m_BtnCenter.Create(_T(""), WS_CHILD | WS_VISIBLE, recCenter, this, IDC_BUTTON_VIEW_DOWN);
		m_BtnCenter.SetImage(IDB_PNG_CENTER);

		m_IcInfoDlg.Create(IDD_DIALOG_IC_VIEW, this);
		m_IcInfoDlg.ShowWindow(SW_SHOW);
	}
	else
	{
		m_BtnLeft.MoveWindow(recLeft);
		m_BtnRight.MoveWindow(recRight);
		m_BtnFront.MoveWindow(recFront);
		m_BtnBack.MoveWindow(recBack);
		m_BtnUp.MoveWindow(recUp);
		m_BtnDown.MoveWindow(recDown);
		m_BtnCenter.MoveWindow(recCenter);

		/*GetWindowRect(&rect);

		CRect info;
		m_IcInfoDlg.GetWindowRect(&info);

		CRect recMenu;
		((HF_MainFrame*)GetParent())->m_wndMenuBar.GetWindowRect(&recMenu);

		CString strWinTitle;
		m_IcInfoDlg.m_btnIcInfoMore.GetWindowTextW(strWinTitle);
		if (strWinTitle == _T("显示更多"))
		{
			CRect iRec;
			m_IcInfoDlg.m_btnIcInfoMore.GetWindowRect(&iRec);
			m_IcInfoDlg.SetWindowPos(nullptr, 200, 200, info.Width(), iRec.bottom, SWP_SHOWWINDOW);
		}
		else if(strWinTitle == _T("隐藏"))
		{

		}*/
	}
}

int HF_ChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}

void HF_ChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	TRACE("Mouse Move!\n");

	CWnd::OnMouseMove(nFlags, point);
}
