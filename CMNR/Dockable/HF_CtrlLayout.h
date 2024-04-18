#ifndef __HF_CTRL_LAYOUT_HPP__
#define __HF_CTRL_LAYOUT_HPP__

namespace HFST
{
    class CCtrlLayout
    {
    public:
        CCtrlLayout();
        CCtrlLayout(const CPoint& start, int CtrlHeight, int Gap);

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

        void SetIndentOffset(int nIndentOffset) {
            m_nIndentOffset = nIndentOffset;
        }

        int GetIndentOffset() const {
            return m_nIndentOffset;
        }

        CPoint& GetCurrentPos() {
            return m_ptCurPos;
        }
    private:
        CPoint  m_ptStart;
        CPoint  m_ptCurPos;
        int     m_nHeight;
        int     m_nGap;

        int     m_nIndentOffset{ 0 };   // 相对于起始点 x 位置的偏移量，如果设置该值，新行都会偏移该值
    };
}

#endif //__HF_CTRL_LAYOUT_HPP__
