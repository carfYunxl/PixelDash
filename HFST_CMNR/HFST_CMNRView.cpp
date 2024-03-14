#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "HFST_CMNR.h"
#endif

#include "HFST_CMNRDoc.h"
#include "HFST_CMNRView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHFSTCMNRView
IMPLEMENT_DYNCREATE(CHFSTCMNRView, CView)

BEGIN_MESSAGE_MAP(CHFSTCMNRView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CHFSTCMNRView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CHFSTCMNRView 构造/析构
CHFSTCMNRView::CHFSTCMNRView() noexcept
{
	// TODO: 在此处添加构造代码
}

CHFSTCMNRView::~CHFSTCMNRView()
{
}

BOOL CHFSTCMNRView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式
	return CView::PreCreateWindow(cs);
}

// CHFSTCMNRView 绘图
void CHFSTCMNRView::OnDraw(CDC* /*pDC*/)
{
	CHFSTCMNRDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
}


// CHFSTCMNRView 打印
void CHFSTCMNRView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CHFSTCMNRView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CHFSTCMNRView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CHFSTCMNRView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CHFSTCMNRView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CHFSTCMNRView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

// CHFSTCMNRView 诊断
#ifdef _DEBUG
void CHFSTCMNRView::AssertValid() const
{
	CView::AssertValid();
}

void CHFSTCMNRView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CHFSTCMNRDoc* CHFSTCMNRView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHFSTCMNRDoc)));
	return (CHFSTCMNRDoc*)m_pDocument;
}
#endif //_DEBUG
