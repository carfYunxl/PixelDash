#include "pch.h"
#include "HF_PropertiesWnd.h"
#include "HF_MainFrm.h"
#include "CMNR.h"
#include "HF_ChildView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

HF_PropertiesWnd::HF_PropertiesWnd() noexcept {
}

HF_PropertiesWnd::~HF_PropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(HF_PropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, &HF_PropertiesWnd::OnWmPropertyChanged)
END_MESSAGE_MAP()

void HF_PropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(nullptr, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);

	RecalcLayout();
}

int HF_PropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建组合: 
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("未能创建属性网格\n");
		return -1;      // 未能创建
	}

	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_MAINFRAME);
	m_wndToolBar.LoadToolBar(IDR_MAINFRAME, 0, 0, TRUE /* 已锁定*/);
	//m_wndToolBar.CleanUpLockedImages();
	//m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* 锁定*/);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// 所有命令将通过此控件路由，而不是通过主框架路由: 
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void HF_PropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void HF_PropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void HF_PropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void HF_PropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void HF_PropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void HF_PropertiesWnd::OnProperties1()
{
	// TODO: 在此处添加命令处理程序代码
}

void HF_PropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO: 在此处添加命令更新 UI 处理程序代码
}

void HF_PropertiesWnd::OnProperties2()
{
	// TODO: 在此处添加命令处理程序代码
}

void HF_PropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	// TODO: 在此处添加命令更新 UI 处理程序代码
}

void HF_PropertiesWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();
	m_wndPropList.SetDescriptionRows(1);

	AddDefaultProperty();
	
}

void HF_PropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void HF_PropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void HF_PropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
	//m_wndObjectCombo.SetFont(&m_fntPropList);
}

void HF_PropertiesWnd::AddDefaultProperty()
{
#if 0
	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(_T("外观"));

	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("三维外观"), (_variant_t)false, _T("指定窗口的字体不使用粗体，并且控件将使用三维边框")));

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(_T("边框"), _T("对话框外框"), _T("其中之一: “无”、“细”、“可调整大小”或“对话框外框”"));
	pProp->AddOption(_T("无"));
	pProp->AddOption(_T("细"));
	pProp->AddOption(_T("可调整大小"));
	pProp->AddOption(_T("对话框外框"));
	pProp->AllowEdit(FALSE);

	pGroup1->AddSubItem(pProp);
	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("标题"), (_variant_t)_T("关于"), _T("指定窗口标题栏中显示的文本")));

	m_wndPropList.AddProperty(pGroup1);

	CMFCPropertyGridProperty* pSize = new CMFCPropertyGridProperty(_T("窗口大小"), 0, TRUE);

	pProp = new CMFCPropertyGridProperty(_T("高度"), (_variant_t)250l, _T("指定窗口的高度"));
	pProp->EnableSpinControl(TRUE, 50, 300);
	pSize->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty(_T("宽度"), (_variant_t)150l, _T("指定窗口的宽度"));
	pProp->EnableSpinControl(TRUE, 50, 200);
	pSize->AddSubItem(pProp);

	m_wndPropList.AddProperty(pSize);

	CMFCPropertyGridProperty* pGroup2 = new CMFCPropertyGridProperty(_T("字体"));

	LOGFONT lf;
	CFont* font = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	font->GetLogFont(&lf);

	_tcscpy_s(lf.lfFaceName, _T("宋体, Arial"));

	pGroup2->AddSubItem(new CMFCPropertyGridFontProperty(_T("字体"), lf, CF_EFFECTS | CF_SCREENFONTS, _T("指定窗口的默认字体")));
	pGroup2->AddSubItem(new CMFCPropertyGridProperty(_T("使用系统字体"), (_variant_t)true, _T("指定窗口使用“MS Shell Dlg”字体")));

	m_wndPropList.AddProperty(pGroup2);

	CMFCPropertyGridProperty* pGroup3 = new CMFCPropertyGridProperty(_T("杂项"));
	pProp = new CMFCPropertyGridProperty(_T("(名称)"), _T("应用程序"));
	pProp->Enable(FALSE);
	pGroup3->AddSubItem(pProp);

	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("窗口颜色"), RGB(210, 192, 254), nullptr, _T("指定默认的窗口颜色"));
	pColorProp->EnableOtherButton(_T("其他..."));
	pColorProp->EnableAutomaticButton(_T("默认"), ::GetSysColor(COLOR_3DFACE));
	pGroup3->AddSubItem(pColorProp);

	static const TCHAR szFilter[] = _T("图标文件(*.ico)|*.ico|所有文件(*.*)|*.*||");
	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("图标"), TRUE, _T(""), _T("ico"), 0, szFilter, _T("指定窗口图标")));

	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("文件夹"), _T("c:\\")));

	m_wndPropList.AddProperty(pGroup3);

	CMFCPropertyGridProperty* pGroup4 = new CMFCPropertyGridProperty(_T("层次结构"));

	CMFCPropertyGridProperty* pGroup41 = new CMFCPropertyGridProperty(_T("第一个子级"));
	pGroup4->AddSubItem(pGroup41);

	CMFCPropertyGridProperty* pGroup411 = new CMFCPropertyGridProperty(_T("第二个子级"));
	pGroup41->AddSubItem(pGroup411);

	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("项 1"), (_variant_t)_T("值 1"), _T("此为说明")));
	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("项 2"), (_variant_t)_T("值 2"), _T("此为说明")));
	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("项 3"), (_variant_t)_T("值 3"), _T("此为说明")));

	pGroup4->Expand(FALSE);
	m_wndPropList.AddProperty(pGroup4);
