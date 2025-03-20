#pragma once

#include <Windows.h>
#include <functional>
#include <vector>

namespace Zongine {
    struct WindowEvent {
        HWND hWnd{};
        UINT message{};
        WPARAM wParam{};
        LPARAM lParam{};
    };

    using EventCallback = std::function<void(const WindowEvent&)>;

    struct WindowManagerDesc {
        HINSTANCE hInstance{};
        const wchar_t* szClassName{};
        const wchar_t* szTitle{};
        int x{};
        int y{};
        int width{};
        int height{};
    };

    class WindowManager {
    public:
        void Initialize(const WindowManagerDesc& desc);
        void OnMessageEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

        HWND GetWindowHandle() const { return m_Wnd; }

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        int GetViewportWidth() const { return m_ViewportWidth; }
        int GetViewportHeight() const { return m_ViewportHeight; }

        void AddEventCallback(EventCallback callback) {
            m_EventCallbacks.push_back(callback);
        }

    private:
        HWND m_Wnd{};

        int m_Width{};
        int m_Height{};
        int m_ViewportWidth{};
        int m_ViewportHeight{};
        std::vector<EventCallback> m_EventCallbacks;
    };

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
}