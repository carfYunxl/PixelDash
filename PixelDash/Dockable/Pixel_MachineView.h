#ifndef __Pixel_MACHINE_VIEW_H__
#define __Pixel_MACHINE_VIEW_H__

#include <afxdockablepane.h>
#include "Pixel_MachineDlg.h"

class Pixel_MachineView : public CDockablePane
{
public:
    Pixel_MachineView() noexcept;
    virtual ~Pixel_MachineView();
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()
private:
    Pixel_MachineDlg m_dlgCtl;

};

#endif //__Pixel_MACHINE_VIEW_H__
