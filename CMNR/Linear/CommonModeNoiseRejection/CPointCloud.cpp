#include "pch.h"
#include "CommonModeNoiseRejection.h"
#include "CPointCloud.h"

IMPLEMENT_DYNAMIC(CPointCloud, CDialogEx)

CPointCloud::CPointCloud(float width, float height, CWnd* pParent)
	: m_CanvasWidth(width)
	, m_CanvasHeight(height)
	, CDialogEx(IDD_DIALOG_POINT_CANVAS, pParent)
{

}

CPointCloud::~CPointCloud()
{
}

void CPointCloud::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPointCloud, CDialogEx)
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_COMBO_SECTION, &CPointCloud::OnCbnSelchangeComboSection)
	ON_BN_CLICKED(IDC_CHECK_SHOW_NG, &CPointCloud::OnBnClickedCheckShowNg)
	ON_BN_CLICKED(IDC_CHECK_SHOW_PASS, &CPointCloud::OnBnClickedCheckShowPass)
	ON_EN_CHANGE(IDC_EDIT_WIDTH, &CPointCloud::OnEnChangeEditWidth)
	ON_EN_CHANGE(IDC_EDIT_HEIGHT, &CPointCloud::OnEnChangeEditHeight)
END_MESSAGE_MAP()


void CPointCloud::OnPaint()
{

	constexpr int offsetX = 10;
	constexpr int offsetY = 40;

	CPaintDC dc(this);

	CPen green_pen(PS_SOLID, 1, RGB(0, 255, 0));
	CPen red_pen(PS_SOLID, 1, RGB(255, 0, 0));
	CPen white_pen(PS_SOLID, 1, RGB(255, 255, 255));

	//draw black background
	dc.FillSolidRect(offsetX, offsetY, static_cast<int>(m_Ratio * m_CanvasWidth), static_cast<int>(m_Ratio * m_CanvasHeight), RGB(0,0,0));

	//draw all point
	dc.SelectObject(&green_pen);
	dc.SelectStockObject(NULL_BRUSH);
	for ( size_t i = 0;i < m_vecAllPt.size(); ++i)
	{
		const auto& pt = m_vecAllPt.at(i);

		int radius = m_radius;
		if (m_bDebugMode)
		{
			if (
				pt.x <= 8 * m_EdgeSize ||
				pt.x >= m_CanvasWidth - 8 * m_EdgeSize ||
				pt.y <= 8 * m_EdgeSize ||
				pt.y >= m_CanvasHeight - 8 * m_EdgeSize)
			{
				radius = static_cast<int>(m_radius * 1.5);
			}
		}
		else
		{
			radius = static_cast<int>(m_radius * 2.0);
		}

		if ( mCurSel == 0 )
		{
			dc.MoveTo(static_cast<int>(m_Ratio * pt.x + offsetX - radius), static_cast<int>(m_Ratio * pt.y + offsetY));
			dc.LineTo(static_cast<int>(m_Ratio * pt.x + offsetX + radius), static_cast<int>(m_Ratio * pt.y + offsetY));
			dc.MoveTo(static_cast<int>(m_Ratio * pt.x + offsetX), static_cast<int>(m_Ratio * pt.y + offsetY - radius));
			dc.LineTo(static_cast<int>(m_Ratio * pt.x + offsetX), static_cast<int>(m_Ratio * pt.y + offsetY + radius));

			dc.RoundRect(
				static_cast<int>(m_Ratio * pt.x + offsetX - radius),
				static_cast<int>(m_Ratio * pt.y + offsetY - radius),
				static_cast<int>(m_Ratio * pt.x + offsetX + radius), 
				static_cast<int>(m_Ratio * pt.y + offsetY + radius), 
				static_cast<int>(2 * radius), 
				static_cast<int>(2 * radius)
			);
		}
		else
		{
			if ( int(i) >= (mCurSel-1)* 100 && int(i) < mCurSel * 100)
			{
				dc.MoveTo(static_cast<int>(m_Ratio * pt.x + offsetX - radius), static_cast<int>(m_Ratio * pt.y + offsetY));
				dc.LineTo(static_cast<int>(m_Ratio * pt.x + offsetX + radius), static_cast<int>(m_Ratio * pt.y + offsetY));
				dc.MoveTo(static_cast<int>(m_Ratio * pt.x + offsetX), static_cast<int>(m_Ratio * pt.y + offsetY - radius));
				dc.LineTo(static_cast<int>(m_Ratio * pt.x + offsetX), static_cast<int>(m_Ratio * pt.y + offsetY + radius));

				dc.RoundRect(
					static_cast<int>(m_Ratio * pt.x + offsetX - radius), 
					static_cast<int>(m_Ratio * pt.y + offsetY - radius), 
					static_cast<int>(m_Ratio * pt.x + offsetX + radius),
					static_cast<int>(m_Ratio * pt.y + offsetY + radius),
					static_cast<int>(2 * radius),
					static_cast<int>(2 * radius)
				);
			}
		}
	}

	//draw fail pt
	if (bShowNG)
	{
		dc.SelectObject(&red_pen);
		for (size_t i = 0; i < m_vecFailPt.size(); ++i)
		{
			const auto& pt = m_vecFailPt.at(i);

			if (mCurSel == 0)
			{
				dc.MoveTo(static_cast<int>(m_Ratio * pt.x + offsetX - 3), static_cast<int>(m_Ratio * pt.y + offsetY));
				dc.LineTo(static_cast<int>(m_Ratio * pt.x + offsetX + 3), static_cast<int>(m_Ratio * pt.y + offsetY));
				dc.MoveTo(static_cast<int>(m_Ratio * pt.x + offsetX), static_cast<int>(m_Ratio * pt.y + offsetY - 3));
				dc.LineTo(static_cast<int>(m_Ratio * pt.x + offsetX), static_cast<int>(m_Ratio * pt.y + offsetY + 3));
			}
			else
			{
				if (int(i) >= (mCurSel - 1) * 100 && int(i) < mCurSel * 100)
				{
					dc.MoveTo(static_cast<int>(m_Ratio * pt.x + offsetX - 3), static_cast<int>(m_Ratio * pt.y + offsetX));
					dc.LineTo(static_cast<int>(m_Ratio * pt.x + offsetX + 3), static_cast<int>(m_Ratio * pt.y + offsetX));
					dc.MoveTo(static_cast<int>(m_Ratio * pt.x + offsetX), static_cast<int>(m_Ratio * pt.y + offsetY - 3));
					dc.LineTo(static_cast<int>(m_Ratio * pt.x + offsetX), static_cast<int>(m_Ratio * pt.y + offsetY + 3));
				}
			}
		}
	}

	//draw success pt
	if (bShowPASS)
	{
		dc.SelectObject(&white_pen);
		for (size_t i = 0; i < m_vecSuccessPt.size(); ++i)
		{
			const auto& pt = m_vecSuccessPt.at(i);

			if (mCurSel == 0)
			{
				dc.MoveTo(static_cast<int>(m_Ratio * pt.x + offsetX - 3), static_cast<int>(m_Ratio * pt.y + offsetY));
				dc.LineTo(static_cast<int>(m_Ratio * pt.x + offsetX + 3), static_cast<int>(m_Ratio * pt.y + offsetY));
				dc.MoveTo(static_cast<int>(m_Ratio * pt.x + offsetX), static_cast<int>(m_Ratio * pt.y + offsetY - 3));
				dc.LineTo(static_cast<int>(m_Ratio * pt.x + offsetX), static_cast<int>(m_Ratio * pt.y + offsetY + 3));
			}
			else
			{
				if (int(i) >= (mCurSel - 1) * 100 && int(i) < mCurSel * 100)
				{
					dc.MoveTo(static_cast<int>(m_Ratio * pt.x + offsetX - 3), static_cast<int>(m_Ratio * pt.y + offsetX));
					dc.LineTo(static_cast<int>(m_Ratio * pt.x + offsetX + 3), static_cast<int>(m_Ratio * pt.y + offsetX));
					dc.MoveTo(static_cast<int>(m_Ratio * pt.x + offsetX), static_cast<int>(m_Ratio * pt.y + offsetY - 3));
					dc.LineTo(static_cast<int>(m_Ratio * pt.x + offsetX), static_cast<int>(m_Ratio * pt.y + offsetY + 3));
				}
			}
		}
	}
	CDialogEx::OnPaint();
}


