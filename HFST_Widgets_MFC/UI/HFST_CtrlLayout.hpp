#ifndef __CCTRL_LAYOUT_HPP__
#define __CCTRL_LAYOUT_HPP__

#include <afxwin.h>

namespace HFST
{
    class CtrlLayout
    {
    public:
        CtrlLayout();
        CtrlLayout(const CPoint& start, int CtrlHeight, int Gap);

        void Add(CWnd& wnd, int CtrlWidth, int nShow);
        void NextLine() {
            m_ptCurPos.x = 0;
            m_ptCurPos.y += (m_nGap + m_nHeight);
        }

        void SetStartPoint(const CPoint& start) {
            m_ptStart = start;

            m_ptCurPos = { 0 , 0 };
        }

        void SetCtrlHeight(int height){
            m_nHeight = height;
        }
        void SetCurrentPos(const CPoint& pos) {
            m_ptCurPos = pos;
        }

        void SetGap(int gap){
            m_nGap = gap;
        }

        CPoint& GetCurrentPos() {
            return m_ptCurPos;
        }
    private:
        CPoint  m_ptStart;
        CPoint  m_ptCurPos;
        int     m_nHeight;
        int     m_nGap;
    };
}

#endif //__CCTRL_LAYOUT_HPP__
