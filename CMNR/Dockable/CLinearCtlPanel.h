#pragma once
#include <afxdockablepane.h>
#include "CLinearctlDlg.h"

class LinearCtlPane : public CDockablePane
{
public:
    LinearCtlPane() noexcept;
    virtual ~LinearCtlPane();
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()
private:
    CLinearCtlDlg m_dlgCtl;

};