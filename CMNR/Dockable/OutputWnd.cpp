
#include "pch.h"
#include "framework.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "Clipboard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputWnd::COutputWnd() noexcept
{
}

COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建选项卡窗口: 
	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
	{
		TRACE0("未能创建输出选项卡窗口\n");
		return -1;      // 未能创建
	}

	// 创建输出窗格: 
	//const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;
	const DWORD dwStyle = LBS_NOTIFY | LBS_MULTIPLESEL |
		LBS_OWNERDRAWFIXED | LBS_HASSTRINGS |
		LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_HSCROLL |
		WS_TABSTOP;

	if (!m_wndOutputBuild.Create(dwStyle, rectDummy, &m_wndTabs, 2) ||
		!m_wndOutputDebug.Create(dwStyle, rectDummy, &m_wndTabs, 3) ||
		!m_wndOutputFind.Create(dwStyle, rectDummy, &m_wndTabs, 4))
	{
		TRACE0("未能创建输出窗口\n");
		return -1;      // 未能创建
	}

	UpdateFonts();

	CString strTabName;
	BOOL bNameValid;

	// 将列表窗口附加到选项卡: 
	bNameValid = strTabName.LoadString(IDS_BUILD_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputBuild, strTabName, (UINT)0);
	bNameValid = strTabName.LoadString(IDS_DEBUG_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputDebug, strTabName, (UINT)1);
	bNameValid = strTabName.LoadString(IDS_FIND_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputFind, strTabName, (UINT)2);

	// 使用一些虚拟文本填写输出选项卡(无需复杂数据)
	m_wndOutputBuild.EnableColor(TRUE);
	m_wndOutputDebug.EnableColor(TRUE);
	m_wndOutputFind.EnableColor(TRUE);

	m_wndOutputBuild.SetContextMenuId(IDR_XLISTBOX);
	m_wndOutputDebug.SetContextMenuId(IDR_XLISTBOX);
	m_wndOutputFind.SetContextMenuId(IDR_XLISTBOX);

	FillBuildWindow();
	FillDebugWindow();
	FillFindWindow();

	m_Font.CreatePointFont(130, _T("微软雅黑"));
	m_wndOutputBuild.SetFont(&m_Font);
	m_wndOutputDebug.SetFont(&m_Font);
	m_wndOutputFind.SetFont(&m_Font);

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// 选项卡控件应覆盖整个工作区: 
	m_wndTabs.SetWindowPos (nullptr, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i ++)
	{
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, (int)dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}

void COutputWnd::FillBuildWindow()
{
	m_wndOutputBuild.AddString(_T("生成输出正显示在此处。"));
	m_wndOutputBuild.AddString(_T("输出正显示在列表视图的行中"));
	m_wndOutputBuild.AddString(_T("但您可以根据需要更改其显示方式..."));

	m_wndOutputBuild.AddLine(CXListBox::Red, CXListBox::Color::White, _T("这是一段测试文字，检查颜色是否能正确设置"));

	m_wndOutputBuild.Printf(CXListBox::Blue, CXListBox::White, 0, _T("XHtmlTreeTest v1.6"));
}

void COutputWnd::FillDebugWindow()
{
	m_wndOutputDebug.AddString(_T("调试输出正显示在此处。"));
	m_wndOutputDebug.AddString(_T("输出正显示在列表视图的行中"));
	m_wndOutputDebug.AddString(_T("但您可以根据需要更改其显示方式..."));

	m_wndOutputDebug.AddLine(CXListBox::Red, CXListBox::Color::White, _T("这是一段测试文字，检查颜色是否能正确设置"));

	m_wndOutputDebug.Printf(CXListBox::Blue, CXListBox::White, 0, _T("XHtmlTreeTest v1.6"));
}

void COutputWnd::FillFindWindow()
{
	m_wndOutputFind.AddString(_T("查找输出正显示在此处。"));
	m_wndOutputFind.AddString(_T("输出正显示在列表视图的行中"));
	m_wndOutputFind.AddString(_T("但您可以根据需要更改其显示方式..."));

	m_wndOutputFind.AddLine(CXListBox::Red, CXListBox::Color::White, _T("这是一段测试文字，检查颜色是否能正确设置"));

	m_wndOutputFind.Printf(CXListBox::Blue, CXListBox::White, 0, _T("XHtmlTreeTest v1.6"));
}

void COutputWnd::UpdateFonts()
{
	m_wndOutputBuild.SetFont(&afxGlobalData.fontRegular);
	m_wndOutputDebug.SetFont(&afxGlobalData.fontRegular);
	m_wndOutputFind.SetFont(&afxGlobalData.fontRegular);
}

#define ID_VIEW_OUTPUTWND 149
#define IDR_OUTPUT_POPUP			182

