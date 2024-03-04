#include "pch.h"
#include "HFST_DummyWindow.hpp"

namespace HFST
{
#define HWND_MESSAGE     ((HWND)-3)
#define CLS_NAME L"DUMMY_CLASS"

    DummyWindow::DummyWindow()
        : m_hWnd { NULL }
        , m_WinProc{ NULL }
    {}

    DummyWindow::~DummyWindow() {
        DestroyWindow(m_hWnd);
    }

    void DummyWindow::SetWndProc(WNDPROC proc)
    {
        m_WinProc = proc;
    }

    void DummyWindow::SetGUID(GUID guid)
    {
        m_Guid = guid;
    }

    void DummyWindow::Run()
    {
        ZeroMemory(&wx, sizeof(wx));
        wx.cbSize = sizeof(WNDCLASSEX);
        wx.lpfnWndProc = m_WinProc;
        wx.hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(0));
        wx.style = CS_HREDRAW | CS_VREDRAW;
        wx.hInstance = GetModuleHandle(0);
        wx.hbrBackground = (HBRUSH)(COLOR_WINDOW);
        wx.lpszClassName = CLS_NAME;

        GUID guid = m_Guid;

        if (RegisterClassEx(&wx))
        {
            m_hWnd = CreateWindow(
                CLS_NAME, 
                L"DevNotifWnd", 
                WS_ICONIC,
                0,
                0, 
                CW_USEDEFAULT,
                0,
                HWND_MESSAGE,
                NULL, 
                GetModuleHandle(0),
                (void*)&guid
            );
        }
        if (m_hWnd == NULL)
        {
            throw std::runtime_error("Could not create message window!");
        }
        std::cout << "waiting for new devices..\n";

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}
