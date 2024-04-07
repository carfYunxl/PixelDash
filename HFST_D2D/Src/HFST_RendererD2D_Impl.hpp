#ifndef __HFST_RENDERER_D2D_IMPL_HPP__
#define __HFST_RENDERER_D2D_IMPL_HPP__

#include "Core/Core.hpp"

namespace HFST
{
    class HFST_D2D_EXPORT RendererD2D_Impl
    {
    public:
        explicit RendererD2D_Impl( CWnd& pWnd );
        ~RendererD2D_Impl();

        void DrawAxis(int Count, COLORREF Color);
    private:
        CWnd& m_pWnd;
    };
}
#endif //__HFST_RENDERER_D2D_IMPL_HPP__
