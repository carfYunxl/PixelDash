#ifndef __HFST_RENDERER_D2D_IMPL_HPP__
#define __HFST_RENDERER_D2D_IMPL_HPP__

#include "Core/Core.hpp"

namespace Pixel
{
    class D2D_EXPORT RendererD2D_Impl
    {
    public:
        explicit RendererD2D_Impl( CWnd& pWnd );
        ~RendererD2D_Impl();

        void DrawTpArea(const CD2DRectF& rect, const D2D1::ColorF& bgColor, const D2D1::ColorF& gridColor, float gap);
        void DrawBgColor(const D2D1::ColorF& bgColor);

        void DrawPoint( const CD2DPointF& pt, const D2D1::ColorF& color, float radius );
        void DrawLine( const CD2DPointF& start, const CD2DPointF& end, const D2D1::Matrix3x2F& transform, const D2D1::ColorF& color, float opacity, float line_width);
        void DrawRect( const CD2DRectF& rect, const D2D1::Matrix3x2F& transform, const D2D1::ColorF& line_Color, float line_width );
        void DrawRoundRect( const CD2DRectF& rect, const D2D1::ColorF& line_Color, float line_width, float radius);
        void DrawCircle(const CD2DPointF& center, const D2D1::ColorF& color, float radius);
        void DrawEllipse(const CD2DPointF& center, const D2D1::ColorF& color, float radiusX, float radiusY);

        void FillRect(const CD2DRectF& rect, const D2D1::ColorF& color);
        void FillRoundRect(const CD2DRectF& rect, const D2D1::ColorF& color, float radius);
        void FillCircle(const CD2DPointF& center, const D2D1::ColorF& color, float radius);
        void FillEllipse(const CD2DPointF& center, const D2D1::ColorF& color, float radiusX, float radiusY);

        void DrawD2DText(const CString& text, const CD2DRectF& rect);
    private:
        CWnd& m_pWnd;
    };
}
#endif //__HFST_RENDERER_D2D_IMPL_HPP__
