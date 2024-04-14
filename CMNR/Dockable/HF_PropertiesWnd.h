#ifndef __HF_PROPERTIES_WND_H__
#define __HF_PROPERTIES_WND_H__
#include "HF_Components.hpp"

class HF_PropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class HF_PropertyGridProperty final : public CMFCPropertyGridProperty
{
public:
	HF_PropertyGridProperty(const CString& strGroupName, DWORD_PTR dwData = 0, BOOL bIsValueList = FALSE)
		: CMFCPropertyGridProperty(strGroupName, dwData, bIsValueList)
	{
	}
	HF_PropertyGridProperty(
		const CString& strName, 
		const COleVariant& varValue, 
		LPCTSTR lpszDescr = NULL, 
		DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL,
		LPCTSTR lpszEditTemplate = NULL, 
		LPCTSTR lpszValidChars = NULL)
		: CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars)
	{
	}

	virtual BOOL HasButton() const override {
		return TRUE;
	}

	~HF_PropertyGridProperty() {
	}

	virtual void OnDrawButton(CDC* pDC, CRect rect) override
	{
		if (GetHierarchyLevel() == 0)
		{
			CRect rec = rect;
			rec.left -= rect.Width() * 2;

			m_recButton = rec;

			CBrush brush(m_GroupBkColor);
			pDC->FillRect(rec, &brush);

			auto old_color = pDC->SetTextColor(RGB(255,255,255));
			if(GetSubItemsCount() == 0)
				pDC->DrawText(_T("Add") , rec, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			else
				pDC->DrawText(_T("Delete"), rec, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			pDC->SetTextColor(old_color);
		}
	}

	virtual void OnClickName(CPoint point) override
	{
		if ( m_recButton.IsRectNull() || !m_recButton.PtInRect(point) ) {
			return;
		}

		auto group_property = HitTest(point);

		if (!group_property) {
			return;
		}

		auto id = group_property->GetData();
		if ( group_property->GetSubItemsCount() == 0 )
		{
			// Add Component
			switch (static_cast<LINE_GROUP>(id))
			{
				case LINE_GROUP::POSITION:
					break;
				case LINE_GROUP::STYLE:
					break;
				case LINE_GROUP::LINE_COLOR:
					break;
				case LINE_GROUP::LINE_WIDTH:
					break;
				case LINE_GROUP::LINE_STYLE:
					break;
				case LINE_GROUP::FILL_color:
					break;
			}
			// Add UI
		}
		else
		{
			// Remove Component

			// Remove UI
		}

	}

	virtual BOOL HasValueField() const override { return FALSE; }

	virtual void OnDrawName(CDC* pDC, CRect rect) override
	{
		ASSERT_VALID(this);
		ASSERT_VALID(pDC);
		ASSERT_VALID(m_pWndList);

		COLORREF clrTextOld = (COLORREF)-1;
		if ( IsSelected() )
		{
			CRect rectFill = rect;
			rectFill.top++;

			clrTextOld = pDC->SetTextColor(GetGlobalData()->clrTextHilite);

			CBrush brush(m_GroupBkColor);
			pDC->FillRect(rectFill, &brush);
		}

		if ( m_pParent != NULL )
		{
			rect.left += rect.Height();
		}

		rect.DeflateRect(AFX_TEXT_MARGIN, 0);

		int nTextHeight = pDC->DrawText(m_strName, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

		m_bNameIsTruncated = pDC->GetTextExtent(m_strName).cx > rect.Width();

		if ( IsSelected() )
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

private:
	CRect		m_recButton;
	COLORREF	m_GroupBkColor{ RGB(27,103,160) };
};

class HF_PropertiesWnd : public CDockablePane
{
	// 构造
public:
	HF_PropertiesWnd() noexcept;
	virtual ~HF_PropertiesWnd();
private:
	void AdjustLayout();
	void SetVSDotNetLook(BOOL bSet) {
		m_wndPropList.SetVSDotNetLook(bSet);
		m_wndPropList.SetGroupNameFullWidth(bSet);
	}
	void InitPropList();
	void SetPropListFont();
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnExpandAllProperties();
	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	afx_msg void OnSortProperties();
	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);
	afx_msg void OnProperties1();
	afx_msg void OnUpdateProperties1(CCmdUI* pCmdUI);
	afx_msg void OnProperties2();
	afx_msg void OnUpdateProperties2(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnWmPropertyChanged(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()

public:
	CMFCPropertyGridCtrl& GetPropertyCtrl() { return m_wndPropList; }
	void AddDefaultProperty();
	void AddLineProperty(HF_Entity entity);
	void AddRectangleProperty(HF_Entity entity);
	void AddStyleProperty();
private:
	CFont					m_fntPropList;
	HF_PropertiesToolBar	m_wndToolBar;
	CMFCPropertyGridCtrl	m_wndPropList;
};

#endif //__HF_PROPERTIES_WND_H__

