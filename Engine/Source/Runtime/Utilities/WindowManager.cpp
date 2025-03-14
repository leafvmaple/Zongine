#include "WindowManager.h"

namespace Zongine {
    void WindowManager::Initialize(WindowManagerDesc& desc) {
        WNDCLASSEX wndClassEx{};

        wndClassEx.cbSize = sizeof(WNDCLASSEX);
        wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
        wndClassEx.lpfnWndProc = WndProc;
        wndClassEx.hInstance = desc.hInstance;
        wndClassEx.hIcon = ::LoadIcon(NULL, IDI_WINLOGO);
        wndClassEx.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wndClassEx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wndClassEx.lpszClassName = desc.szClassName;

        RegisterClassEx(&wndClassEx);
        m_Wnd = CreateWindow(desc.szClassName, desc.szTitle, WS_OVERLAPPEDWINDOW,
            desc.x, desc.y, desc.width, desc.height, NULL, NULL, desc.hInstance, NULL);

        SetWindowLongPtr(m_Wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        ShowWindow(m_Wnd, SW_SHOW);
    }

    void WindowManager::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        for (auto& callback : m_EventCallbacks) {
            callback({ hWnd, message, wParam, lParam });
        }
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        WindowManager* pThis = reinterpret_cast<WindowManager*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (pThis) {
            pThis->HandleMessage(hWnd, message, wParam, lParam);
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    WindowManager GWindowManager;
}