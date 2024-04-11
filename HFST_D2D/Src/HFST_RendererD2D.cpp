#include "pch.h"
#include "HFST_RendererD2D.hpp"
#include "HFST_RendererD2D_Impl.hpp"

namespace HFST
{
    RendererD2D::RendererD2D( CWnd& pWnd )
        : m_Impl{ std::make_unique<RendererD2D_Impl>(pWnd) } {
    }

    RendererD2D::~RendererD2D() {

    }

    void RendererD2D::DrawTpArea(
        const CD2DRectF& rect, D2D1::ColorF bgColor, D2D1::ColorF gridColor, float gap)
    {
        m_Impl->DrawTpArea( rect, bgColor, gridColor, gap );
    }

    void RendererD2D::DrawBgColor(D2D1::ColorF bgColor)
    {
        m_Impl->DrawBgColor( bgColor );
    }

    void RendererD2D::DrawLine(const CD2DPointF& start, const CD2DPointF& end,
        D2D1::ColorF color, float opacity, float line_width)
    {
        m_Impl->DrawLine(start, end, color, opacity, line_width);
    }

    void RendererD2D::DrawRect(const CD2DRectF& rect, const D2D1::ColorF& line_Color, float line_width)
    {
        m_Impl->DrawRect( rect, line_Color, line_width );
    }

    void RendererD2D::DrawRoundRect(const CD2DRectF& rect,
        const D2D1::ColorF& line_Color, float line_width, float radius)
    {
        m_Impl->DrawRoundRect(rect, line_Color, line_width, radius);
    }

    void RendererD2D::DrawCircle(const CD2DPointF& center, const D2D1::ColorF& color, float radius)
    {
        m_Impl->DrawCircle( center, color, radius );
    }

    void RendererD2D::DrawEllipse(const CD2DPointF& center, const D2D1::ColorF& color, float radiusX, float radiusY)
    {
        m_Impl->DrawEllipse( center, color, radiusX, radiusY );
    }

    void RendererD2D::DrawPoint(const CD2DPointF& pt, const D2D1::ColorF& color, float radius)
    {
        m_Impl->DrawPoint(pt, color, radius);
    }
    void RendererD2D::FillRect(const CD2DRectF& rect, const D2D1::ColorF& color)
    {
        m_Impl->FillRect(rect, color);
    }
    void RendererD2D::FillRoundRect(const CD2DRectF& rect, const D2D1::ColorF& color, float radius)
    {
        m_Impl->FillRoundRect(rect, color, radius);
    }
    void RendererD2D::FillCircle(const CD2DPointF& center, const D2D1::ColorF& color, float radius)
    {
        m_Impl->FillCircle(center, color, radius);
    }
    void RendererD2D::FillEllipse(const CD2DPointF& center, const D2D1::ColorF& color, float radiusX, float radiusY)
    {
        m_Impl->FillEllipse(center, color, radiusX, radiusY);
    }
}
