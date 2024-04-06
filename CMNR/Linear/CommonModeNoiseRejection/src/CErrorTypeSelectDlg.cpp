#include "pch.h"
#include "../CommonModeNoiseRejection.h"
#include "CErrorTypeSelectDlg.h"

IMPLEMENT_DYNAMIC(CErrorTypeSelectDlg, CDialogEx)

CErrorTypeSelectDlg::CErrorTypeSelectDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ERROR_TYPE_SEL_DIALOG, pParent)
{
    memset(&m_rErrTypeSel, 0, sizeof(ErrorTypeSel));
}

CErrorTypeSelectDlg::~CErrorTypeSelectDlg()
{
}

void CErrorTypeSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CErrorTypeSelectDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CErrorTypeSelectDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_CHECK_JITTER, &CErrorTypeSelectDlg::OnBnClickedCheckJitter)
    ON_EN_KILLFOCUS(IDC_EDIT_JITTER_VALUE, &CErrorTypeSelectDlg::OnEnKillfocusEditJitterValue)
END_MESSAGE_MAP()


// CErrorTypeSelectDlg 消息处理程序
BOOL CErrorTypeSelectDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    if (m_rErrTypeSel.fgNoTouch)
    {
        ((CButton*)GetDlgItem(IDC_CHECK_NO_TOUCH))->SetCheck(BST_CHECKED);
    }
    if (m_rErrTypeSel.fgMoreTouch)
    {
        ((CButton*)GetDlgItem(IDC_CHECK_MORE_TOUCH))->SetCheck(BST_CHECKED);
    }
    if (m_rErrTypeSel.fgPenUp)
    {
        ((CButton*)GetDlgItem(IDC_CHECK_PEN_UP))->SetCheck(BST_CHECKED);
    }
    if (m_rErrTypeSel.fgJitter)
    {
        ((CButton*)GetDlgItem(IDC_CHECK_JITTER))->SetCheck(BST_CHECKED);
        ((CButton*)GetDlgItem(IDC_EDIT_JITTER_VALUE))->EnableWindow(true);
        SetDlgItemInt(IDC_EDIT_JITTER_VALUE, m_rErrTypeSel.uiJitterValue);
    }
    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}

BOOL CErrorTypeSelectDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
    {
        return TRUE;
    }

    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && pMsg->wParam)
    {
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void CErrorTypeSelectDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
    if (((CButton*)GetDlgItem(IDC_CHECK_JITTER))->GetCheck())
    {
        CString str;
        m_rErrTypeSel.fgJitter = true;
        ((CButton*)GetDlgItem(IDC_EDIT_JITTER_VALUE))->GetWindowText(str);
        m_rErrTypeSel.uiJitterValue = _tcstol(str, NULL, 10);
        if (m_rErrTypeSel.uiJitterValue <= 0)
        {
            AfxMessageBox(_T("Please input right jitter value\n"));
            return;
        }
    }
	if (((CButton*)GetDlgItem(IDC_CHECK_NO_TOUCH))->GetCheck())
	{
		m_rErrTypeSel.fgNoTouch = true;
	}
    if (((CButton*)GetDlgItem(IDC_CHECK_MORE_TOUCH))->GetCheck())
    {
        m_rErrTypeSel.fgMoreTouch = true;
    }
    if (((CButton*)GetDlgItem(IDC_CHECK_PEN_UP))->GetCheck())
    {
        m_rErrTypeSel.fgPenUp = true;
    }
    
	CDialogEx::OnOK();
}


void CErrorTypeSelectDlg::OnBnClickedCheckJitter()
{
    // TODO: 在此添加控件通知处理程序代码
    if (((CButton*)GetDlgItem(IDC_CHECK_JITTER))->GetCheck())
    {
        ((CButton*)GetDlgItem(IDC_EDIT_JITTER_VALUE))->EnableWindow(true);
    }
    else
    {
        ((CButton*)GetDlgItem(IDC_EDIT_JITTER_VALUE))->EnableWindow(false);
    }
}


void CErrorTypeSelectDlg::OnEnKillfocusEditJitterValue()
{
    // TODO: 在此添加控件通知处理程序代码
    CString str, msg;
    ((CButton*)GetDlgItem(IDC_EDIT_JITTER_VALUE))->GetWindowText(str);
    //判断是否是非十六进制数字，如果是提示
    std::string pattern = "[0-9]*";
    std::regex re(pattern);
    std::string strstring = CT2A(str.GetString()); //Unicode编码, 所以CT2A其实就是CW2A就是将Unicode转换为多字符集ASCII，也可写成CW2A
    if (!std::regex_match(strstring, re))
    {
        msg.Format(_T("\"%s\" 里有非法符号, 请填写十进制数字!\n"), str);
        AfxMessageBox(msg);
        return;
    }
}
