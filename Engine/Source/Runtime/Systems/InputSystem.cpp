#include "InputSystem.h"

#include "Managers/Mananger.h"

namespace Zongine {
    bool InputSystem::Initialize(const ManagerList& info) {
        info.windowManager->AddEventCallback(std::bind(&InputSystem::OnWindowEvent, this, std::placeholders::_1));

        m_WindowManager = info.windowManager;

        return true;
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