#include "pch.h"
#include "HFST_RendererD2D.hpp"
#include "HFST_RendererD2D_Impl.hpp"

namespace HFST
{
    RendererD2D::RendererD2D( CWnd& pWnd )
        : m_Impl{ std::make_unique<RendererD2D_Impl>(pWnd) }
    {
    }

    RendererD2D::~RendererD2D()
    {

    }

    void RendererD2D::Initialize(int Count, COLORREF Color)
    {
        m_Impl->DrawAxis(Count, Color);
    }
}
