#include "pch.h"
#include "HF_CtrlLayout.h"

namespace HFST
{
    CCtrlLayout::CCtrlLayout()
        : m_ptStart(CPoint(0,0))
        , m_ptCurPos(CPoint(0, 0))
        , m_nHeight(0)
        , m_nGap(0) {
        GetSystemScaleFactor();
    }
    CCtrlLayout::CCtrlLayout(const CPoint& start, int CtrlHeight, int Gap)
        : m_ptStart(start)
        , m_nHeight(CtrlHeight)
        , m_nGap(Gap)
        , m_ptCurPos(CPoint(0, 0)) {
        GetSystemScaleFactor();
    }

    void CCtrlLayout::Add(CWnd& wnd, int CtrlWidth, int nShow)
    {
        int nWidth = static_cast<int>(CtrlWidth * m_Scale);
        int nHeight = static_cast<int>(m_nHeight * m_Scale);

        wnd.SetWindowPos(nullptr, m_ptStart.x + m_nIndentOffset + m_ptCurPos.x, m_ptStart.y + m_ptCurPos.y, nWidth, nHeight, nShow);

        m_ptCurPos.x += (m_nGap + nWidth);
    }

    void CCtrlLayout::SetIndentOffset(int nIndentOffset) 
    {
        m_nIndentOffset = nIndentOffset * m_Scale;
    }

    void CCtrlLayout::NextLine()
    {
        int nHeight = static_cast<int>(m_nHeight * m_Scale);

        m_ptCurPos.x = 0;
        m_ptCurPos.y += (m_nGap + nHeight);
    }

    void CCtrlLayout::SetStartPoint(const CPoint& start)
    {
        m_ptStart = start;
        m_ptCurPos = { 0 , 0 };
    }

    void CCtrlLayout::SetGap(int gap) 
    {
        m_nGap = gap * m_Scale;
    }

    void CCtrlLayout::GetSystemScaleFactor()
    {
        HDC		hdc;

        hdc = ::GetDC(NULL);

        UINT	dpiX = 96;

        dpiX = GetDeviceCaps(hdc, LOGPIXELSX);

        m_Scale = (float)dpiX / 96.0f;
        if (m_Scale < 1.0)
            m_Scale = 1.0;

        ::ReleaseDC(NULL, hdc);
    }
}

