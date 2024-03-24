#include "pch.h"
#include "framework.h"
#include "mainfrm.h"
#include "GraphicsView.h"
#include "Resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileView

CGraphicsView::CGraphicsView() noexcept
{
}

CGraphicsView::~CGraphicsView()
{
}

BEGIN_MESSAGE_MAP(CGraphicsView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 消息处理程序

int CGraphicsView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建视图: 
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_wndGraphicsView.Create(dwViewStyle, rectDummy, this, 4))
	{
		TRACE0("未能创建文件视图\n");
		return -1;      // 未能创建
	}

	// 加载视图图像: 
	m_FileViewImages.Create(1330, 16, 0, RGB(255, 0, 255));
	m_wndGraphicsView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, 1340);
	m_wndToolBar.LoadToolBar(IDR_MAINFRAME, 0, 0, TRUE /* 已锁定*/);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// 所有命令将通过此控件路由，而不是通过主框架路由: 
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	// 填入一些静态树视图数据(此处只需填入虚拟代码，而不是复杂的数据)
	FillGraphicsView();
	AdjustLayout();

	return 0;
}

void CGraphicsView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CGraphicsView::FillGraphicsView()
{
	HTREEITEM hRoot = m_wndGraphicsView.InsertItem(_T("图形绘制"), 0, 0);
	m_wndGraphicsView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	HTREEITEM hSrc = m_wndGraphicsView.InsertItem(_T("直线"), 0, 0, hRoot);

	m_wndGraphicsView.InsertItem(_T("默认"), 1, 1, hSrc);
	m_wndGraphicsView.InsertItem(_T("用户自定义"), 1, 1, hSrc);

	HTREEITEM hInc = m_wndGraphicsView.InsertItem(_T("曲线"), 0, 0, hRoot);

	m_wndGraphicsView.InsertItem(_T("默认"), 2, 2, hInc);
	m_wndGraphicsView.InsertItem(_T("用户自定义"), 2, 2, hInc);

	HTREEITEM hRes = m_wndGraphicsView.InsertItem(_T("米字形"), 0, 0, hRoot);

	m_wndGraphicsView.InsertItem(_T("默认"), 2, 2, hRes);
	m_wndGraphicsView.InsertItem(_T("用户自定义"), 2, 2, hRes);

	HTREEITEM hRes1 = m_wndGraphicsView.InsertItem(_T("口字形"), 0, 0, hRoot);

	m_wndGraphicsView.InsertItem(_T("默认"), 2, 2, hRes1);
	m_wndGraphicsView.InsertItem(_T("用户自定义"), 2, 2, hRes1);

	HTREEITEM hRes2 = m_wndGraphicsView.InsertItem(_T("回字形"), 0, 0, hRoot);

	m_wndGraphicsView.InsertItem(_T("默认"), 2, 2, hRes2);
	m_wndGraphicsView.InsertItem(_T("用户自定义"), 2, 2, hRes2);

	m_wndGraphicsView.Expand(hRoot, TVE_EXPAND);
	m_wndGraphicsView.Expand(hSrc, TVE_EXPAND);
	m_wndGraphicsView.Expand(hInc, TVE_EXPAND);
	m_wndGraphicsView.Expand(hRes, TVE_EXPAND);
	m_wndGraphicsView.Expand(hRes1, TVE_EXPAND);
	m_wndGraphicsView.Expand(hRes2, TVE_EXPAND);
}

void CGraphicsView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndGraphicsView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// 选择已单击的项: 
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != nullptr)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	//theApp.GetContextMenuManager()->ShowPopupMenu(134, point.x, point.y, this, TRUE);
}

void CGraphicsView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndGraphicsView.SetWindowPos(nullptr, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CGraphicsView::OnProperties()
{
	AfxMessageBox(_T("属性...."));

}

void CGraphicsView::OnFileOpen()
{
	// TODO: 在此处添加命令处理程序代码
}

void CGraphicsView::OnFileOpenWith()
{
	// TODO: 在此处添加命令处理程序代码
}

void CGraphicsView::OnDummyCompile()
{
	// TODO: 在此处添加命令处理程序代码
}

void CGraphicsView::OnEditCut()
{
	// TODO: 在此处添加命令处理程序代码
}

void CGraphicsView::OnEditCopy()
{
	// TODO: 在此处添加命令处理程序代码
}

void CGraphicsView::OnEditClear()
{
	// TODO: 在此处添加命令处理程序代码
}

void CGraphicsView::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rectTree;
	m_wndGraphicsView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CGraphicsView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndGraphicsView.SetFocus();
}

void CGraphicsView::OnChangeVisualStyle()
{
	//m_wndToolBar.CleanUpLockedImages();
	//m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* 锁定*/);

	//m_FileViewImages.DeleteImageList();

	//UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;

	/*CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("无法加载位图: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}*/

	/*BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_FileViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_FileViewImages.Add(&bmp, RGB(255, 0, 255));

	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);*/
}


