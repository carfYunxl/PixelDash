#pragma once
#include "afxdialogex.h"
#include "CommonModeNoiseRejectionDlg.h"
#include <vector>

class CPointCloud : public CDialogEx
{
	DECLARE_DYNAMIC(CPointCloud)

public:
	CPointCloud(float width, float height, CWnd* pParent = nullptr);
	virtual ~CPointCloud();
	virtual BOOL OnInitDialog();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_POINT_CANVAS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();

public:
	void SetAllPoint(const std::vector<CPoint>& vecAll) {
		m_vecAllPt = vecAll;
		Invalidate();
	}
	void AddFailPt(const vPoint& pt) {
		m_vecFailPt.push_back(pt);
		Invalidate();
		SetDlgItemInt(IDC_EDIT_NG, m_vecFailPt.size());
	}

	void AddSuccessPt(const vPoint& pt) {
		m_vecSuccessPt.push_back(pt);
		Invalidate();
		SetDlgItemInt(IDC_EDIT_PASS, m_vecSuccessPt.size());
	}

	void SetEdgeSize(int size) {
		m_EdgeSize = size;
	}

	void SetDebugMode(BOOL debug) {
		m_bDebugMode = debug;
	}

	void ClearAll() {
		m_vecAllPt.clear();
		m_vecFailPt.clear();
		m_vecSuccessPt.clear();
	}

	void SetRadius(int radius)
	{
		m_radius = radius;
	}
	void RemoveFailPt(const vPoint& vp);

	void ClearNG()
	{
		m_vecFailPt.clear();
		Invalidate();
		SetDlgItemText(IDC_EDIT_NG, _T("0"));
	}

	void SetResX(int resX)
	{
		m_CanvasWidth = resX;
		SetWindowPos(nullptr, 0, 0, int(m_Ratio * m_CanvasWidth + 25), int(m_Ratio * m_CanvasHeight + 55), SWP_NOMOVE | SWP_SHOWWINDOW);
	}
	void SetResY(int resY)
	{
		m_CanvasHeight = resY;
		SetWindowPos(nullptr, 0, 0, int(m_Ratio * m_CanvasWidth + 25), int(m_Ratio * m_CanvasHeight + 55), SWP_NOMOVE | SWP_SHOWWINDOW);
	}
private:
	std::vector<CPoint> m_vecAllPt;
	std::vector<vPoint> m_vecFailPt;
	std::vector<vPoint> m_vecSuccessPt;

	float m_CanvasWidth{0.0f};
	float m_CanvasHeight{0.0f};

	int m_radius{8};
	int mCurSel{0};
	float m_Ratio{1};
	int m_EdgeSize{5};

	BOOL m_bDebugMode{FALSE};

	BOOL bShowNG{TRUE};
	BOOL bShowPASS{ TRUE };

public:
	afx_msg void OnCbnSelchangeComboSection();
	afx_msg void OnBnClickedCheckShowNg();
	afx_msg void OnBnClickedCheckShowPass();
	afx_msg void OnEnChangeEditWidth();
	afx_msg void OnEnChangeEditHeight();
};