#endif
	HF_PropertyGridProperty* pGroup1 = new HF_PropertyGridProperty(_T("位置"), static_cast<int>(LINE_GROUP::POSITION));
	m_wndPropList.AddProperty(pGroup1);

	HF_PropertyGridProperty* pGroup2 = new HF_PropertyGridProperty(_T("样式"), static_cast<int>(LINE_GROUP::STYLE));
	m_wndPropList.AddProperty(pGroup2);

	HF_PropertyGridProperty* pGroup3 = new HF_PropertyGridProperty(_T("线粗"), static_cast<int>(LINE_GROUP::BORDER_WIDTH));
	m_wndPropList.AddProperty(pGroup3);

	HF_PropertyGridProperty* pGroup4 = new HF_PropertyGridProperty(_T("线型"), static_cast<int>(LINE_GROUP::LINE_STYLE));
	m_wndPropList.AddProperty(pGroup4);

	HF_PropertyGridProperty* pGroup5 = new HF_PropertyGridProperty(_T("边框颜色"), static_cast<int>(LINE_GROUP::BORDER_COLOR));
	m_wndPropList.AddProperty(pGroup5);

	HF_PropertyGridProperty* pGroup6 = new HF_PropertyGridProperty(_T("填充颜色"), static_cast<int>(LINE_GROUP::FILL_color));
	m_wndPropList.AddProperty(pGroup6);
}

void HF_PropertiesWnd::AddStyleProperty()
{
	auto pGroup = m_wndPropList.GetProperty(static_cast<int>(LINE_GROUP::STYLE));

	if (pGroup->GetSubItemsCount() == 0)
	{
		//CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(_T("线型"), _T("实线"), _T("线型"), static_cast<int>(LINE::TYPE));
		//pProp->AddOption(_T("实线"));
		//pProp->AddOption(_T("虚线"));
		//pProp->AddOption(_T("点划线"));
		//pProp->AddOption(_T("双点划线"));
		//pGroup->AddSubItem(pProp);
		//pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("粗细"), (_variant_t)_T("粗"), _T("指定线的粗细"), static_cast<int>(LINE::WIDTH)));
	}
}

void HF_PropertiesWnd::AddLineProperty(HF_Entity entity)
{
	auto pGroup = m_wndPropList.GetProperty(static_cast<int>(LINE_GROUP::POSITION));
	int nCnt = pGroup->GetSubItemsCount();
	if (nCnt != 0)
	{
		for (int i = nCnt-1; i >= 0; --i)
		{
			auto item = pGroup->GetSubItem(i);
			pGroup->RemoveSubItem(item);
		}
	}

	auto& Line = HFST::GetComponent<LineComponent>(entity.GetScene(), entity.GetHandleID());

	pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("起点 X"), (_variant_t)Line.m_Start.x,		_T("起点X坐标"), static_cast<int>(LINE::START_X)));
	pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("起点 Y"), (_variant_t)Line.m_Start.y,		_T("起点Y坐标"), static_cast<int>(LINE::START_Y)));
	pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("终点 X"), (_variant_t)Line.m_End.x,			_T("终点X坐标"), static_cast<int>(LINE::END_X)));
	pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("终点 Y"), (_variant_t)Line.m_End.y,			_T("终点Y坐标"), static_cast<int>(LINE::END_Y)));
}

