#include "pch.h"
#include "HF_mainfrm.h"
#include "HF_TestView.h"
#include "HF_PropertiesWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

HF_TestView::HF_TestView() noexcept {
}

HF_TestView::~HF_TestView() {
	DeleteObject(&m_Font);
}

BEGIN_MESSAGE_MAP(HF_TestView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TEST_VIEW_TREE, &HF_TestView::OnTvnSelchangedTree)
END_MESSAGE_MAP()

int HF_TestView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_wndTestView.Create(dwViewStyle, rectDummy, this, IDC_TEST_VIEW_TREE))
	{
		TRACE0("未能创建文件视图\n");
		return -1;      // 未能创建
	}

	// 加载视图图像: 
	m_FileViewImages.Create(IDB_BITMAP_TREE_VIEW, 20, 0, RGB(255, 0, 255));
	m_wndTestView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, 134);
	m_wndToolBar.LoadToolBar(IDR_MAINFRAME, 0, 0, TRUE);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	FillFileView();
	AdjustLayout();

	m_Font.CreatePointFont(100, _T("微软雅黑"));
	m_wndTestView.SetFont(&m_Font);
	return 0;
}

void HF_TestView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void HF_TestView::FillFileView()
{
	HTREEITEM hRoot = m_wndTestView.InsertItem(_T("图形"), 0, 0);
	m_wndTestView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	HTREEITEM hBasic = m_wndTestView.InsertItem(_T("基本图形"), 1, 1, hRoot);
	m_wndTestView.InsertItem(	_T("直线"),		1, 1, hBasic	);
	m_wndTestView.InsertItem(	_T("矩形"),		2, 2, hBasic	);
	m_wndTestView.InsertItem(	_T("三角形"),	3, 3, hBasic	);
	m_wndTestView.InsertItem(	_T("圆形"),		4, 4, hBasic	);
	m_wndTestView.InsertItem(	_T("椭圆"),		5, 5, hBasic	);

	HTREEITEM hUserDefine = m_wndTestView.InsertItem(_T("自定义图形"), 2, 2, hRoot);
	m_wndTestView.InsertItem(	_T("米字形"),		1, 1, hUserDefine	);
	m_wndTestView.InsertItem(	_T("口字形"),		2, 2, hUserDefine	);
	m_wndTestView.InsertItem(	_T("回字形"),		3, 3, hUserDefine	);
	m_wndTestView.InsertItem(	_T("螺旋形"),		4, 4, hUserDefine	);
	m_wndTestView.InsertItem(	_T("X字形"),			5, 5, hUserDefine	);

	m_wndTestView.Expand( hRoot, TVE_EXPAND );
	m_wndTestView.Expand( hBasic, TVE_EXPAND );
	m_wndTestView.Expand( hUserDefine, TVE_EXPAND );

	GetStockObject(NULL_BRUSH);
}

void HF_TestView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndTestView;
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
}

void HF_TestView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndTestView.SetWindowPos(
		nullptr,
		rectClient.left + 1,
		rectClient.top + cyTlb + 1,
		rectClient.Width() - 2,
		rectClient.Height() - cyTlb - 2,
		SWP_NOACTIVATE | SWP_NOZORDER
	);
}

void HF_TestView::OnProperties()
{
	AfxMessageBox(_T("属性...."));
}

void HF_TestView::OnFileOpen()
{
	// TODO: 在此处添加命令处理程序代码
}

void HF_TestView::OnFileOpenWith()
{
	// TODO: 在此处添加命令处理程序代码
}

void HF_TestView::OnDummyCompile()
{
	// TODO: 在此处添加命令处理程序代码
}

void HF_TestView::OnEditCut()
{
	// TODO: 在此处添加命令处理程序代码
}

void HF_TestView::OnEditCopy()
{
	// TODO: 在此处添加命令处理程序代码
}

void HF_TestView::OnEditClear()
{
	// TODO: 在此处添加命令处理程序代码
}

void HF_TestView::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rectTree;
	m_wndTestView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void HF_TestView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndTestView.SetFocus();
}

void HF_TestView::OnChangeVisualStyle()
{
	//m_wndToolBar.CleanUpLockedImages();
	//m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* 锁定*/);

	m_FileViewImages.DeleteImageList();

	//UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;
	CBitmap bmp;
	if (!bmp.LoadBitmap(IDB_BITMAP_TREE_VIEW))
	{
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	/*nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;*/
	nFlags |= ILC_COLOR24;

	//m_FileViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_FileViewImages.Create(20, bmpObj.bmHeight, nFlags, 0, 0);
	m_FileViewImages.Add(&bmp, RGB(255, 0, 255));

	m_wndTestView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);
}

void HF_TestView::OnTvnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if ( m_pWndProperty && m_pWndProperty->m_hWnd != NULL )
	{
		CString strSelectItem = m_wndTestView.GetItemText( pNMTreeView->itemNew.hItem );

		DRAW_TYPE type = GetIndexByTitle(strSelectItem);
		if ( type == DRAW_TYPE::NONE )
			return;

		m_pMainView->SetDrawType( type );

		m_pWndProperty->GetPropertyCtrl().RemoveAll();

		switch ( type )
		{
			case DRAW_TYPE::LINE:
				m_pWndProperty->AddLineProperty();
				break;
		}
	}
	
	*pResult = 0;
}

DRAW_TYPE HF_TestView::GetIndexByTitle(const CString& title)
{
	auto item = std::find_if(m_sShapeMap.cbegin(), m_sShapeMap.cend(), [&](const std::pair<const CString, DRAW_TYPE>& pair) {
			return title == pair.first;
		});

	if ( item != m_sShapeMap.cend() )
		return (*item).second;

	return DRAW_TYPE::NONE;
}
