#include "pch.h"
#include "HF_PropertyGridProperty.hpp"

#include "HF_MainFrm.h"

HF_PropertyGridProperty::HF_PropertyGridProperty(const CString& strGroupName, DWORD_PTR dwData/* = 0*/, BOOL bIsValueList /*= FALSE*/)
	: CMFCPropertyGridProperty(strGroupName, dwData, bIsValueList)
{
}
HF_PropertyGridProperty::HF_PropertyGridProperty(
	const CString& strName,
	const COleVariant& varValue,
	LPCTSTR lpszDescr/* = NULL*/,
	DWORD_PTR dwData/* = 0*/,
	LPCTSTR lpszEditMask /*= NULL*/,
	LPCTSTR lpszEditTemplate/* = NULL*/,
	LPCTSTR lpszValidChars/* = NULL*/)
	: CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars)
{
}

BOOL HF_PropertyGridProperty::HasButton() const {
	return TRUE;
}

HF_PropertyGridProperty::~HF_PropertyGridProperty() {}

void HF_PropertyGridProperty::OnDrawButton(CDC* pDC, CRect rect)
{
	if (GetHierarchyLevel() == 0)
	{
		CRect rec = rect;
		rec.left -= rect.Width() * 2;

		m_recButton = rec;

		CBrush brush(m_GroupBkColor);
		pDC->FillRect(rec, &brush);

		auto old_color = pDC->SetTextColor(RGB(255, 255, 255));
		if (GetSubItemsCount() == 0)
			pDC->DrawText(_T("Add"), rec, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		else
			pDC->DrawText(_T("Delete"), rec, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		pDC->SetTextColor(old_color);
	}
}

void HF_PropertyGridProperty::OnClickName(CPoint point)
{
	if (m_recButton.IsRectNull() || !m_recButton.PtInRect(point)) {
		return;
	}

	auto group_property = HitTest(point);

	if (!group_property) {
		return;
	}

	auto id = group_property->GetData();
	if (group_property->GetSubItemsCount() == 0)
	{
		switch (static_cast<LINE_GROUP>(id))
		{
			case LINE_GROUP::POSITION:
				break;
			case LINE_GROUP::TRANSFORM:
			{
				HF_MainFrame* pMainFrame = (HF_MainFrame*)(theApp.m_pMainWnd);
				HF_Entity entity = pMainFrame->m_wndView.GetEntity();
				HFST::AddComponent<TransformComponent>(entity.GetScene(), entity.GetHandleID());

				pMainFrame->m_wndProperty.AddTransformProperty(entity);
				break;
			}
			case LINE_GROUP::BORDER_COLOR:
			{
				HF_MainFrame* pMainFrame = (HF_MainFrame*)(theApp.m_pMainWnd);
				HF_Entity entity = pMainFrame->m_wndView.GetEntity();
				HFST::AddComponent<BorderColorComponent>(entity.GetScene(), entity.GetHandleID());

				pMainFrame->m_wndProperty.AddBorderColorProperty(entity);
				break;
			}
			case LINE_GROUP::BORDER_WIDTH:
			{
				HF_MainFrame* pMainFrame = (HF_MainFrame*)(theApp.m_pMainWnd);
				HF_Entity entity = pMainFrame->m_wndView.GetEntity();

				HFST::AddComponent<BorderWidthComponent>(entity.GetScene(), entity.GetHandleID());

				pMainFrame->m_wndProperty.AddBorderWidthProperty(entity);
				break;
			}
			case LINE_GROUP::LINE_STYLE:
				break;
			case LINE_GROUP::FILL_color:
				break;
		}
	}
	else
	{
		switch (static_cast<LINE_GROUP>(id))
		{
			case LINE_GROUP::POSITION:
				break;
			case LINE_GROUP::TRANSFORM:
			{
				HF_MainFrame* pMainFrame = (HF_MainFrame*)(theApp.m_pMainWnd);
				HF_Entity entity = pMainFrame->m_wndView.GetEntity();

				auto& trans = HFST::GetComponent<TransformComponent>(entity.GetScene(), entity.GetHandleID());
				trans.m_Center = {0.0f, 0.0f};
				trans.m_Trans = {0.0f,0.0f};
				trans.m_Scale = {1.0f,1.0f};
				trans.m_Rotate = 0.0f;

				HFST::RemoveComponent<TransformComponent>(entity.GetScene(), entity.GetHandleID());

				pMainFrame->m_wndProperty.AddTransformProperty(entity);
				pMainFrame->m_wndView.Invalidate();
				break;
			}
			case LINE_GROUP::BORDER_COLOR:
			{
				HF_MainFrame* pMainFrame = (HF_MainFrame*)(theApp.m_pMainWnd);
				HF_Entity entity = pMainFrame->m_wndView.GetEntity();

				auto& border_color = HFST::GetComponent<BorderColorComponent>(entity.GetScene(), entity.GetHandleID());
				border_color.m_BorderColor = D2D1::ColorF::Black;

				HFST::RemoveComponent<BorderColorComponent>(entity.GetScene(), entity.GetHandleID());

				pMainFrame->m_wndProperty.AddBorderColorProperty(entity);
				pMainFrame->m_wndView.Invalidate();
				break;
			}
			case LINE_GROUP::BORDER_WIDTH:
			{
				HF_MainFrame* pMainFrame = (HF_MainFrame*)(theApp.m_pMainWnd);
				HF_Entity entity = pMainFrame->m_wndView.GetEntity();

				auto& border_width = HFST::GetComponent<BorderWidthComponent>(entity.GetScene(), entity.GetHandleID());
				border_width.m_BorderWidth = 1.0f;

				HFST::RemoveComponent<BorderWidthComponent>(entity.GetScene(), entity.GetHandleID());

				pMainFrame->m_wndProperty.AddBorderWidthProperty(entity);
				pMainFrame->m_wndView.Invalidate();
				break;
			}
			case LINE_GROUP::LINE_STYLE:
				break;
			case LINE_GROUP::FILL_color:
				break;
		}
	}

}

BOOL HF_PropertyGridProperty::HasValueField() const { return FALSE; }

void HF_PropertyGridProperty::OnDrawName(CDC* pDC, CRect rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(m_pWndList);

	COLORREF clrTextOld = (COLORREF)-1;
	if (IsSelected())
	{
		CRect rectFill = rect;
		rectFill.top++;

		clrTextOld = pDC->SetTextColor(GetGlobalData()->clrTextHilite);

		CBrush brush(m_GroupBkColor);
		pDC->FillRect(rectFill, &brush);
	}

	if (m_pParent != NULL)
	{
		rect.left += rect.Height();
	}

	rect.DeflateRect(AFX_TEXT_MARGIN, 0);

	int nTextHeight = pDC->DrawText(m_strName, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

	m_bNameIsTruncated = pDC->GetTextExtent(m_strName).cx > rect.Width();

	if (IsSelected())
	{
		CRect rectFocus = rect;
		rectFocus.top = rectFocus.CenterPoint().y - nTextHeight / 2;
		rectFocus.bottom = rectFocus.top + nTextHeight;
		rectFocus.right = min(rect.right, rectFocus.left + pDC->GetTextExtent(m_strName).cx);
		rectFocus.InflateRect(2, 0);

		COLORREF clrShadow = GetGlobalData()->clrBarShadow;

		pDC->Draw3dRect(rectFocus, clrShadow, clrShadow);
	}

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor(clrTextOld);
	}
}
