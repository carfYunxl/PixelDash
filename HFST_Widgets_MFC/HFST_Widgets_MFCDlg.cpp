#include "pch.h"
#include "framework.h"
#include "HFST_Widgets_MFC.h"
#include "HFST_Widgets_MFCDlg.h"
#include "HFST_CtrlLayout.hpp"
#include "HFST_Core.hpp"

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
	DDX_Control(pDX, IDC_LIST_LOG_BOX, m_LogBox);
	DDX_Control(pDX, IDC_LIST_DRAWER_CONTENT, m_ListDrawerContent);
}

BEGIN_MESSAGE_MAP(CHFSTWidgetsMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(HFST::UI::ID_CLOSE,		&CHFSTWidgetsMFCDlg::OnBtnClose)
	ON_BN_CLICKED(HFST::UI::ID_MAX,			&CHFSTWidgetsMFCDlg::OnBtnShowMax)
	ON_BN_CLICKED(HFST::UI::ID_NORMAL,		&CHFSTWidgetsMFCDlg::OnBtnShowNormal)
	ON_BN_CLICKED(HFST::UI::ID_WIN_TITLE,	&CHFSTWidgetsMFCDlg::OnBtnWinTitle)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_DRAWER_CONTENT, &CHFSTWidgetsMFCDlg::OnNMCustomdrawListDrawerContent)
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

	CString title;
	CString ver;
	ver = HFST::GetFileVersion();
	ver.Replace(_T(", "), _T("."));
	m_pBtnWindowTitle->GetWindowText(title);
	title.Append(_T(" - v"));
	title.Append(ver);
	m_pBtnWindowTitle->SetWindowTextW(title);

	m_LogBox.AppendString(_T("Hello"), RGB(255,0,0), RGB(255,255,255));

	InitToolBar();

	CRect rect;
	GetClientRect(&rect);
	ResizeUI(rect.Width(), rect.Height());

	// Set up the image list.
	CWinApp* pApp = AfxGetApp();
	HICON    hStan, hKyle, hCartman, hKenny;
	m_DrawerImglist.Create(48, 48, ILC_COLOR24 | ILC_MASK, 4, 1);

	hStan = reinterpret_cast<HICON>(
		::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON1),
			IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR));
	hKyle = reinterpret_cast<HICON>(
		::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON2),
			IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR));
	hCartman = reinterpret_cast<HICON>(
		::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON3),
			IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR));
	hKenny = reinterpret_cast<HICON>(
		::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON4),
			IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR));

	m_DrawerImglist.Add(hStan);
	m_DrawerImglist.Add(hKyle);
	m_DrawerImglist.Add(hCartman);
	m_DrawerImglist.Add(hKenny);

	m_ListDrawerContent.SetImageList(&m_DrawerImglist, LVSIL_SMALL);

	m_ListDrawerContent.GetClientRect(&rect);
	m_ListDrawerContent.InsertColumn(0, _T(""), LVCFMT_LEFT, rect.Width(), 0);

	m_ListDrawerContent.InsertItem(0, _T("精准度测试"), 0);
	m_ListDrawerContent.InsertItem(1, _T("直线度测试"), 1);
	m_ListDrawerContent.InsertItem(2, _T("灵敏度测试"), 2);
	m_ListDrawerContent.InsertItem(3, _T("重复打点测试撒大大阿斯顿撒大"), 3);

	m_ListDrawerContent.SetColumnWidth(0, LVSCW_AUTOSIZE);

	m_ListDrawerContent.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
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
		rect.bottom = rect.top + HFST::UI::HEIGHT;
		dc.FillRect(&rect, &m_GrayBrush);

		GetClientRect(&rect);
		rect.top += HFST::UI::HEIGHT;
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
	int nX = recClient.right - HFST::UI::GAP - HFST::UI::ICON_SIZE;
	int nY = HFST::UI::GAP;
	// Close button
	m_pBtnClose->Create(_T(""), WS_VISIBLE | WS_CHILD, CRect(nX, nY, nX + HFST::UI::ICON_SIZE, nY + HFST::UI::ICON_SIZE), this, HFST::UI::ID_CLOSE);
	m_pBtnClose->SetImage(IDB_PNG_UI_CLOSE);
	m_pBtnClose->m_nFlatStyle = CMFCButton::FlatStyle::BUTTONSTYLE_FLAT;
	m_pBtnClose->SetFaceColor(HFST::UI::GRAY);
	m_pBtnClose->m_bRightImage = false;
	m_pBtnClose->m_bTransparent = FALSE;
	m_pBtnClose->m_bDontUseWinXPTheme = TRUE;

	nX -= (HFST::UI::GAP + HFST::UI::ICON_SIZE);
	// Show Max Button
	m_pBtnMax->Create(_T(""), WS_VISIBLE | WS_CHILD, CRect(nX, nY, nX + HFST::UI::ICON_SIZE, nY + HFST::UI::ICON_SIZE), this, HFST::UI::ID_MAX);
	m_pBtnMax->SetImage(IDB_PNG_UI_MAX);
	m_pBtnMax->m_nFlatStyle = CMFCButton::FlatStyle::BUTTONSTYLE_FLAT;
	m_pBtnMax->SetFaceColor(HFST::UI::GRAY);
	m_pBtnMax->m_bRightImage = false;
	m_pBtnMax->m_bTransparent = FALSE;
	m_pBtnMax->m_bDontUseWinXPTheme = TRUE;

	nX -= (HFST::UI::GAP + HFST::UI::ICON_SIZE);
	// Show Normal Button
	m_pBtnNormal->Create(_T(""), WS_VISIBLE | WS_CHILD, CRect(nX, nY, nX + HFST::UI::ICON_SIZE, nY + HFST::UI::ICON_SIZE), this, HFST::UI::ID_NORMAL);
	m_pBtnNormal->SetImage(IDB_PNG_UI_NORMAL);
	m_pBtnNormal->m_nFlatStyle = CMFCButton::FlatStyle::BUTTONSTYLE_FLAT;
	m_pBtnNormal->SetFaceColor(HFST::UI::GRAY);
	m_pBtnNormal->m_bRightImage = false;
	m_pBtnNormal->m_bTransparent = FALSE;
	m_pBtnNormal->m_bDontUseWinXPTheme = TRUE;

	nX = HFST::UI::GAP;
	nY = HFST::UI::GAP;
	// Window Title Button
	m_pBtnWindowTitle->Create(_T("CMNR"), WS_VISIBLE | WS_CHILD | BS_LEFTTEXT, CRect(nX, nY, nX + 400, nY + HFST::UI::ICON_SIZE), this, HFST::UI::ID_WIN_TITLE);
	m_pBtnWindowTitle->SetImage(IDB_PNG_UI_WINDOW_ICON);
	m_pBtnWindowTitle->m_nFlatStyle = CMFCButton::FlatStyle::BUTTONSTYLE_FLAT;
	m_pBtnWindowTitle->SetFaceColor(HFST::UI::GRAY);
	m_pBtnWindowTitle->m_bRightImage = FALSE;
	m_pBtnWindowTitle->SetFont(&m_CtrlFont_Big);
	m_pBtnWindowTitle->SetTextColor(HFST::UI::WHITE);
	m_pBtnWindowTitle->m_bGrayDisabled = TRUE;
	m_pBtnWindowTitle->m_bResponseOnButtonDown = FALSE;
	m_pBtnWindowTitle->m_nAlignStyle = CMFCButton::AlignStyle::ALIGN_LEFT;
	m_pBtnWindowTitle->m_bTransparent = FALSE;
	m_pBtnWindowTitle->m_bDontUseWinXPTheme = TRUE;

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

	m_GrayBrush.CreateSolidBrush(HFST::UI::GRAY);
	m_WhiteBrush.CreateSolidBrush(HFST::UI::WHITE);

	m_Pen.CreatePen(PS_SOLID, HFST::UI::PEN_SIZE, HFST::UI::RED);
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

	HFST::CtrlLayout layout;

	//**************** Window Title *******************//
	layout.SetStartPoint({ HFST::UI::GAP , HFST::UI::GAP });
	layout.SetCtrlHeight(HFST::UI::ICON_SIZE);
	layout.SetGap(HFST::UI::GAP);
	layout.Add(*m_pBtnWindowTitle, 400, SWP_SHOWWINDOW);

	//**************** Close/Max/Min Button *******************//
	layout.SetStartPoint({ cx - (HFST::UI::GAP + HFST::UI::ICON_SIZE) * 3 , HFST::UI::GAP });
	layout.Add(*m_pBtnNormal, HFST::UI::ICON_SIZE, SWP_SHOWWINDOW);
	layout.Add(*m_pBtnMax, HFST::UI::ICON_SIZE, SWP_SHOWWINDOW);
	layout.Add(*m_pBtnClose, HFST::UI::ICON_SIZE, SWP_SHOWWINDOW);
	//**************** Tool Bar *******************//
	layout.SetStartPoint({0, HFST::UI::HEIGHT + HFST::UI::GAP });
	layout.SetCtrlHeight(55);
	layout.SetGap(HFST::UI::GAP);
	layout.Add(m_ToolBar, cx, SWP_SHOWWINDOW);
	//**************** ListCtrl *******************************//
	layout.SetStartPoint({ HFST::UI::GAP, HFST::UI::HEIGHT + HFST::UI::GAP + 55 });
	layout.SetCtrlHeight(cy-HFST::UI::HEIGHT- HFST::UI::GAP*2 - 55);
	layout.SetGap(HFST::UI::GAP);
	layout.Add(m_ListDrawerContent, int(cx * 0.15), SWP_SHOWWINDOW);
	//**************** LogBox *********************************//
	layout.SetStartPoint({ int(cx * 0.15) + HFST::UI::GAP * 2, int(cy * 0.8)});
	layout.SetCtrlHeight(int(cy * 0.2) - HFST::UI::GAP);
	layout.SetGap(HFST::UI::GAP);
	layout.Add(m_LogBox, int(cx * 0.85) - HFST::UI::GAP * 3, SWP_SHOWWINDOW);

}

void CHFSTWidgetsMFCDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	ResizeUI(cx, cy);

	Invalidate();
}


LRESULT CHFSTWidgetsMFCDlg::OnNcHitTest(CPoint point)
{
	CRect rect;
	GetClientRect(&rect);
	rect.bottom = HFST::UI::HEIGHT;

	::ScreenToClient(this->GetSafeHwnd(), &point);

	return rect.PtInRect(point) ? HTCAPTION : CDialogEx::OnNcHitTest(point);
}

void CHFSTWidgetsMFCDlg::OnBtnClose()
{
	EndDialog(0);
}
void CHFSTWidgetsMFCDlg::OnBtnShowMax()
{
	ShowWindow(SW_SHOWMAXIMIZED);
}
void CHFSTWidgetsMFCDlg::OnBtnShowNormal()
{
	ShowWindow(SW_SHOWNORMAL);
}
void CHFSTWidgetsMFCDlg::OnBtnWinTitle()
{

}

void CHFSTWidgetsMFCDlg::OnNMCustomdrawListDrawerContent(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

	*pResult = 0;

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage) {
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		int  nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );
		BOOL bListHasFocus = ( m_ListDrawerContent.GetSafeHwnd() == ::GetFocus() );

		// Get List Item
		LVITEM   ListItem;
		ZeroMemory( &ListItem, sizeof(LVITEM) );
		ListItem.mask = LVIF_IMAGE | LVIF_STATE;
		ListItem.iItem = nItem;
		ListItem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
		m_ListDrawerContent.GetItem( &ListItem );

		// Draw ICON
		CRect IconRect;
		m_ListDrawerContent.GetItemRect( nItem, &IconRect, LVIR_ICON );
		UINT uFormat = ILD_TRANSPARENT;
		if ( (ListItem.state & LVIS_SELECTED) && bListHasFocus )
			uFormat |= ILD_FOCUS;
		CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
		m_DrawerImglist.Draw( pDC, ListItem.iImage, IconRect.TopLeft(), uFormat );

		CRect TextRect;
		m_ListDrawerContent.GetItemRect( nItem, TextRect, LVIR_LABEL);

		COLORREF background_color;
		if (ListItem.state & LVIS_SELECTED ) {
			if (bListHasFocus) {
				background_color = GetSysColor(COLOR_HIGHLIGHT);
				pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			} else {
				background_color = GetSysColor(COLOR_BTNFACE);
				pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
			}
		} else {
			background_color = GetSysColor( COLOR_WINDOW );
			pDC->SetTextColor( GetSysColor( COLOR_BTNTEXT ) );
		}

		// Draw Background
		// Background color
		pDC->FillSolidRect( TextRect, background_color );
		pDC->SetBkMode( TRANSPARENT );

		// Draw Text
		TextRect.left += 3;
		TextRect.right = TextRect.left + 200;
		TextRect.top++;
		CString strText = m_ListDrawerContent.GetItemText( nItem, 0 );
		pDC->DrawText( strText, TextRect, DT_VCENTER | DT_SINGLELINE );

		// Draw Focus
		//if ( bListHasFocus && ( ListItem.state & LVIS_FOCUSED ) ) {
		//	pDC->DrawFocusRect( TextRect );
		//}

		*pResult = CDRF_SKIPDEFAULT;
	}
}

