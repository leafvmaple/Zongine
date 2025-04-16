// Simple.cpp : Defines the entry point for the application.

#include <windows.h>

#include "Runtime/Engine.h"

// 需要实现窗口过程函数
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    WNDCLASSEX wndClassEx{};

    int width = 1182;
    int height = 852;

    wndClassEx.cbSize = sizeof(WNDCLASSEX);
    wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
    wndClassEx.lpfnWndProc = WndProc;
    wndClassEx.hInstance = hInstance;
    wndClassEx.hIcon = ::LoadIcon(NULL, IDI_WINLOGO);
    wndClassEx.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wndClassEx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClassEx.lpszClassName = L"Zongine";

    RegisterClassEx(&wndClassEx);
    auto hWnd = CreateWindow(L"Zongine", L"Zongine", WS_OVERLAPPEDWINDOW, 0, 0, width, height, NULL, NULL, hInstance, NULL);

    Zongine::Engine engine;
    engine.Initialize(hWnd);

    while (true) {
        engine.Tick();
    }
}
