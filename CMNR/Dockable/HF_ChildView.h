#ifndef __HF_CHILD_VIEW_H__
#define __HF_CHILD_VIEW_H__

#include "HF_IcInfoDlg.h"
#include "HF_Header.h"
#include "HF_Scene.hpp"
constexpr int SZ = 200;

class HF_ChildView : public CWnd
{
public:
	inline static const COLORREF m_clrColors[5] = {
		RGB(255, 0, 0),
		RGB(255, 255, 0),
		RGB(0, 255, 0),
		RGB(0, 255, 255),
		RGB(0, 0, 255)
	};

	HF_ChildView();

	virtual BOOL	PreCreateWindow(CREATESTRUCT& cs);
	virtual			~HF_ChildView();
	virtual BOOL	PreTranslateMessage(MSG* pMsg);

	void		SetDrawType(DRAW_TYPE type) { m_emDrawType = type; }
	DRAW_TYPE	GetDrawType() const { return m_emDrawType; }
	
	void SetPropertyValue(int id, COleVariant value);

	void NewEntity(DRAW_TYPE type);

	HF_Scene* GetScene() const { return m_pScene.get(); }
	HF_Entity GetEntity() const { return m_Entity; }
protected:
	afx_msg void	OnPaint();
	afx_msg int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void	OnButtonViewLeft();
	afx_msg void	OnButtonViewRight();
	afx_msg void	OnButtonViewUp();
	afx_msg void	OnButtonViewFront();
	afx_msg void	OnButtonViewDown();
	afx_msg void	OnButtonViewBack();
	afx_msg void	OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void	OnShape(UINT id);
	afx_msg void	OnColor(UINT id);
	afx_msg void	OnShapeUI(CCmdUI* pCmd);
	afx_msg void	OnColorUI(CCmdUI* pCmd);
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnDraw2D(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL	OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void	OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	void DrawCtrls();
	void AssignLineProperty(int id, COleVariant value);
	void AssignRectangleProperty(int id, COleVariant value);
private:
	std::unique_ptr<HF_Scene> m_pScene;
	HF_Entity m_Entity;

	CFont		m_Font;
	CMFCButton	m_BtnLeft;
	CMFCButton	m_BtnRight;
	CMFCButton	m_BtnFront;
	CMFCButton	m_BtnBack;
	CMFCButton	m_BtnUp;
	CMFCButton	m_BtnDown;
	CMFCButton	m_BtnCenter;
	CStatic		m_StaI2cClk;
	CEdit		m_EdtI2cClk;
	CStatic		m_StaI2cAddr;
	CComboBox	m_ComboI2cAddr;
	CStatic		m_StaBridge;
	CComboBox	m_ComboBridge;

	UINT		m_nColor{0};
	UINT		m_nShape{0};

	COLORREF	m_Color{ RGB(255,255,255) };

	float		m_nGap{ 5.0f };
	float		m_fRatio = 1.0f;

	DRAW_TYPE	m_emDrawType{ DRAW_TYPE::NONE };
public:
};

#endif //__HF_CHILD_VIEW_H__

