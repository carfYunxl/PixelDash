#include "pch.h"
#include "PixelDash.h"
#include "Pixel_IcInfoDlg.h"
#include "Pixel_MainFrm.h"

IMPLEMENT_DYNAMIC(Pixel_IcInfoDlg, CDialogEx)

Pixel_IcInfoDlg::Pixel_IcInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_IC_VIEW, pParent)
	, m_strI2cClock(_T("1500000"))
{
	m_Font.CreatePointFont(100, _T("Consolas"));
}

Pixel_IcInfoDlg::~Pixel_IcInfoDlg()
{
	if (m_hInst)
	{
		FreeLibrary(m_hInst);
	}

	DeleteObject(&m_Font);
}

void Pixel_IcInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCBUTTON_IC_INFO_MORE, m_btnIcInfoMore);
	DDX_Text(pDX, IDC_EDIT_IC_I2C_CLOCK, m_strI2cClock);
	DDX_Control(pDX, IDC_MFCBUTTON_IC_INFO_STATUS, m_btnInfoStatus);
}


BEGIN_MESSAGE_MAP(Pixel_IcInfoDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_MFCBUTTON_IC_INFO_MORE, &Pixel_IcInfoDlg::OnBnClickedMfcbuttonIcInfoMore)
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

BOOL Pixel_IcInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE, GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE) | WS_EX_LAYERED);
	m_hInst = LoadLibrary(L"User32.DLL");
	if (m_hInst)
	{   
		m_pFun = (fnSetLayeredWindowAttributes)GetProcAddress(m_hInst, "SetLayeredWindowAttributes");
		/*if (m_pFun)
			m_pFun(this->GetSafeHwnd(), 0, 25, LWA_ALPHA);*/
	}

	m_btnIcInfoMore.SetImage(IDB_PNG_FOLD);
	m_btnIcInfoMore.m_bRightImage = TRUE;

	m_btnInfoStatus.SetImage( IDB_PNG_CONNECT );
	m_btnInfoStatus.m_bRightImage = TRUE;
	m_btnInfoStatus.SetFont(&m_Font);
	return TRUE;
}

HBRUSH Pixel_IcInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void Pixel_IcInfoDlg::OnPaint()
{
	CPaintDC dc(this);

	/*CRect rect;
	GetClientRect(&rect);

	CBrush red_brush(RGB(255,0,0));
	CBrush* pOldBrush = dc.SelectObject(&red_brush);
	CPen* pOldPen = dc.SelectObject( CPen::FromHandle((HPEN)GetStockObject(NULL_PEN)) );

	int nCenterX = rect.Width() / 2;
	int nCenterY = rect.Width() / 2 - 25;

	int nRadius = nCenterX - 40;
	dc.Ellipse(nCenterX - nRadius, nCenterY - nRadius, nCenterX + nRadius, nCenterY + nRadius);

	dc.SelectObject(pOldBrush);
	dc.SelectObject(pOldPen);*/

	return CDialogEx::OnPaint();
}


LRESULT Pixel_IcInfoDlg::OnNcHitTest(CPoint point)
{
	UINT nHitTest = CDialogEx::OnNcHitTest(point);
	if ( nHitTest == HTCLIENT )
	{
		nHitTest = HTCAPTION;
	}
	return nHitTest;
}


void Pixel_IcInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	Invalidate();
}


void Pixel_IcInfoDlg::OnBnClickedMfcbuttonIcInfoMore()
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


void Pixel_IcInfoDlg::OnMouseHover(UINT nFlags, CPoint point)
{
	m_bMouseTrack = FALSE;
	//m_pFun(this->GetSafeHwnd(), 0, 255, LWA_ALPHA);
	CDialogEx::OnMouseHover(nFlags, point);
}


void Pixel_IcInfoDlg::OnMouseLeave()
{
	m_bMouseTrack = FALSE;
	//m_pFun(this->GetSafeHwnd(), 0, 25, LWA_ALPHA);
	CDialogEx::OnMouseLeave();
}


void Pixel_IcInfoDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	Pixel_MainFrame* pMain = (Pixel_MainFrame*)theApp.m_pMainWnd;
	if (!m_bMouseTrack)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.hwndTrack = this->m_hWnd;
		tme.dwHoverTime = 1;

		if (::_TrackMouseEvent(&tme))
		{
			m_bMouseTrack = TRUE;
		}
	}

	CDialogEx::OnMouseMove(nFlags, point);
}
