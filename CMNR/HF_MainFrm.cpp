#include "pch.h"
#include "CMNR.h"
#include "HF_MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(HF_MainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(HF_MainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &HF_MainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &HF_MainFrame::OnToolbarCreateNew)
	ON_COMMAND(ID_BUTTON_IC, &HF_MainFrame::OnButtonIc)
	ON_COMMAND(ID_BUTTON_CON, &HF_MainFrame::OnButtonCon)
	ON_COMMAND(ID_BUTTON_DISCON, &HF_MainFrame::OnButtonDiscon)
	ON_COMMAND(ID_BUTTON_TEST1, &HF_MainFrame::OnButtonTest1)
	ON_COMMAND(ID_BUTTON_TEST2, &HF_MainFrame::OnButtonTest2)
	ON_COMMAND(ID_BUTTON_TEST3, &HF_MainFrame::OnButtonTest3)
	ON_WM_PAINT()
	ON_COMMAND(ID_VIEW_TEST_ITEM, &HF_MainFrame::OnViewTestItem)
	ON_COMMAND(ID_VIEW_PROPERTY, &HF_MainFrame::OnViewProperty)
	ON_COMMAND(ID_VIEW_MACHINE, &HF_MainFrame::OnViewMachine)
	ON_COMMAND(ID_VIEW_OUTPUT, &HF_MainFrame::OnViewOutput)

	ON_COMMAND_RANGE(IDM_COLOR_RED, IDM_COLOR_YELLOW, &HF_MainFrame::OnColor)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_COLOR_RED, IDM_COLOR_YELLOW, &HF_MainFrame::OnUpdateColorUI)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_DEVICECHANGE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,
	IDS_PROGRESS,
	//ID_INDICATOR_CAPS,
	//ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

HF_MainFrame::HF_MainFrame()noexcept 
	: m_nCurrentColor()
	, m_pConnector(std::make_unique<HFST::Connector>())
{
}

