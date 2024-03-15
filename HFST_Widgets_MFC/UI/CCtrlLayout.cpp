#include "pch.h"
#include "CCtrlLayout.h"

namespace HFST
{
    CCtrlLayout::CCtrlLayout()
        : m_ptStart(CPoint(0,0))
        , m_ptCurPos(CPoint(0, 0))
        , m_nHeight(0)
        , m_nGap(0) {
    
    }
    CCtrlLayout::CCtrlLayout(const CPoint& start, int CtrlHeight, int Gap)
        : m_ptStart(start)
        , m_nHeight(CtrlHeight)
        , m_nGap(Gap)
        , m_ptCurPos(CPoint(0, 0)) {
    }

    void CCtrlLayout::Add(CWnd& wnd, int CtrlWidth, int nShow)
    {
        wnd.SetWindowPos(nullptr, m_ptStart.x + m_ptCurPos.x, m_ptStart.y + m_ptCurPos.y, CtrlWidth, m_nHeight, nShow);

        m_ptCurPos.x += (m_nGap + CtrlWidth);
    }
}

