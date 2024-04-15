#ifndef __HF_PROPERTIES_WND_H__
#define __HF_PROPERTIES_WND_H__
#include "HF_Components.hpp"
#include "HF_PropertyGridProperty.hpp"

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
	void AddBorderColorProperty(HF_Entity entity);
	void AddBorderWidthProperty(HF_Entity entity);
private:
	CFont					m_fntPropList;
	HF_PropertiesToolBar	m_wndToolBar;
	CMFCPropertyGridCtrl	m_wndPropList;
};

#endif //__HF_PROPERTIES_WND_H__

