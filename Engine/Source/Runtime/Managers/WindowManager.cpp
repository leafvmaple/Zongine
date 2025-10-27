#include "WindowManager.h"

namespace Zongine {
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        WindowManager* pThis = reinterpret_cast<WindowManager*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (pThis) {
            pThis->OnMessageEvent(hWnd, message, wParam, lParam);
        }
        if (message == WM_CLOSE) {
            DestroyWindow(hWnd);
            return 0;
        }
        if (message == WM_DESTROY) {
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    void WindowManager::RegisterWindowClass() {
        if (m_WindowClassRegistered) {
            return;
        }

        WNDCLASSEX wndClassEx{};
        wndClassEx.cbSize = sizeof(WNDCLASSEX);
        wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
        wndClassEx.lpfnWndProc = WndProc;
        wndClassEx.hInstance = m_hInstance;
        wndClassEx.hIcon = ::LoadIcon(NULL, IDI_WINLOGO);
        wndClassEx.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wndClassEx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wndClassEx.lpszClassName = L"Zongine";

        RegisterClassEx(&wndClassEx);
        m_WindowClassRegistered = true;
    }

    HWND WindowManager::CreateGameWindow(HINSTANCE hInstance, const wchar_t* title, int width, int height) {
        m_hInstance = hInstance;
        m_Width = width;
        m_Height = height;

        RegisterWindowClass();

        m_Wnd = CreateWindow(
            L"Zongine", 
            title, 
            WS_OVERLAPPEDWINDOW, 
            CW_USEDEFAULT, CW_USEDEFAULT, 
            width, height, 
            NULL, NULL, 
            hInstance, 
            NULL
        );

        // Set WindowManager instance as window user data before showing
        SetWindowLongPtr(m_Wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        ShowWindow(m_Wnd, SW_SHOW);
        UpdateWindow(m_Wnd);

        return m_Wnd;
    }

    void WindowManager::Initialize(HWND wnd) {
        RECT rect{};

        m_Wnd = wnd;

        ::GetClientRect(m_Wnd, &rect);

        // m_Width = rect.right - rect.left;
        // m_Height = rect.bottom - rect.top;

        m_Width = 1182;
        m_Height = 852;

        SetWindowLongPtr(m_Wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        ShowWindow(m_Wnd, SW_SHOW);
    }

    void WindowManager::Resize(int nWidth, int nHeight) {
        m_Width = nWidth;
        m_Height = nHeight;
    };

    void WindowManager::OnMessageEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        for (auto& callback : m_EventCallbacks) {
            callback({ hWnd, message, wParam, lParam });
        }
    }
}