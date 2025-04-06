#pragma once

#include <Windows.h>
#include <functional>
#include <vector>

#include "Managers/Mananger.h"

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

        int m_Width{};
        int m_Height{};
        std::vector<EventCallback> m_EventCallbacks;
    };

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
}