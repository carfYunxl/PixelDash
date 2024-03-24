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
// CWorkspaceBar ��Ϣ�������

int CGraphicsView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// ������ͼ: 
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_wndGraphicsView.Create(dwViewStyle, rectDummy, this, 4))
	{
		TRACE0("δ�ܴ����ļ���ͼ\n");
		return -1;      // δ�ܴ���
	}

	// ������ͼͼ��: 
	m_FileViewImages.Create(1330, 16, 0, RGB(255, 0, 255));
	m_wndGraphicsView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, 1340);
	m_wndToolBar.LoadToolBar(IDR_MAINFRAME, 0, 0, TRUE /* ������*/);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// �������ͨ���˿ؼ�·�ɣ�������ͨ�������·��: 
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	// ����һЩ��̬����ͼ����(�˴�ֻ������������룬�����Ǹ��ӵ�����)
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
	HTREEITEM hRoot = m_wndGraphicsView.InsertItem(_T("ͼ�λ���"), 0, 0);
	m_wndGraphicsView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	HTREEITEM hSrc = m_wndGraphicsView.InsertItem(_T("ֱ��"), 0, 0, hRoot);

	m_wndGraphicsView.InsertItem(_T("Ĭ��"), 1, 1, hSrc);
	m_wndGraphicsView.InsertItem(_T("�û��Զ���"), 1, 1, hSrc);

	HTREEITEM hInc = m_wndGraphicsView.InsertItem(_T("����"), 0, 0, hRoot);

	m_wndGraphicsView.InsertItem(_T("Ĭ��"), 2, 2, hInc);
	m_wndGraphicsView.InsertItem(_T("�û��Զ���"), 2, 2, hInc);

	HTREEITEM hRes = m_wndGraphicsView.InsertItem(_T("������"), 0, 0, hRoot);

	m_wndGraphicsView.InsertItem(_T("Ĭ��"), 2, 2, hRes);
	m_wndGraphicsView.InsertItem(_T("�û��Զ���"), 2, 2, hRes);

	HTREEITEM hRes1 = m_wndGraphicsView.InsertItem(_T("������"), 0, 0, hRoot);

	m_wndGraphicsView.InsertItem(_T("Ĭ��"), 2, 2, hRes1);
	m_wndGraphicsView.InsertItem(_T("�û��Զ���"), 2, 2, hRes1);

	HTREEITEM hRes2 = m_wndGraphicsView.InsertItem(_T("������"), 0, 0, hRoot);

	m_wndGraphicsView.InsertItem(_T("Ĭ��"), 2, 2, hRes2);
	m_wndGraphicsView.InsertItem(_T("�û��Զ���"), 2, 2, hRes2);

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
		// ѡ���ѵ�������: 
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
	AfxMessageBox(_T("����...."));

}

void CGraphicsView::OnFileOpen()
{
	// TODO: �ڴ˴���������������
}

void CGraphicsView::OnFileOpenWith()
{
	// TODO: �ڴ˴���������������
}

void CGraphicsView::OnDummyCompile()
{
	// TODO: �ڴ˴���������������
}

void CGraphicsView::OnEditCut()
{
	// TODO: �ڴ˴���������������
}

void CGraphicsView::OnEditCopy()
{
	// TODO: �ڴ˴���������������
}

void CGraphicsView::OnEditClear()
{
	// TODO: �ڴ˴���������������
}

void CGraphicsView::OnPaint()
{
	CPaintDC dc(this); // ���ڻ��Ƶ��豸������

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
	//m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* ����*/);

	//m_FileViewImages.DeleteImageList();

	//UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;

	/*CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("�޷�����λͼ: %x\n"), uiBmpId);
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


