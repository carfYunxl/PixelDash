#pragma once
#include <afxdockablepane.h>
#include "CTestDialog.h"

class TestPanel : public CDockablePane
{
public:
    TestPanel() noexcept;
    virtual ~TestPanel();
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()
private:
    TestDialog m_Dialog;

};
