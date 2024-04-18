#ifndef __CCTRL_LAYOUT_HPP__
#define __CCTRL_LAYOUT_HPP__

namespace HFST
{
    class CCtrlLayout
    {
    public:
        CCtrlLayout();
        CCtrlLayout(const CPoint& start, int CtrlHeight, int Gap);

        void Add(CWnd& wnd, int CtrlWidth, int nShow);
        void NextLine();

        void SetStartPoint(const CPoint& start);

        void SetCtrlHeight(int height){
            m_nHeight = height;
        }
        void SetCurrentPos(const CPoint& pos) {
            m_ptCurPos = pos;
        }

        void SetGap(int gap);

        void SetIndentOffset(int nIndentOffset);

        int GetIndentOffset() const {
            return m_nIndentOffset;
        }

        CPoint& GetCurrentPos() {
            return m_ptCurPos;
        }

        const float GetScale() const {
            return m_Scale;
        }
    private:
        void GetSystemScaleFactor();

    private:
        CPoint  m_ptStart;
        CPoint  m_ptCurPos;
        int     m_nHeight;
        int     m_nGap;

        int     m_nIndentOffset{ 0 };   // �������ʼ�� x λ�õ�ƫ������������ø�ֵ�����ж���ƫ�Ƹ�ֵ

        float   m_Scale{1.0f};
    };
}

#endif //__CCTRL_LAYOUT_HPP__
