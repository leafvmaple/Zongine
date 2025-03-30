#include "WindowManager.h"

namespace Zongine {
    void WindowManager::Initialize(HWND wnd) {
        RECT rect{};

        m_Wnd = wnd;

        ::GetClientRect(m_Wnd, &rect);

        m_Width = rect.right - rect.left;
        m_Height = rect.bottom - rect.top;

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

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        WindowManager* pThis = reinterpret_cast<WindowManager*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (pThis) {
            pThis->OnMessageEvent(hWnd, message, wParam, lParam);
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}