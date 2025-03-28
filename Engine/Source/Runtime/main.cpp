#include "Engine.h"

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wndClassEx{};

    int width = 1280;
    int height = 720;

    wndClassEx.cbSize = sizeof(WNDCLASSEX);
    wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
    // wndClassEx.lpfnWndProc = WndProc;
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