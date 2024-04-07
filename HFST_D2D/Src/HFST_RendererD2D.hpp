#ifndef __HFST_RENDERER_D2D_HPP__
#define __HFST_RENDERER_D2D_HPP__

#include "Core.hpp"

class CWnd;

namespace HFST
{
    class RendererD2D_Impl;

    class HFST_D2D_EXPORT RendererD2D
    {
    public:
        RendererD2D( CWnd& pWnd );
        ~RendererD2D();
        void Initialize( int Count, COLORREF Color);
    private:
        std::unique_ptr<RendererD2D_Impl> m_Impl;
    };
}


#endif //__HFST_RENDERER_D2D_HPP__
