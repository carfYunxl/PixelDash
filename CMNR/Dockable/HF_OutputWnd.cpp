#include "pch.h"
#include "HF_OutputWnd.h"
#include "HF_MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HF_OutputWnd::HF_OutputWnd() noexcept
{
}

HF_OutputWnd::~HF_OutputWnd()
{
	DeleteObject(m_Font);
}

BEGIN_MESSAGE_MAP(HF_OutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int HF_OutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
	{
		return -1;
	}

	const DWORD dwStyle = LBS_NOTIFY | LBS_MULTIPLESEL |
		LBS_OWNERDRAWFIXED | LBS_HASSTRINGS |
		LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_HSCROLL |
		WS_TABSTOP;

	if (!m_wndOutputBuild.Create(dwStyle, rectDummy, &m_wndTabs, 2) ||
		!m_wndOutputDebug.Create(dwStyle, rectDummy, &m_wndTabs, 3) ||
		!m_wndOutputFind.Create(dwStyle, rectDummy, &m_wndTabs, 4))
	{
		return -1;
	}

	UpdateFonts();

	CString strTabName;
	BOOL bNameValid;

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

	m_Font.CreatePointFont(120, _T("Consolas"));
	m_wndOutputBuild.SetFont(&m_Font);
	m_wndOutputDebug.SetFont(&m_Font);
	m_wndOutputFind.SetFont(&m_Font);

	return 0;
}

void HF_OutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	m_wndTabs.SetWindowPos(nullptr, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void HF_OutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i++)
	{
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, (int)dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}

void HF_OutputWnd::UpdateFonts()
{
	m_wndOutputBuild.SetFont(&afxGlobalData.fontRegular);
	m_wndOutputDebug.SetFont(&afxGlobalData.fontRegular);
	m_wndOutputFind.SetFont(&afxGlobalData.fontRegular);
}

void HF_OutputWnd::AddString(LogType type, const CString& message)
{
	switch (type)
	{
	case LogType::INFO:
		m_wndOutputBuild.AddLine(HF_ListBox::Color::Gray, HF_ListBox::Color::White, _T("[INFO]\t") + message);
		break;
	case LogType::WARN:
		m_wndOutputBuild.AddLine(HF_ListBox::Color::Purple, HF_ListBox::Color::White, _T("[WARN]\t") + message);
		break;
	case LogType::ERR:
		m_wndOutputBuild.AddLine(HF_ListBox::Color::Red, HF_ListBox::Color::White, _T("[EROR]\t") + message);
		break;
	default:
		m_wndOutputBuild.AddLine(HF_ListBox::Color::Gray, HF_ListBox::Color::White, _T("[UnKnown]") + message);
		break;
	}
}

