#include "pch.h"
#include "HFST_RendererD2D.hpp"

namespace HFST
{
    RendererD2D::RendererD2D()
    {
    }

    RendererD2D::~RendererD2D()
    {

    }

    void RendererD2D::Initialize(CWnd* pView )
    {
        CHwndRenderTarget* pRenderTarget = pView->GetRenderTarget();

        CD2DSolidColorBrush brush( pRenderTarget, D2D1::ColorF(D2D1::ColorF::Red) );

        pRenderTarget->SetTransform( D2D1::Matrix3x2F::Identity() );

        pRenderTarget->Clear( D2D1::ColorF(D2D1::ColorF::White) );

        pRenderTarget->DrawLine( CD2DPointF(100.0f, 100.0f), CD2DPointF(200.0f, 200.0f), &brush );


        pRenderTarget->FillRectangle( CD2DRectF(220.0f, 220.0f,400.0f,400.0f), &brush );
    }
}
