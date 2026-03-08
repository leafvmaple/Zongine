// Simple.cpp : Defines the entry point for the application.

#include <windows.h>
#include <cstdio>
#include <iostream>

#include "Runtime/Engine.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    // Create console for debug output
    AllocConsole();
    FILE* pConsole = nullptr;
    freopen_s(&pConsole, "CONOUT$", "w", stdout);
    freopen_s(&pConsole, "CONOUT$", "w", stderr);
    std::cout << "[Zongine] Console initialized" << std::endl;
    
    Zongine::Engine engine;

    // Engine provides unified API - all internal managers are transparent
    engine.CreateAndInitialize(hInstance, L"Zongine", 1182, 852);

    while (engine.IsRunning()) {
        engine.Tick();
    }

    return 0;
}