HF_MainFrame::~HF_MainFrame()
{
	DeleteObject(m_Font);
}
int HF_MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	//EnableLoadDockState(FALSE);

	if (!m_wndMenuBar.Create(this)) {
		return -1;
	}
	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	CMFCPopupMenu::SetForceMenuFocus(FALSE);
	if (!m_wndView.Create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, nullptr))
	{
		return -1;
	}

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		//!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
		!m_wndToolBar.LoadToolBar(IDR_TOOLBAR_IC, 0, 0, TRUE))
	{
		return -1;
	}
	m_wndToolBar.SetWindowTextW(_T("图形"));
	m_wndToolBar.SetPermament(TRUE);

	m_NewToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_TOOLBAR_TEST);
	m_NewToolBar.LoadToolBar(IDR_TOOLBAR_TEST, 0, 0, TRUE /* 已锁定*/);
	m_NewToolBar.SetPermament(TRUE);
	m_NewToolBar.SetShowTooltips(TRUE);

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	// 允许用户定义的工具栏操作: 
	InitUserToolbars(nullptr, USR_TOOLBAR_BG_ID, USR_TOOLBAR_ED_ID);

	if (!m_wndStatusBar.Create(this))
	{
		return -1;
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));

	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_NewToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	DockPane(&m_NewToolBar);

	if (!m_wndProperty.Create(_T("属性"), this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		return FALSE;
	}

	if (!m_wndTestView.Create(_T("图形视图"), this, CRect(0, 0, 200, 200), TRUE, 133, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		return FALSE;
	}

	if (!m_wndOutput.Create(_T("输出"), this, CRect(0, 0, 100, 100), TRUE, 1350, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		return FALSE;
	}
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	m_wndTestView.EnableDocking(CBRS_ALIGN_LEFT);
	//m_wndTestView.SetControlBarStyle(~AFX_CBRS_CLOSE);
	//m_wndTestView.SetControlBarStyle(AFX_CBRS_RESIZE);
	m_wndProperty.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndProperty.SetControlBarStyle(~AFX_CBRS_CLOSE);
	//m_wndProperty.SetControlBarStyle(AFX_CBRS_RESIZE);
	DockPane(&m_wndOutput);
	DockPane(&m_wndProperty);
	DockPane(&m_wndTestView);

	CDockingManager::SetDockingMode(DT_SMART);
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	//EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);
	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == nullptr)
	{
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}

	CList<UINT, UINT> lstBasicCommands;

	lstBasicCommands.AddTail(ID_APP_EXIT);
	lstBasicCommands.AddTail(ID_EDIT_CUT);
	lstBasicCommands.AddTail(ID_EDIT_PASTE);
	lstBasicCommands.AddTail(ID_EDIT_UNDO);
	lstBasicCommands.AddTail(ID_APP_ABOUT);
	lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);

	CMFCToolBar::SetBasicCommands(lstBasicCommands);

	m_wndStatusBar.SetPaneInfo(1, IDS_PROGRESS, SBPS_NORMAL, 400);
	m_wndStatusBar.EnablePaneProgressBar(1, 100, TRUE, RGB(0,255,0), -1, RGB(0, 0, 0));
	m_wndStatusBar.SetPaneProgress(1, 50);
	m_wndStatusBar.SetPaneBackgroundColor(0, RGB(230, 230, 230));

	m_Font.CreatePointFont(120, _T("微软雅黑"));
	LOGFONT font;
	memset(&font, 0, sizeof(LOGFONT));
	m_Font.GetLogFont(&font);
	m_wndMenuBar.SetMenuFont(&font);

	CMenu* pMenu = CMenu::FromHandle(m_wndMenuBar.GetDefaultMenu());
	CMenu* pSubMenu = pMenu->GetSubMenu(4);
	for (UINT i = IDM_COLOR_RED; i <= IDM_COLOR_YELLOW; ++i)
	{
		pSubMenu->ModifyMenuW(IDM_COLOR_RED + i, MF_OWNERDRAW, IDM_COLOR_RED + i);
	}
	m_wndStatusBar.SetPaneBackgroundColor(2, RGB(230, 230, 230));
	m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_CAPS, SBPS_NORMAL, 200);

	auto error = m_pConnector->Connect(2.8,1.8);
	if ( error.value() < 0 )
	{
		Log(LogType::ERR, error.message().c_str());

		//m_wndStatusBar.SetPaneIcon(4, LoadIcon(theApp.m_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME)), TRUE);
		m_wndStatusBar.SetPaneIcon(2, LoadBitmap(theApp.m_hInstance, MAKEINTRESOURCE(IDB_BITMAP_CONNECT)), TRUE);
		m_wndStatusBar.SetPaneText(2, _T("连接失败"), TRUE);
		m_wndStatusBar.SetPaneTextColor(2, RGB(0, 0, 0), TRUE);
	}
	else
	{
		Log(LogType::INFO, _T("IC Connected!"));

		//m_wndStatusBar.SetPaneIcon(4, LoadIcon(theApp.m_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME)), TRUE);
		m_wndStatusBar.SetPaneIcon(2, LoadBitmap(theApp.m_hInstance, MAKEINTRESOURCE(IDB_BITMAP_DIS_CONNECT)), TRUE);
		m_wndStatusBar.SetPaneText(2, _T("已连接"), TRUE);
		m_wndStatusBar.SetPaneTextColor(2, RGB(0, 255, 0), TRUE);

		RecalcLayout();
	}

	
	auto ic_info = m_pConnector->IC_GetInfo();

	m_pRawReader = HFST::CreateRawReader(ic_info);

 	std::thread th([&]() {
		while (1)
		{
			m_ChannelRaw.vecRaw.clear();
			strShow.Empty();
			int res = m_pRawReader->ReadChannelRaw(m_ChannelRaw);
			if (res < 0)
				continue;

			strShow.Format(_T("%02x %02x %02x "), m_ChannelRaw.nDataType, m_ChannelRaw.nChannelIdx, static_cast<int>(m_ChannelRaw.Type));

			for (int i = 0; i < m_ChannelRaw.vecRaw.size(); ++i)
			{
				strShow.AppendFormat(_T("%02x "), m_ChannelRaw.vecRaw.at(i));
			}

			this->Log(LogType::INFO, strShow);

			Sleep(100);
		}
	});

	th.detach();

	return 0;
}

BOOL HF_MainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	//return m_wndSplitter.Create(this,
	//	2, 2,               // TODO: 调整行数和列数
	//	CSize(10, 10),      // TODO: 调整最小窗格大小
	//	pContext);
	return TRUE;
}

BOOL HF_MainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWndEx::PreCreateWindow(cs))
		return FALSE;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		::LoadCursor(nullptr, IDC_ARROW), nullptr, LoadIcon(NULL, IDI_ERROR));
	return TRUE;
}

#ifdef _DEBUG
void HF_MainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void HF_MainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG

