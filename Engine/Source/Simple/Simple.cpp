// Simple.cpp : Defines the entry point for the application.

#include <windows.h>

#include "Runtime/Engine.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    Zongine::Engine engine;

    // Engine provides unified API - all internal managers are transparent
    engine.CreateAndInitialize(hInstance, L"Zongine", 1182, 852);

    while (engine.IsRunning()) {
        engine.Tick();
    }

    return 0;
}
