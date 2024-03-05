#ifndef __HFST_DUMMY_WINDOW_HPP__
#define __HFST_DUMMY_WINDOW_HPP__

#include <windows.h>
#include <atlstr.h>

namespace HFST
{
    class DummyWindow
    {
    public:
        DummyWindow();
        ~DummyWindow();

        void Run();

        void SetWndProc(WNDPROC proc);
        void SetGUID(GUID guid);

        HWND GetWnd() const { return m_hWnd; }

    private:
        HWND        m_hWnd;
        WNDCLASSEX  wx;
        WNDPROC     m_WinProc;
        GUID        m_Guid;
    };
}


#endif //__HFST_DUMMY_WINDOW_HPP__