void HF_PropertiesWnd::AddBorderColorProperty(HF_Entity entity)
{
	auto pGroup = m_wndPropList.GetProperty(static_cast<int>(LINE_GROUP::BORDER_COLOR));
	int nCnt = pGroup->GetSubItemsCount();
	if (nCnt != 0)
	{
		for (int i = nCnt - 1; i >= 0; --i)
		{
			auto item = pGroup->GetSubItem(i);
			pGroup->RemoveSubItem(item);
		}
	}

	if ( HFST::HasComponent<BorderColorComponent>(entity.GetScene(), entity.GetHandleID()) )
	{
		auto& border_color = HFST::GetComponent<BorderColorComponent>(entity.GetScene(), entity.GetHandleID());

		CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(
			_T("窗口颜色"),
			RGB(border_color.m_BorderColor.r * 255, border_color.m_BorderColor.g * 255, border_color.m_BorderColor.b * 255),
			nullptr,
			_T("指定默认的窗口颜色"),
			static_cast<int>(NORMAL_PROPERTY::BORDER_COLOR)
		);

		pColorProp->EnableOtherButton(_T("其他..."));
		pColorProp->EnableAutomaticButton(_T("默认"), ::GetSysColor(COLOR_3DFACE));
		pGroup->AddSubItem(pColorProp);
	}

	m_wndPropList.ExpandAll(FALSE);
	m_wndPropList.ExpandAll(TRUE);
}

void HF_PropertiesWnd::AddBorderWidthProperty(HF_Entity entity)
{
	auto pGroup = m_wndPropList.GetProperty(static_cast<int>(LINE_GROUP::BORDER_WIDTH));
	int nCnt = pGroup->GetSubItemsCount();
	if (nCnt != 0)
	{
		for (int i = nCnt - 1; i >= 0; --i)
		{
			auto item = pGroup->GetSubItem(i);
			pGroup->RemoveSubItem(item);
		}
	}

	if ( HFST::HasComponent<BorderWidthComponent>(entity.GetScene(), entity.GetHandleID()) )
	{
		auto& border_width = HFST::GetComponent<BorderWidthComponent>(entity.GetScene(), entity.GetHandleID());

		CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(
			_T("线宽"),
			(_variant_t)(int)border_width.m_BorderWidth, 
			_T("线宽"), 
			static_cast<int>(NORMAL_PROPERTY::BORDER_WIDTH)
		);
		pProp->EnableSpinControl(TRUE, 1, 50);
		pGroup->AddSubItem(pProp);
	}

	m_wndPropList.ExpandAll(FALSE);
	m_wndPropList.ExpandAll(TRUE);
}

void HF_PropertiesWnd::AddRectangleProperty(HF_Entity entity)
{
	auto pGroup = m_wndPropList.GetProperty(static_cast<int>(LINE_GROUP::POSITION));

	int nCnt = pGroup->GetSubItemsCount();
	if ( nCnt != 0 )
	{
		for ( int i = nCnt-1; i >= 0; --i )
		{
			auto item = pGroup->GetSubItem(i);
			pGroup->RemoveSubItem(item);
		}	
	}

	auto& rectangle = HFST::GetComponent<RectangleComponent>(entity.GetScene(), entity.GetHandleID());

	pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Left"),		(_variant_t)rectangle.m_LeftTop.x,		_T("起点X坐标"), static_cast<int>(RECTANGLE::LEFT)));
	pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Top"),		(_variant_t)rectangle.m_LeftTop.y,		_T("起点Y坐标"), static_cast<int>(RECTANGLE::TOP)));
	pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Right"),	(_variant_t)rectangle.m_RightBottom.x,	_T("终点X坐标"), static_cast<int>(RECTANGLE::RIGHT)));
	pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Bottom"),	(_variant_t)rectangle.m_RightBottom.y,	_T("终点Y坐标"), static_cast<int>(RECTANGLE::BOTTOM)));
}

LRESULT HF_PropertiesWnd::OnWmPropertyChanged(WPARAM wparam, LPARAM lparam)
{
	CMFCPropertyGridProperty* pProp = (CMFCPropertyGridProperty*)lparam;
	int pID = pProp->GetData();

	CString str = pProp->GetName();
	COleVariant i = pProp->GetValue();

	LPVARIANT pVar = (LPVARIANT)i;
	int x;
	short y;
	double d;
	float f;
	bool status;
	CString str1;
	switch (pVar->vt)
	{
		case VT_I2:    // short
			y = pVar->iVal;
			break;
		case VT_I4:     // int
			x = pVar->lVal;
			break;
		case VT_R4:    // float
			f = pVar->fltVal;
			break;
		case VT_R8:    // double
			d = pVar->dblVal;
			break;
		case VT_INT:
			x = pVar->lVal;
			break;
		case VT_BOOL:
			status = pVar->boolVal;
			break;
		case VT_BSTR:
			str1 = pVar->bstrVal;
			break;
			// etc.
	}
	((HF_MainFrame*)theApp.m_pMainWnd)->m_wndView.SetPropertyValue(pID, i);

	return 0;
}
