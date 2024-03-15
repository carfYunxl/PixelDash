#include "pch.h"
#include "framework.h"
#include "HFST_Widgets_MFC.h"
#include "HFST_Widgets_MFCDlg.h"
#include "afxdialogex.h"
#include "CCtrlLayout.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CHFSTWidgetsMFCDlg::CHFSTWidgetsMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HFST_WIDGETS_MFC_DIALOG, pParent)
	, m_pBtnClose(std::make_unique<CMFCButton>())
	, m_pBtnMax(std::make_unique<CMFCButton>())
	, m_pBtnNormal(std::make_unique<CMFCButton>())
	, m_pBtnWindowTitle(std::make_unique<CMFCButton>())
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CHFSTWidgetsMFCDlg::~CHFSTWidgetsMFCDlg() {
	DeleteObject(&m_CtrlFont_Normal);
	DeleteObject(&m_CtrlFont_Middle);
	DeleteObject(&m_CtrlFont_Big);
	DeleteObject(&m_GrayBrush);
	DeleteObject(&m_WhiteBrush);
	DeleteObject(&m_Pen);

	m_pBtnClose->DestroyWindow();
	m_pBtnMax->DestroyWindow();
	m_pBtnNormal->DestroyWindow();
	m_pBtnWindowTitle->DestroyWindow();
}

void CHFSTWidgetsMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHFSTWidgetsMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CHFSTWidgetsMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	CreateGdiObjects();
	CreateUIControls();

	return TRUE;
}

void CHFSTWidgetsMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CHFSTWidgetsMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		CRect rect;
		GetClientRect(&rect);
		rect.bottom = rect.top + ADV_RECORD::HEIGHT;
		dc.FillRect(&rect, &m_GrayBrush);

		GetClientRect(&rect);
		rect.top += ADV_RECORD::HEIGHT;
		dc.FillRect(&rect, &m_WhiteBrush);
		CDialogEx::OnPaint();
	}
}

HCURSOR CHFSTWidgetsMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CHFSTWidgetsMFCDlg::CreateUIControls()
{
	CRect recClient;
	GetClientRect(&recClient);
	int nX = recClient.right - ADV_RECORD::GAP - ADV_RECORD::ICON_SIZE;
	int nY = ADV_RECORD::GAP;
	// Close button
	m_pBtnClose->Create(_T(""), WS_VISIBLE | WS_CHILD, CRect(nX, nY, nX + ADV_RECORD::ICON_SIZE, nY + ADV_RECORD::ICON_SIZE), this, ADV_RECORD::ID_CLOSE);
	m_pBtnClose->SetImage(IDB_PNG_UI_CLOSE);
	m_pBtnClose->m_nFlatStyle = CMFCButton::FlatStyle::BUTTONSTYLE_FLAT;
	m_pBtnClose->SetFaceColor(ADV_RECORD::GRAY);
	m_pBtnClose->m_bRightImage = false;

	nX -= (ADV_RECORD::GAP + ADV_RECORD::ICON_SIZE);
	// Show Max Button
	m_pBtnMax->Create(_T(""), WS_VISIBLE | WS_CHILD, CRect(nX, nY, nX + ADV_RECORD::ICON_SIZE, nY + ADV_RECORD::ICON_SIZE), this, ADV_RECORD::ID_MAX);
	m_pBtnMax->SetImage(IDB_PNG_UI_MAX);
	m_pBtnMax->m_nFlatStyle = CMFCButton::FlatStyle::BUTTONSTYLE_FLAT;
	m_pBtnMax->SetFaceColor(ADV_RECORD::GRAY);
	m_pBtnMax->m_bRightImage = false;

	nX -= (ADV_RECORD::GAP + ADV_RECORD::ICON_SIZE);
	// Show Normal Button
	m_pBtnNormal->Create(_T(""), WS_VISIBLE | WS_CHILD, CRect(nX, nY, nX + ADV_RECORD::ICON_SIZE, nY + ADV_RECORD::ICON_SIZE), this, ADV_RECORD::ID_NORMAL);
	m_pBtnNormal->SetImage(IDB_PNG_UI_NORMAL);
	m_pBtnNormal->m_nFlatStyle = CMFCButton::FlatStyle::BUTTONSTYLE_FLAT;
	m_pBtnNormal->SetFaceColor(ADV_RECORD::GRAY);
	m_pBtnNormal->m_bRightImage = false;

	nX = ADV_RECORD::GAP;
	nY = ADV_RECORD::GAP;
	// Window Title Button
	m_pBtnWindowTitle->Create(_T("CMNR"), WS_VISIBLE | WS_CHILD | BS_LEFTTEXT, CRect(nX, nY, nX + 200, nY + ADV_RECORD::ICON_SIZE), this, ADV_RECORD::ID_WIN_TITLE);
	m_pBtnWindowTitle->SetImage(IDB_PNG_UI_WINDOW_ICON);
	m_pBtnWindowTitle->m_nFlatStyle = CMFCButton::FlatStyle::BUTTONSTYLE_FLAT;
	m_pBtnWindowTitle->SetFaceColor(ADV_RECORD::GRAY);
	m_pBtnWindowTitle->m_bRightImage = FALSE;
	m_pBtnWindowTitle->SetFont(&m_CtrlFont_Big);
	m_pBtnWindowTitle->SetTextColor(ADV_RECORD::WHITE);
	m_pBtnWindowTitle->m_bGrayDisabled = TRUE;
	m_pBtnWindowTitle->m_bResponseOnButtonDown = FALSE;
	m_pBtnWindowTitle->m_nAlignStyle = CMFCButton::AlignStyle::ALIGN_LEFT;

	m_bCreateCtrls = TRUE;
}

