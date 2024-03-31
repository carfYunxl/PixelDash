#ifndef __HF_MACHINE_VIEW_H__
#define __HF_MACHINE_VIEW_H__

#include <afxdockablepane.h>
#include "HF_MachineDlg.h"

namespace HF
{
    class HF_MachineView : public CDockablePane
    {
    public:
        HF_MachineView() noexcept;
        virtual ~HF_MachineView();
    protected:
        afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
        afx_msg void OnSize(UINT nType, int cx, int cy);
        DECLARE_MESSAGE_MAP()
    private:
        HF_MachineDlg m_dlgCtl;

    };

}

#endif //__HF_MACHINE_VIEW_H__
