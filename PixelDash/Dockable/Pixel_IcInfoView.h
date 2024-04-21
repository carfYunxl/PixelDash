#ifndef __Pixel_IC_INFO_VIEW_H__
#define __Pixel_IC_INFO_VIEW_H__

#include <afxdockablepane.h>
#include "Pixel_IcInfoDlg.h"

class Pixel_IcInfoView : public CDockablePane
{
public:
    Pixel_IcInfoView() noexcept;
    virtual ~Pixel_IcInfoView();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()
private:
    Pixel_IcInfoDlg m_Dialog;
};

#endif //__Pixel_IC_INFO_VIEW_H__
