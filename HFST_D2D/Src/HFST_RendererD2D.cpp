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
}