void CHFSTWidgetsMFCDlg::InitToolBar()
{
	m_ToolBar.Create
	(
		this,
		WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS
	);
	m_ToolBar.LoadToolBar(IDR_TOOLBAR1);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	/*m_ToolBarImage.Create(32, 32, ILC_COLOR32 | ILC_MASK, 2, 2);
	m_ToolBarImage.SetBkColor(RGB(255, 255, 255));

	CPngImage* pPngImage = new CPngImage();
	pPngImage->Load(IDB_PNG_HEADER);
	m_ToolBarImage.Add(pPngImage, RGB(0, 0, 0));
	pPngImage->DeleteObject();

	pPngImage = new CPngImage();
	pPngImage->Load(IDB_PNG_CFG);
	m_ToolBarImage.Add(pPngImage, RGB(0, 0, 0));
	pPngImage->DeleteObject();

	pPngImage = new CPngImage();
	pPngImage->Load(IDB_PNG_INI);
	m_ToolBarImage.Add(pPngImage, RGB(0, 0, 0));
	pPngImage->DeleteObject();

	pPngImage = new CPngImage();
	pPngImage->Load(IDB_PNG_SAVE_INI);
	m_ToolBarImage.Add(pPngImage, RGB(0, 0, 0));
	pPngImage->DeleteObject();

	pPngImage = new CPngImage();
	pPngImage->Load(IDB_PNG_CATEGORY);
	m_ToolBarImage.Add(pPngImage, RGB(0, 0, 0));
	pPngImage->DeleteObject();

	pPngImage = new CPngImage();
	pPngImage->Load(IDB_PNG_SEARCH);
	m_ToolBarImage.Add(pPngImage, RGB(0, 0, 0));
	pPngImage->DeleteObject();*/

	//m_ToolBar.SetButtonText(0, _T("&Open .h"));
	//m_ToolBar.SetButtonText(1, _T("&Open .cfg"));
	//m_ToolBar.SetButtonText(2, _T("&Open .ini"));
	//m_ToolBar.SetButtonText(3, _T("&Save .ini"));
	//m_ToolBar.SetButtonText(4, _T("&Category"));
	//m_ToolBar.SetButtonText(5, _T("&Search"));
	//m_ToolBar.SetButtonText(6, _T("&Search"));
	//m_ToolBar.SetButtonText(7, _T("&Search"));

	//mToolBar.GetToolBarCtrl().SetImageList(&m_ToolBarImage);
	//m_ToolBar.GetToolBarCtrl().EnableButton(ID_BUTTON_H, TRUE);
	//m_ToolBar.GetToolBarCtrl().EnableButton(ID_BUTTON_CFG, FALSE);
	//m_ToolBar.GetToolBarCtrl().EnableButton(ID_BUTTON_INI, FALSE);
	//m_ToolBar.GetToolBarCtrl().EnableButton(ID_BUTTON_SAVE_INI, FALSE);
	//m_ToolBar.GetToolBarCtrl().EnableButton(IDB_PNG_CATEGORY, TRUE);
	//m_ToolBar.GetToolBarCtrl().EnableButton(ID_BUTTON_SEARCH, TRUE);
	m_ToolBar.ShowWindow(SW_SHOW);
	m_ToolBar.EnableToolTips(TRUE);
}