BOOL CPointCloud::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CComboBox* pCom = (CComboBox*)GetDlgItem(IDC_COMBO_SECTION);
	if (pCom)
	{
		pCom->AddString(_T("All Section"));
		pCom->AddString(_T("Section 0"));
		pCom->AddString(_T("Section 1"));
		pCom->AddString(_T("Section 2"));
		pCom->AddString(_T("Section 3"));
		pCom->AddString(_T("Section 4"));
		pCom->AddString(_T("Section 5"));
		pCom->AddString(_T("Section 6"));
		pCom->AddString(_T("Section 7"));
		pCom->AddString(_T("Section 8"));
		pCom->AddString(_T("Section 9"));
		pCom->SetCurSel(0);
	}

	//IDC_EDIT_WIDTH
	SetDlgItemInt(IDC_EDIT_WIDTH, int(m_CanvasWidth));

	//IDC_EDIT_WIDTH
	SetDlgItemInt(IDC_EDIT_HEIGHT, int(m_CanvasHeight));

	SetWindowPos(nullptr, 0, 0, int(m_Ratio * m_CanvasWidth + 25), int(m_Ratio * m_CanvasHeight + 55), SWP_NOMOVE | SWP_SHOWWINDOW);

	SetDlgItemInt( IDC_EDIT_PASS, 0 );
	SetDlgItemInt( IDC_EDIT_NG,   0 );

	((CButton*)GetDlgItem(IDC_CHECK_SHOW_NG))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_SHOW_PASS))->SetCheck(BST_CHECKED);

	return TRUE;
}

