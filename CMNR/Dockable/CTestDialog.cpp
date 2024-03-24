#include "pch.h"
#include "CMNR.h"
#include "CTestDialog.h"

IMPLEMENT_DYNAMIC( TestDialog, CDialogEx )

TestDialog::TestDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_TEST_PAN, pParent)
{

}

TestDialog::~TestDialog()
{
}

void TestDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(TestDialog, CDialogEx)
END_MESSAGE_MAP()


// CPaneDialog 消息处理程序
