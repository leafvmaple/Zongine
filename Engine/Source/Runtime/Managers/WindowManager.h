#pragma once

#include <Windows.h>
#include <functional>
#include <vector>

#include "Mananger.h"

namespace Zongine {
    struct WindowEvent {
        HWND hWnd{};
        UINT message{};
        WPARAM wParam{};
        LPARAM lParam{};
    };

    using EventCallback = std::function<void(const WindowEvent&)>;

    class WindowManager : public SingleManager<WindowManager> {
    public:
        // Create window and register window class internally
        HWND CreateGameWindow(HINSTANCE hInstance, const wchar_t* title, int width, int height);
        
        // Initialize with existing window (for backward compatibility)
        void Initialize(HWND wnd);
        
        void Resize(int nWidth, int nHeight);

        void OnMessageEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

        HWND GetWindowHandle() const { return m_Wnd; }

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }

        void AddEventCallback(EventCallback callback) {
            m_EventCallbacks.push_back(callback);
        }

    private:
        HWND m_Wnd{};
        HINSTANCE m_hInstance{};

        int m_Width{};
        int m_Height{};
        std::vector<EventCallback> m_EventCallbacks;
        
        bool m_WindowClassRegistered = false;
        
        void RegisterWindowClass();
    };

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
}