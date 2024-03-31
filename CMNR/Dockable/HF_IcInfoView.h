#ifndef __HF_IC_INFO_VIEW_H__
#define __HF_IC_INFO_VIEW_H__

#include <afxdockablepane.h>
#include "HF_IcInfoDlg.h"

class HF_IcInfoView : public CDockablePane
{
public:
    HF_IcInfoView() noexcept;
    virtual ~HF_IcInfoView();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()
private:
    HF_IcInfoDlg m_Dialog;
};

#endif //__HF_IC_INFO_VIEW_H__
