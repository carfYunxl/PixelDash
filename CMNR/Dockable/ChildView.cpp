#include "pch.h"
#include "framework.h"
#include "CMNR.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
END_MESSAGE_MAP()

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH), nullptr);
	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this);

	CPen pen(PS_SOLID, 4, RGB(255,0,0));
	CPen* oldpen = dc.SelectObject(&pen);

	dc.MoveTo(0,0);
	dc.LineTo(100,100);

	dc.SelectObject(oldpen);
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_Btn.Create(_T("测试按钮"), WS_CHILD | WS_VISIBLE, CRect(500, 500, 800, 800), this, 8888);

	return 0;
}
