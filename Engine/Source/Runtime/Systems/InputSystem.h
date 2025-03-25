#pragma once

#include <d3d11.h>
#include <memory>

namespace Zongine {
    struct WindowEvent;
    struct ManagerList;

    class WindowManager;

    class InputSystem {
    public:
        bool Initialize(const ManagerList& info);
        void Tick(float fDeltaTime);

        void OnWindowEvent(const WindowEvent& event);

    private:
        std::shared_ptr<WindowManager> m_WindowManager{};
    };
}