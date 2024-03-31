#include "pch.h"
#include "HF_mainfrm.h"
#include "HF_FileView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace HF
{
	HF_FileView::HF_FileView() noexcept {
	}

	HF_FileView::~HF_FileView() {
	}

	BEGIN_MESSAGE_MAP(HF_FileView, CDockablePane)
		ON_WM_CREATE()
		ON_WM_SIZE()
		ON_WM_CONTEXTMENU()
		ON_COMMAND(ID_EDIT_CUT, OnEditCut)
		ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
		ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
		ON_WM_PAINT()
		ON_WM_SETFOCUS()
	END_MESSAGE_MAP()

	int HF_FileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		if (CDockablePane::OnCreate(lpCreateStruct) == -1)
			return -1;

		CRect rectDummy;
		rectDummy.SetRectEmpty();

		const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

		if (!m_wndFileView.Create(dwViewStyle, rectDummy, this, 4))
		{
			TRACE0("未能创建文件视图\n");
			return -1;      // 未能创建
		}

		// 加载视图图像: 
		m_FileViewImages.Create(IDB_BITMAP_TREE_VIEW, 20, 0, RGB(255, 0, 255));
		m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

		m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, 134);
		m_wndToolBar.LoadToolBar(IDR_MAINFRAME, 0, 0, TRUE);

		OnChangeVisualStyle();

		m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

		m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

		m_wndToolBar.SetOwner(this);

		m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

		FillFileView();
		AdjustLayout();

		return 0;
	}

	void HF_FileView::OnSize(UINT nType, int cx, int cy)
	{
		CDockablePane::OnSize(nType, cx, cy);
		AdjustLayout();
	}

	void HF_FileView::FillFileView()
	{
		HTREEITEM hRoot = m_wndFileView.InsertItem(_T("测试选项"), 0, 0);
		m_wndFileView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

		HTREEITEM hSrc = m_wndFileView.InsertItem(_T("灵敏度"), 1, 1, hRoot);

		m_wndFileView.InsertItem(_T("直线"), 0, 1, hSrc);
		m_wndFileView.InsertItem(_T("米字型"), 1, 1, hSrc);
		m_wndFileView.InsertItem(_T("圆形"), 2, 1, hSrc);
		m_wndFileView.InsertItem(_T("口字形"), 3, 1, hSrc);
		m_wndFileView.InsertItem(_T("回字形"), 4, 1, hSrc);
		m_wndFileView.InsertItem(_T("螺旋"), 5, 1, hSrc);

		HTREEITEM hInc = m_wndFileView.InsertItem(_T("精准度"), 2, 2, hRoot);

		m_wndFileView.InsertItem(_T("直线"), 0, 2, hInc);
		m_wndFileView.InsertItem(_T("米字型"), 1, 2, hInc);
		m_wndFileView.InsertItem(_T("圆形"), 2, 2, hInc);
		m_wndFileView.InsertItem(_T("口字形"), 3, 2, hInc);
		m_wndFileView.InsertItem(_T("回字形"), 4, 2, hInc);
		m_wndFileView.InsertItem(_T("螺旋"), 5, 2, hInc);

		HTREEITEM hRes = m_wndFileView.InsertItem(_T("直线度"), 3, 3, hRoot);

		m_wndFileView.InsertItem(_T("直线"), 0, 2, hRes);
		m_wndFileView.InsertItem(_T("米字型"), 1, 2, hRes);
		m_wndFileView.InsertItem(_T("圆形"), 2, 2, hRes);
		m_wndFileView.InsertItem(_T("口字形"), 3, 2, hRes);
		m_wndFileView.InsertItem(_T("回字形"), 4, 2, hRes);
		m_wndFileView.InsertItem(_T("螺旋"), 5, 2, hRes);

		HTREEITEM hRes1 = m_wndFileView.InsertItem(_T("光滑度"), 4, 4, hRoot);

		m_wndFileView.InsertItem(_T("直线"), 0, 2, hRes1);
		m_wndFileView.InsertItem(_T("米字型"), 1, 2, hRes1);
		m_wndFileView.InsertItem(_T("圆形"), 2, 2, hRes1);
		m_wndFileView.InsertItem(_T("口字形"), 3, 2, hRes1);
		m_wndFileView.InsertItem(_T("回字形"), 4, 2, hRes1);
		m_wndFileView.InsertItem(_T("螺旋"), 5, 2, hRes1);

		m_wndFileView.Expand(hRoot, TVE_EXPAND);
		m_wndFileView.Expand(hSrc, TVE_EXPAND);
		m_wndFileView.Expand(hInc, TVE_EXPAND);
		m_wndFileView.Expand(hRes, TVE_EXPAND);
		m_wndFileView.Expand(hRes1, TVE_EXPAND);

		GetStockObject(NULL_BRUSH);
	}

	void HF_FileView::OnContextMenu(CWnd* pWnd, CPoint point)
	{
		CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndFileView;
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

	void HF_FileView::AdjustLayout()
	{
		if (GetSafeHwnd() == nullptr)
		{
			return;
		}

		CRect rectClient;
		GetClientRect(rectClient);

		int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

		m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
		m_wndFileView.SetWindowPos(
			nullptr,
			rectClient.left + 1,
			rectClient.top + cyTlb + 1,
			rectClient.Width() - 2,
			rectClient.Height() - cyTlb - 2,
			SWP_NOACTIVATE | SWP_NOZORDER
		);
	}

	void HF_FileView::OnProperties()
	{
		AfxMessageBox(_T("属性...."));
	}

	void HF_FileView::OnFileOpen()
	{
		// TODO: 在此处添加命令处理程序代码
	}

	void HF_FileView::OnFileOpenWith()
	{
		// TODO: 在此处添加命令处理程序代码
	}

	void HF_FileView::OnDummyCompile()
	{
		// TODO: 在此处添加命令处理程序代码
	}

	void HF_FileView::OnEditCut()
	{
		// TODO: 在此处添加命令处理程序代码
	}

	void HF_FileView::OnEditCopy()
	{
		// TODO: 在此处添加命令处理程序代码
	}

	void HF_FileView::OnEditClear()
	{
		// TODO: 在此处添加命令处理程序代码
	}

	void HF_FileView::OnPaint()
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		CRect rectTree;
		m_wndFileView.GetWindowRect(rectTree);
		ScreenToClient(rectTree);

		rectTree.InflateRect(1, 1);
		dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
	}

	void HF_FileView::OnSetFocus(CWnd* pOldWnd)
	{
		CDockablePane::OnSetFocus(pOldWnd);

		m_wndFileView.SetFocus();
	}

	void HF_FileView::OnChangeVisualStyle()
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

		m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);
	}


}