void CPointCloud::RemoveFailPt(const vPoint& vp)
{
	auto itr = std::find_if(m_vecFailPt.begin(), m_vecFailPt.end(), [=](const vPoint& pt){
		return pt == vp;
	});

	if ( itr != m_vecFailPt.end() )
	{
		m_vecFailPt.erase(itr);
	}
}


void CPointCloud::OnCbnSelchangeComboSection()
{
	CComboBox* pCom = (CComboBox*)GetDlgItem(IDC_COMBO_SECTION);
	if (pCom)
	{
		mCurSel = pCom->GetCurSel();
	}

	Invalidate();
}


void CPointCloud::OnBnClickedCheckShowNg()
{
	BOOL checked = ((CButton*)GetDlgItem(IDC_CHECK_SHOW_NG))->GetCheck();

	bShowNG = checked ? TRUE : FALSE;

	Invalidate();
}


void CPointCloud::OnBnClickedCheckShowPass()
{
	BOOL checked = ((CButton*)GetDlgItem(IDC_CHECK_SHOW_PASS))->GetCheck();

	bShowPASS = checked ? TRUE : FALSE;

	Invalidate();
}


void CPointCloud::OnEnChangeEditWidth()
{
	CString strText;
	GetDlgItemText(IDC_EDIT_WIDTH, strText);
	if (strText.IsEmpty())
		return;

	m_CanvasWidth = _ttoi(strText);
}


void CPointCloud::OnEnChangeEditHeight()
{
	CString strText;
	GetDlgItemText(IDC_EDIT_HEIGHT, strText);
	if (strText.IsEmpty())
		return;

	m_CanvasHeight = _ttoi(strText);
}