void CHFSTWidgetsMFCDlg::CreateGdiObjects()
{
	VERIFY(
		m_CtrlFont_Normal.CreateFont(
			20,
			0,
			0,
			0,
			FW_NORMAL,
			FALSE,
			FALSE,
			0,
			ANSI_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_SWISS,
			_T("微软雅黑")
		)
	);

	VERIFY(
		m_CtrlFont_Middle.CreateFont(
			25,
			0,
			0,
			0,
			FW_BOLD,
			FALSE,
			FALSE,
			0,
			ANSI_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_SWISS,
			_T("微软雅黑")
		)
	);

	VERIFY(
		m_CtrlFont_Big.CreateFont(
			30,
			0,
			0,
			0,
			FW_BOLD,
			FALSE,
			FALSE,
			0,
			ANSI_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_SWISS,
			_T("微软雅黑")
		)
	);

	m_GrayBrush.CreateSolidBrush(ADV_RECORD::GRAY);
	m_WhiteBrush.CreateSolidBrush(ADV_RECORD::WHITE);

	m_Pen.CreatePen(PS_SOLID, ADV_RECORD::PEN_SIZE, ADV_RECORD::RED);
}

void CHFSTWidgetsMFCDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 1366;
	lpMMI->ptMinTrackSize.y = 768;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CHFSTWidgetsMFCDlg::ResizeUI(int cx, int cy)
{
	if (!m_bCreateCtrls)
		return;

	HFST::CCtrlLayout layout;

	//**************** Window Title *******************//
	layout.SetStartPoint({ ADV_RECORD::GAP , ADV_RECORD::GAP });
	layout.SetCtrlHeight(ADV_RECORD::ICON_SIZE);
	layout.SetGap(ADV_RECORD::GAP);
	layout.Add(*m_pBtnWindowTitle, 200, SWP_SHOWWINDOW);

	//**************** Close/Max/Min Button *******************//
	layout.SetStartPoint({ cx - (ADV_RECORD::GAP + ADV_RECORD::ICON_SIZE) * 3 , ADV_RECORD::GAP });
	layout.Add(*m_pBtnNormal, ADV_RECORD::ICON_SIZE, SWP_SHOWWINDOW);
	layout.Add(*m_pBtnMax, ADV_RECORD::ICON_SIZE, SWP_SHOWWINDOW);
	layout.Add(*m_pBtnClose, ADV_RECORD::ICON_SIZE, SWP_SHOWWINDOW);
}

void CHFSTWidgetsMFCDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	ResizeUI(cx, cy);
}
