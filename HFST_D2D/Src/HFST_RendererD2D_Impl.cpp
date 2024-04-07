#include "pch.h"
#include "HFST_RendererD2D_Impl.hpp"

namespace HFST
{
    RendererD2D_Impl::RendererD2D_Impl( CWnd& pWnd )
        : m_pWnd(pWnd) {

    }

    RendererD2D_Impl::~RendererD2D_Impl() {

    }

    void RendererD2D_Impl::DrawAxis(int Count, COLORREF Color)
    {
        CHwndRenderTarget* pRenderTarget = m_pWnd.GetRenderTarget();

        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        //pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::LightCoral));
        pRenderTarget->Clear(D2D1::ColorF(Color));

        CD2DSolidColorBrush black_brush( pRenderTarget, D2D1::ColorF(D2D1::ColorF::Black) );
        CD2DSolidColorBrush blue_brush( pRenderTarget, D2D1::ColorF(D2D1::ColorF::Blue) );
        CD2DSolidColorBrush red_brush( pRenderTarget, D2D1::ColorF(D2D1::ColorF::Red) );

        CD2DSizeF size = pRenderTarget->GetSize();

        pRenderTarget->DrawLine( CD2DPointF(0, size.height / 2 ), CD2DPointF(size.width, size.height/2), &black_brush );
        pRenderTarget->DrawLine( CD2DPointF(size.width/2, 0), CD2DPointF(size.width/2, size.height), &black_brush );

        CD2DPointF ptOld( CD2DPointF(0, size.height / 2) );

        for ( float x = 0.0f; x < size.width; x++ )
        {
            CD2DPointF ptNew( CD2DPointF(x, size.height / 2 + 200.0f * sinf(x / 8)) );

            pRenderTarget->DrawLine(ptOld, ptNew, &blue_brush);

            ptOld = ptNew;
        }

        CD2DTextFormat format( m_pWnd.GetRenderTarget(), _T("Î¢ÈíÑÅºÚ"), 16.0f);
        format.Get()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        format.Get()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

        constexpr int GAP = 10;
        constexpr int WIDTH = 150;

        CD2DRectF recPane(size.width - WIDTH, 2.0f, size.width, 100.0f);
        pRenderTarget->FillRectangle(recPane, &black_brush);

        CD2DRectF recInfo( size.width - WIDTH, GAP, size.width, 30 );

        CD2DSizeF sizef = pRenderTarget->GetDpi();

        CString str;
        str.Format( _T("[%d - %d : %d ]"), Count, (int)sizef.width, (int)sizef.height );
        
        pRenderTarget->DrawTextW(
            str,
            recInfo,
            &red_brush,
            &format
            );

    }
}
