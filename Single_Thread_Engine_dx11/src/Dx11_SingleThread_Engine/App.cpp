#include "App.h"
#include "common/Input.h"
#include "combaseapi.h"

App::App() 
    : wnd_(800, 600, L"Dx11 Engine Window")
{
    wnd_.ActiveWindow();
}

bool App::DoInit()
{
    Timer_.Init();

    Input::GetInstance().InitDirectInput(wnd_.Get_Instance(), wnd_.Get_Handle(), true, true);
    Input::GetInstance().Init();
    ScreenDebug::GetInstance().Init(this);

    return true;
}
bool App::DoFrame()
{
    Timer_.Frame();
    ScreenDebug::GetInstance().Frame();

    return true;
}
bool App::DoRender()
{
    wnd_.Get_gfx().ClearBuffer();

    ScreenDebug::GetInstance().Render();

    wnd_.Get_gfx().EndFrame();

    return true;
}
bool App::DoRelease()
{
    Timer_.Stop();
    if (!Input::GetInstance().Release()) return false;
    if (!ScreenDebug::GetInstance().Release()) return false;
}

int App::Go()
{
    DoInit();

    while (1) {
        // process all messages pending, but to not block for new msg;
        if (const auto ecode = Window::ProcessMessages()) {
            
            // if return optional has value, means we're quitting, so return exit code
            return *ecode;
        }

        DoFrame();
        DoRender();
    }

    DoRelease();
}


