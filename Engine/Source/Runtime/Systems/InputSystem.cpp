#include "InputSystem.h"

#include "Utilities/WindowManager.h"

namespace Zongine {
    void InputSystem::Initialize(const InputSystemInitInfo& info) {
        info.windowManager->AddEventCallback(std::bind(&InputSystem::OnWindowEvent, this, std::placeholders::_1));

        m_WindowManager = info.windowManager;
    }
    void InputSystem::Tick(float fDeltaTime) {
    }

    void InputSystem::OnWindowEvent(const WindowEvent& event) {
        switch (event.message)
        {
        case WM_KEYUP:
        {
            switch (event.wParam)
            {
            case VK_ESCAPE:
                DestroyWindow(m_WindowManager->GetWindowHandle());
                break;
            default:
                break;
            }

            break;
        }
        default:
            break;
        }
    }
}