void HF_MainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndView.SetFocus();
}

BOOL HF_MainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;
	return CFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void HF_MainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* 扫描菜单*/);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT HF_MainFrame::OnToolbarCreateNew(WPARAM wp, LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp, lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

BOOL HF_MainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < MAX_USR_TOOBARS; i++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != nullptr)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
}

void HF_MainFrame::OnButtonIc()
{
}

void HF_MainFrame::OnButtonCon()
{
	// TODO: 在此添加命令处理程序代码
}

void HF_MainFrame::OnButtonDiscon()
{
	// TODO: 在此添加命令处理程序代码
}

void HF_MainFrame::OnButtonTest1()
{
	// TODO: 在此添加命令处理程序代码
}

void HF_MainFrame::OnButtonTest2()
{
	// TODO: 在此添加命令处理程序代码
}

void HF_MainFrame::OnButtonTest3()
{
	// TODO: 在此添加命令处理程序代码
}

void HF_MainFrame::OnPaint()
{
	CPaintDC dc(this);
}

void HF_MainFrame::Log(LogType type, const CString& message) 
{
	m_wndOutput.AddString(type, message);
}

void HF_MainFrame::OnViewTestItem()
{
	BOOL bShow = !m_wndTestView.IsWindowVisible();
	m_wndTestView.ShowPane(bShow, FALSE, bShow);
}

void HF_MainFrame::OnViewProperty()
{
	// TODO: 在此添加命令处理程序代码
}

void HF_MainFrame::OnViewMachine()
{
	// TODO: 在此添加命令处理程序代码
}

void HF_MainFrame::OnViewOutput()
{
	// TODO: 在此添加命令处理程序代码
}

void HF_MainFrame::OnColor(UINT nID)
{
	m_nCurrentColor = nID - IDM_COLOR_RED;
}

void HF_MainFrame::OnUpdateColorUI(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio( m_nCurrentColor == (pCmdUI->m_nID - IDM_COLOR_RED) );
}

void HF_MainFrame::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	int width = ::GetSystemMetrics(SM_CYMENU);
	lpMeasureItemStruct->itemHeight = width * 4;
	lpMeasureItemStruct->itemHeight = width;
	CFrameWndEx::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

void HF_MainFrame::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	BITMAP bm;
	CBitmap bitmap;
	bitmap.LoadOEMBitmap(IDB_BITMAP_TREE_VIEW);
	bitmap.GetObject(sizeof(bm), &bm);
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);
	CBrush* pBrush = new CBrush(::GetSysColor((lpDrawItemStruct->itemState &
		ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_MENU));
	dc.FrameRect(&(lpDrawItemStruct->rcItem), pBrush);
	delete pBrush;
	if (lpDrawItemStruct->itemState & ODS_CHECKED) {
		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CBitmap* pOldBitmap = dcMem.SelectObject(&bitmap);

	CFrameWndEx::OnDrawItem(nIDCtl, lpDrawItemStruct);
	dc.BitBlt(lpDrawItemStruct->rcItem.left + 4, lpDrawItemStruct->rcItem.top +
		(((lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top) -
			bm.bmHeight) / 2), bm.bmWidth, bm.bmHeight, &dcMem,
		0, 0, SRCCOPY);
	dcMem.SelectObject(pOldBitmap);
	}
	UINT itemID = lpDrawItemStruct->itemID & 0xFFFF; // Fix for Win95 bug. 
	pBrush = new CBrush(m_wndView.m_clrColors[itemID -
		IDM_COLOR_RED]);
	CRect rect = lpDrawItemStruct->rcItem;
	rect.DeflateRect(6, 4);
	rect.left += bm.bmWidth;
	dc.FillRect(rect, pBrush);
	delete pBrush;
	dc.Detach();
}

BOOL HF_MainFrame::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	//int ret;
	CString msg;
	DEV_BROADCAST_DEVICEINTERFACE* pDevBCDIF;

	switch (nEventType)
	{
	case DBT_DEVICEARRIVAL:
		
		break;
	case DBT_DEVICEREMOVECOMPLETE:
		pDevBCDIF = (DEV_BROADCAST_DEVICEINTERFACE*)dwData;
		//if (pDevBCDIF->dbcc_classguid == GUID_DEVINTERFACE_ATL)
		//{
		//	
		//}
		break;
	default:
		break;
	}

	return CFrameWndEx::OnDeviceChange(nEventType, dwData);
}
