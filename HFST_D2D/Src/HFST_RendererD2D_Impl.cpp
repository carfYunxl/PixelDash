#include "pch.h"
#include "HFST_RendererD2D_Impl.hpp"

namespace HFST
{
    RendererD2D_Impl::RendererD2D_Impl( CWnd& pWnd )
        : m_pWnd(pWnd) {
        pWnd.EnableD2DSupport();
    }

    RendererD2D_Impl::~RendererD2D_Impl() {

    }

    void RendererD2D_Impl::DrawBgColor( const D2D1::ColorF& bgColor )
    {
        CHwndRenderTarget* pRenderTarget = m_pWnd.GetRenderTarget();

        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        pRenderTarget->Clear( D2D1::ColorF(bgColor) );
    }

    void RendererD2D_Impl::DrawTpArea( 
        const CD2DRectF& rect, 
        const D2D1::ColorF& bgColor, 
        const D2D1::ColorF& gridColor, 
        float gap 
    )
    {
        CHwndRenderTarget* pRenderTarget = m_pWnd.GetRenderTarget();

        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        CD2DSolidColorBrush bgBrush( pRenderTarget, D2D1::ColorF(bgColor) );
        CD2DSolidColorBrush gridBrush( pRenderTarget, D2D1::ColorF(gridColor) );

        pRenderTarget->FillRectangle( rect, &bgBrush );

        float nCenterX = rect.left + (rect.right - rect.left)  / 2.0f ;
        float nCenterY = rect.top  + (rect.bottom - rect.top)  / 2.0f ;

        // Vertical
        for ( float i = nCenterX; i < rect.right; i+=gap )
        {
            CD2DPointF ptStart( i, rect.top     );
            CD2DPointF ptStop ( i, rect.bottom  );

            pRenderTarget->DrawLine( ptStart, ptStop, &gridBrush );
        }

        for ( float i = nCenterX - gap; i > rect.left; i -= gap )
        {
            CD2DPointF ptStart( i, rect.top     );
            CD2DPointF ptStop ( i, rect.bottom  );

            pRenderTarget->DrawLine( ptStart, ptStop, &gridBrush );
        }

        // Horizental
        for ( float i = nCenterY; i < rect.bottom; i += gap )
        {
            CD2DPointF ptStart( rect.left,  i );
            CD2DPointF ptStop ( rect.right, i );

            pRenderTarget->DrawLine(ptStart, ptStop, &gridBrush);
        }

        for ( float i = nCenterY - gap; i > rect.top; i -= gap)
        {
            CD2DPointF ptStart( rect.left,  i );
            CD2DPointF ptStop ( rect.right, i );

            pRenderTarget->DrawLine(ptStart, ptStop, &gridBrush);
        }
    }

    void RendererD2D_Impl::DrawLine(
        const CD2DPointF& start,
        const CD2DPointF& end,
        const D2D1::Matrix3x2F& transform,
        const D2D1::ColorF& color, 
        float opacity, 
        float line_width
    )
    {
        CHwndRenderTarget* pRenderTarget = m_pWnd.GetRenderTarget();

        pRenderTarget->SetTransform(transform);

        CD2DBrushProperties properties(opacity);

        CD2DSolidColorBrush brush(pRenderTarget, D2D1::ColorF(color), &properties );

        pRenderTarget->DrawLine(start, end, &brush, line_width);
    }

    void RendererD2D_Impl::DrawRect( const CD2DRectF& rect, const D2D1::Matrix3x2F& transform, const D2D1::ColorF& line_Color, float line_width )
    {
        CHwndRenderTarget* pRenderTarget = m_pWnd.GetRenderTarget();

        pRenderTarget->SetTransform(transform);

        CD2DSolidColorBrush brush( pRenderTarget, D2D1::ColorF(line_Color) );
        pRenderTarget->DrawRectangle( rect, &brush, line_width );
    }

    void RendererD2D_Impl::DrawRoundRect(const CD2DRectF& rect, const D2D1::ColorF& line_Color, float line_width, float radius)
    {
        CHwndRenderTarget* pRenderTarget = m_pWnd.GetRenderTarget();

        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        CD2DSolidColorBrush brush(pRenderTarget, D2D1::ColorF(line_Color));

        CD2DRoundedRect rectRound(rect, CD2DSizeF(radius, radius));
        pRenderTarget->DrawRoundedRectangle( rectRound, &brush, line_width );
    }

    void RendererD2D_Impl::DrawCircle( const CD2DPointF& center, const D2D1::ColorF& color, float radius)
    {
        CHwndRenderTarget* pRenderTarget = m_pWnd.GetRenderTarget();
        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        CD2DSolidColorBrush brush(pRenderTarget, D2D1::ColorF(color));

        CD2DEllipse ellipse(center, CD2DSizeF(radius, radius));

        pRenderTarget->DrawEllipse( ellipse, &brush );
    }

    void RendererD2D_Impl::DrawEllipse(const CD2DPointF& center, const D2D1::ColorF& color, float radiusX, float radiusY)
    {
        CHwndRenderTarget* pRenderTarget = m_pWnd.GetRenderTarget();
        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        CD2DSolidColorBrush brush(pRenderTarget, D2D1::ColorF(color));

        CD2DEllipse ellipse( center, CD2DSizeF( radiusX, radiusY ) );

        pRenderTarget->DrawEllipse( ellipse, &brush );
    }

    void RendererD2D_Impl::FillRect(const CD2DRectF& rect, const D2D1::ColorF& color)
    {
        CHwndRenderTarget* pRenderTarget = m_pWnd.GetRenderTarget();

        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        CD2DSolidColorBrush brush(pRenderTarget, D2D1::ColorF(color));
        pRenderTarget->FillRectangle(rect, &brush);
    }

    void RendererD2D_Impl::FillRoundRect(const CD2DRectF& rect, const D2D1::ColorF& color, float radius)
    {
        CHwndRenderTarget* pRenderTarget = m_pWnd.GetRenderTarget();

        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        CD2DSolidColorBrush brush(pRenderTarget, D2D1::ColorF(color));

        CD2DRoundedRect rectRound(rect, CD2DSizeF(radius, radius));
        pRenderTarget->FillRoundedRectangle(rectRound, &brush);
    }

    void RendererD2D_Impl::FillCircle(const CD2DPointF& center, const D2D1::ColorF& color, float radius)
    {
        CHwndRenderTarget* pRenderTarget = m_pWnd.GetRenderTarget();
        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        CD2DSolidColorBrush brush(pRenderTarget, D2D1::ColorF(color));

        CD2DEllipse ellipse(center, CD2DSizeF(radius, radius));

        pRenderTarget->FillEllipse(ellipse, &brush);
    }

    void RendererD2D_Impl::FillEllipse(const CD2DPointF& center, const D2D1::ColorF& color, float radiusX, float radiusY)
    {
        CHwndRenderTarget* pRenderTarget = m_pWnd.GetRenderTarget();
        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        CD2DSolidColorBrush brush(pRenderTarget, D2D1::ColorF(color));

        CD2DEllipse ellipse(center, CD2DSizeF(radiusX, radiusY));

        pRenderTarget->FillEllipse(ellipse, &brush);
    }

    void RendererD2D_Impl::DrawPoint(const CD2DPointF& pt, const D2D1::ColorF& color, float radius)
    {
        FillCircle(pt, color, radius);
    }


}
