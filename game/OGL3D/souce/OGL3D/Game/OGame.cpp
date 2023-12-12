#include <OGL3D/Game/OGame.h>
#include <OGL3D/Window/OWindow.h>
#include <Windows.h>


OGame::OGame()
{
    m_display = new OWindow();
}

OGame::~OGame()
{
    delete m_display;
}

void OGame::run()
{
    MSG msg;
    while (m_isRunning)
    {
        if (PeekMessage(&msg, NULL,NULL,NULL, PM_REMOVE)) // PM_REMOVE removes message each time
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        Sleep(1);
    }
    
}

void OGame::quit()
{
    m_isRunning = false;
}
