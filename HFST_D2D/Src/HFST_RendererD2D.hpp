#ifndef __HFST_RENDERER_D2D_HPP__
#define __HFST_RENDERER_D2D_HPP__
#include "Core.hpp"

class CWnd;

namespace HFST
{
    class HFST_D2D_EXPORT RendererD2D
    {
    public:
        RendererD2D();
        ~RendererD2D();
        void Initialize( CWnd* pView );
    private:
        //class RendererD2D_Impl;
    };
}


#endif //__HFST_RENDERER_D2D_HPP__
