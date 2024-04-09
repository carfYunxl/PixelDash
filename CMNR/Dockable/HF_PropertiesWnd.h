#ifndef __HF_PROPERTIES_WND_H__
#define __HF_PROPERTIES_WND_H__

class HF_ChildView;

class HF_PropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class HF_PropertiesWnd : public CDockablePane
{
	// 构造
public:
	enum class LINE
	{
		START_X = 0,
		START_Y,
		END_X,
		END_Y,
		ROTATE,
		TYPE,
		WIDTH
	};
	HF_PropertiesWnd() noexcept;

	void AdjustLayout();

	// 特性
public:
	void SetVSDotNetLook(BOOL bSet)
	{
		m_wndPropList.SetVSDotNetLook(bSet);
		m_wndPropList.SetGroupNameFullWidth(bSet);
	}

	CMFCPropertyGridCtrl& GetPropertyCtrl() { return m_wndPropList; }

	void AddDefaultProperty();
	void AddLineProperty();

protected:
	CFont					m_fntPropList;
	CComboBox				m_wndObjectCombo;
	HF_PropertiesToolBar	m_wndToolBar;
	CMFCPropertyGridCtrl	m_wndPropList;

	// 实现
public:
	virtual ~HF_PropertiesWnd();

	void SetMainView(HF_ChildView* pMainView) {
		m_pMainView = pMainView;
	}

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

	void InitPropList();
	void SetPropListFont();
private:
	int m_nComboHeight;
	HF_ChildView* m_pMainView{ nullptr };
};

#endif //__HF_PROPERTIES_WND_H__

