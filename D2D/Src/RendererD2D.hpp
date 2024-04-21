#ifndef __HFST_RENDERER_D2D_HPP__
#define __HFST_RENDERER_D2D_HPP__

#include "Core.hpp"

class CWnd;

namespace Pixel
{
    class RendererD2D_Impl;

    class D2D_EXPORT RendererD2D
    {
    public:
        RendererD2D( CWnd& pWnd );
        ~RendererD2D();

        void DrawTpArea(const CD2DRectF& rect, D2D1::ColorF bgColor, D2D1::ColorF gridColor, float gap);
        void DrawBgColor(D2D1::ColorF bgColor);

        void DrawLine(const CD2DPointF& start, const CD2DPointF& end, const D2D1::Matrix3x2F& transform, D2D1::ColorF color, float opacity = 1.0f, float line_width = 1.0f);
        void DrawRect(const CD2DRectF& rect, const D2D1::Matrix3x2F& transform, const D2D1::ColorF& line_Color, float line_width);
        void DrawRoundRect(const CD2DRectF& rect, const D2D1::ColorF& line_Color, float line_width, float radius);
        void DrawCircle(const CD2DPointF& center, const D2D1::ColorF& color, float radius);
        void DrawEllipse(const CD2DPointF& center, const D2D1::ColorF& color, float radiusX, float radiusY);

        void DrawPoint(const CD2DPointF& pt, const D2D1::ColorF& color, float radius);
        void FillRect(const CD2DRectF& rect, const D2D1::ColorF& color);
        void FillRoundRect(const CD2DRectF& rect, const D2D1::ColorF& color, float radius);
        void FillCircle(const CD2DPointF& center, const D2D1::ColorF& color, float radius);
        void FillEllipse(const CD2DPointF& center, const D2D1::ColorF& color, float radiusX, float radiusY);

        void DrawD2DText(const CString& text, const CD2DRectF& rect);
    private:
        std::unique_ptr<RendererD2D_Impl> m_Impl;
    };
}


#endif //__HFST_RENDERER_D2D_HPP__
