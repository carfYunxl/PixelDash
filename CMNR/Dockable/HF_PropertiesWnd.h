#ifndef __HF_PROPERTIES_WND_H__
#define __HF_PROPERTIES_WND_H__

class HF_PropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class HF_PropertyGridProperty : public CMFCPropertyGridProperty
{
public:
	HF_PropertyGridProperty(const CString& strGroupName, DWORD_PTR dwData = 0, BOOL bIsValueList = FALSE)
		: CMFCPropertyGridProperty(strGroupName, dwData, bIsValueList)
	{}
	HF_PropertyGridProperty(
		const CString& strName, 
		const COleVariant& varValue, 
		LPCTSTR lpszDescr = NULL, 
		DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL,
		LPCTSTR lpszEditTemplate = NULL, 
		LPCTSTR lpszValidChars = NULL)
		: CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars)
	{}

	virtual BOOL HasButton() const {
		return TRUE;
	}

	virtual void OnClickButton(CPoint point)
	{
		CMFCPropertyGridProperty::OnClickButton(point);

		if(IsGroup())
			AfxMessageBox(L"Button clicked!");
	}

	virtual void OnDrawButton(CDC* pDC, CRect rect)
	{
		CMFCPropertyGridProperty::OnDrawButton( pDC, rect);
		if (/*IsGroup() || */GetHierarchyLevel() == 0)
		{
			CBrush brush(RGB(155, 155, 155));

			CRect rec = rect;
			rec.left -= rect.Width() * 2;

			pDC->FillRect(rec, CBrush::FromHandle(GetSysColorBrush(COLOR_BTNFACE)));

			if(GetSubItemsCount() == 0)
				pDC->DrawText(_T("添加") , rec, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			else
				pDC->DrawText(_T("删除"), rec, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}
	}

	virtual HBRUSH OnCtlColor(CDC* pDC, UINT nCtlColor)
	{
		CMFCPropertyGridProperty::OnCtlColor(pDC, nCtlColor);

		if (IsGroup() && !HasValueField())
		{
			static CBrush brush(RGB(155, 155, 155));
			pDC->SetBkColor(RGB(155, 155, 155));
			return(HBRUSH)brush;
		}
		return NULL;
	}

	void OnDrawName(CDC* pDC, CRect rect)
	{
		CMFCPropertyGridProperty::OnDrawName(pDC, rect);

		if (IsGroup()/* && !HasValueField()*/)
		{
			pDC->SetTextColor(RGB(255,0,0));
		}
	}
};

class HF_PropertiesWnd : public CDockablePane
{
	// 构造
public:

	enum class LINE_GROUP
	{
		POSITION = 0,
		STYLE
	};

	enum class LINE
	{
		START_X = 10,
		START_Y,
		END_X,
		END_Y,
		ROTATE,
		TYPE,
		WIDTH
	};
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
	void AddPositionProperty();
	void AddStypeProperty();
private:
	CFont					m_fntPropList;
	CComboBox				m_wndObjectCombo;
	HF_PropertiesToolBar	m_wndToolBar;
	CMFCPropertyGridCtrl	m_wndPropList;
	int						m_nComboHeight;
};

#endif //__HF_PROPERTIES_WND_H__

