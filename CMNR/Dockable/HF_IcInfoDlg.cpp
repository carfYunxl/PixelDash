#include "pch.h"
#include "CMNR.h"
#include "HF_IcInfoDlg.h"

IMPLEMENT_DYNAMIC(HF_IcInfoDlg, CDialogEx)

HF_IcInfoDlg::HF_IcInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_IC_VIEW, pParent) {
}

HF_IcInfoDlg::~HF_IcInfoDlg()
{
}

void HF_IcInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCBUTTON_IC_INFO_MORE, m_btnIcInfoMore);
}


BEGIN_MESSAGE_MAP(HF_IcInfoDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_MFCBUTTON_IC_INFO_MORE, &HF_IcInfoDlg::OnBnClickedMfcbuttonIcInfoMore)
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

BOOL HF_IcInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE, GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE) | WS_EX_LAYERED);
	HINSTANCE    hInst = LoadLibrary(L"User32.DLL");
	if (hInst)
	{
		typedef    BOOL(WINAPI* MYFUNC)(HWND, COLORREF, BYTE, DWORD);
		MYFUNC    fun = NULL;
		//取得SetLayeredWindowAttributes函数指针     
		fun = (MYFUNC)GetProcAddress(hInst, "SetLayeredWindowAttributes");
		if (fun)fun(this->GetSafeHwnd(), 0, 255, LWA_ALPHA);
		FreeLibrary(hInst);
	}

	m_btnIcInfoMore.SetImage(IDB_PNG_IC_INFO_FOLD);
	m_btnIcInfoMore.m_bRightImage = TRUE;

	return TRUE;
}

HBRUSH HF_IcInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (nCtlColor)
	{
	case CTLCOLOR_DLG:
		HBRUSH aBrush;
		aBrush = CreateSolidBrush(RGB(240, 240, 240));
		hbr = aBrush;
		break;
	}
	return hbr;
}


void HF_IcInfoDlg::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	CBrush red_brush(RGB(255,0,0));
	CBrush* pOldBrush = dc.SelectObject(&red_brush);
	CPen* pOldPen = dc.SelectObject( CPen::FromHandle((HPEN)GetStockObject(NULL_PEN)) );

	int nCenterX = rect.Width() / 2;
	int nCenterY = rect.Width() / 2 - 25;

	int nRadius = nCenterX - 40;
	dc.Ellipse(nCenterX - nRadius, nCenterY - nRadius, nCenterX + nRadius, nCenterY + nRadius);

	dc.SelectObject(pOldBrush);
	dc.SelectObject(pOldPen);
}


LRESULT HF_IcInfoDlg::OnNcHitTest(CPoint point)
{
	UINT nHitTest = CDialogEx::OnNcHitTest(point);
	if (nHitTest == HTCLIENT)
		nHitTest = HTCAPTION;
	return nHitTest;
}


void HF_IcInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	Invalidate();
}


void HF_IcInfoDlg::OnBnClickedMfcbuttonIcInfoMore()
{
	CString strText;
	m_btnIcInfoMore.GetWindowTextW(strText);

	if (strText == _T("收起"))
	{
		CRect rect;
		GetWindowRect(rect);
		m_nWinHeight = rect.Height();
		m_nWinWidth = rect.Width();

		m_btnIcInfoMore.GetWindowRect(&rect);
		ScreenToClient(rect);

		SetWindowPos(nullptr, 0, 0, m_nWinWidth, rect.bottom, SWP_SHOWWINDOW | SWP_NOMOVE);

		m_btnIcInfoMore.SetWindowTextW(_T("显示更多"));
		m_btnIcInfoMore.SetImage(IDB_PNG_IC_INFO_MORE);
	}
	else if(strText == _T("显示更多"))
	{
		SetWindowPos(nullptr, 0, 0, m_nWinWidth, m_nWinHeight, SWP_SHOWWINDOW | SWP_NOMOVE);

		m_btnIcInfoMore.SetWindowTextW(_T("收起"));
		m_btnIcInfoMore.SetImage(IDB_PNG_IC_INFO_FOLD);
	}
}


void HF_IcInfoDlg::OnMouseHover(UINT nFlags, CPoint point)
{
	m_bMouseTrack = FALSE;
	TRACE("Mouse Hover!\n");
	CDialogEx::OnMouseHover(nFlags, point);
}


void HF_IcInfoDlg::OnMouseLeave()
{
	m_bMouseTrack = TRUE;
	TRACE("Mouse Leave!\n");
	CDialogEx::OnMouseLeave();
}


void HF_IcInfoDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	TRACE("Mouse Move!\n");
	if (!m_bMouseTrack)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = 10;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);

		m_bMouseTrack = TRUE;
		Invalidate(FALSE);
	}

	CDialogEx::OnMouseMove(nFlags, point);
}
