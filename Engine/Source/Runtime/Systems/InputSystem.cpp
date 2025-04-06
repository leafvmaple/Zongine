#include "InputSystem.h"

#include "Managers/WindowManager.h"

namespace Zongine {
    bool InputSystem::Initialize() {
        WindowManager::GetInstance().AddEventCallback(std::bind(&InputSystem::OnWindowEvent, this, std::placeholders::_1));

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
                DestroyWindow(WindowManager::GetInstance().GetWindowHandle());
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