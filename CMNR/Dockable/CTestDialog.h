#pragma once
#include "afxdialogex.h"
class TestDialog : public CDialogEx
{
	DECLARE_DYNAMIC(TestDialog)

public:
	TestDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~TestDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_TEST_PAN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
