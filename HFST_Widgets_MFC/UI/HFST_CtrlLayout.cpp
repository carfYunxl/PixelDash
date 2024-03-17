#include "pch.h"
#include "HFST_CtrlLayout.hpp"

namespace HFST
{
    CtrlLayout::CtrlLayout()
        : m_ptStart(CPoint(0,0))
        , m_ptCurPos(CPoint(0, 0))
        , m_nHeight(0)
        , m_nGap(0) {
    
    }
    CtrlLayout::CtrlLayout(const CPoint& start, int CtrlHeight, int Gap)
        : m_ptStart(start)
        , m_nHeight(CtrlHeight)
        , m_nGap(Gap)
        , m_ptCurPos(CPoint(0, 0)) {
    }

    void CtrlLayout::Add(CWnd& wnd, int CtrlWidth, int nShow)
    {
        wnd.SetWindowPos(nullptr, m_ptStart.x + m_ptCurPos.x, m_ptStart.y + m_ptCurPos.y, CtrlWidth, m_nHeight, nShow);

        m_ptCurPos.x += (m_nGap + CtrlWidth);
    }
}

