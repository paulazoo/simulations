#include <OGL3D/Window/OWindow.h>
#include <Windows.h>
#include <assert.h>

OWindow::OWindow()
{
    WINDCLASSEX wc = {};
    wc.cbSize = sizeof(WINDCLASSEX);
    wc.lpszClassName = L"OGL3DWindow";
    wc.lpfcWndProc = DefWindowProc;

    assert(RegisterClassEx(&wc)); // assert to check for any errors

    RECT rc = {0,0,1024,768};
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, false);

    m_handle = CreateWindowEx(NULL, L"OGL3DWindow", L"An OpenGL 3D game", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, \
    CW_USEDEFAULT, CW_USEDEFAULT, rc.rigth - rc.left, rc.bottom - rc.top, NULL,NULL,NULL,NULL) // let user and display decide window size
    // width = right - left
    // height = bottom - top
    assert(m_handle); // check m_handle for errors too

    ShowWindow((HWMD)m_handle, SW_SHOW);
    UpdateWindow((HWMD)m_handle);
}

OWindow::~OWindow()
{
    DestroyWindow((HWMD)m_handle)
}