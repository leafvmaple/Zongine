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
        void Initialize(WindowManagerDesc& desc);
        void HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

        HWND GetWindowHandle() const { return m_Wnd; }
        void AddEventCallback(EventCallback callback) {
            m_EventCallbacks.push_back(callback);
        }

    private:
        HWND m_Wnd{};
        std::vector<EventCallback> m_EventCallbacks;
    };

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    extern WindowManager GWindowManager;
}