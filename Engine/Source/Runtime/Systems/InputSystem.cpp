#include "InputSystem.h"

#include "Managers/WindowManager.h"
#include "Entities/EntityManager.h"
#include "Components/InputComponent.h"

namespace Zongine {
    bool InputSystem::Initialize() {
        WindowManager::GetInstance().AddEventCallback(std::bind(&InputSystem::OnWindowEvent, this, std::placeholders::_1));

        // Create global input entity
        GetInputEntity();

        return true;
    }

    void InputSystem::Tick(float fDeltaTime) {
        // Clear transient events at end of frame
        auto inputEntity = GetInputEntity();
        auto& entity = EntityManager::GetInstance().GetEntity(inputEntity);

        if (entity.HasComponent<InputComponent>()) {
            auto& input = entity.GetComponent<InputComponent>();
            input.ClearFrameEvents();
        }
    }

    void InputSystem::OnWindowEvent(const WindowEvent& event) {
        auto inputEntity = GetInputEntity();
        auto& entity = EntityManager::GetInstance().GetEntity(inputEntity);

        if (!entity.HasComponent<InputComponent>()) {
            entity.AddComponent(InputComponent{});
        }

        auto& input = entity.GetComponent<InputComponent>();

        switch (event.message)
        {
        case WM_KEYDOWN:
        {
            int keyCode = static_cast<int>(event.wParam);

            // Record only on first press (ignore repeated messages)
            if (input.KeysHeld.find(keyCode) == input.KeysHeld.end()) {
                input.KeysDown.insert(keyCode);
                input.KeysHeld.insert(keyCode);
            }
            break;
        }
        case WM_KEYUP:
        {
            int keyCode = static_cast<int>(event.wParam);
            input.KeysUp.insert(keyCode);
            input.KeysHeld.erase(keyCode);

            // Special handling for ESC to exit
            if (keyCode == VK_ESCAPE) {
                DestroyWindow(WindowManager::GetInstance().GetWindowHandle());
            }
            break;
        }
        case WM_MOUSEMOVE:
        {
            int x = LOWORD(event.lParam);
            int y = HIWORD(event.lParam);

            input.MouseDeltaX = x - input.MouseX;
            input.MouseDeltaY = y - input.MouseY;
            input.MouseX = x;
            input.MouseY = y;
            break;
        }
        case WM_LBUTTONDOWN:
        {
            input.LeftMouseDown = true;
            input.KeysDown.insert(VK_LBUTTON);
            input.KeysHeld.insert(VK_LBUTTON);
            break;
        }
        case WM_LBUTTONUP:
        {
            input.LeftMouseDown = false;
            input.KeysUp.insert(VK_LBUTTON);
            input.KeysHeld.erase(VK_LBUTTON);
            break;
        }
        case WM_RBUTTONDOWN:
        {
            input.RightMouseDown = true;
            input.KeysDown.insert(VK_RBUTTON);
            input.KeysHeld.insert(VK_RBUTTON);
            break;
        }
        case WM_RBUTTONUP:
        {
            input.RightMouseDown = false;
            input.KeysUp.insert(VK_RBUTTON);
            input.KeysHeld.erase(VK_RBUTTON);
            break;
        }
        case WM_MBUTTONDOWN:
        {
            input.MiddleMouseDown = true;
            input.KeysDown.insert(VK_MBUTTON);
            input.KeysHeld.insert(VK_MBUTTON);
            break;
        }
        case WM_MBUTTONUP:
        {
            input.MiddleMouseDown = false;
            input.KeysUp.insert(VK_MBUTTON);
            input.KeysHeld.erase(VK_MBUTTON);
            break;
        }
        case WM_MOUSEWHEEL:
        {
            input.MouseWheel = GET_WHEEL_DELTA_WPARAM(event.wParam) / WHEEL_DELTA;
            break;
        }
        default:
            break;
        }
    }

    EntityID InputSystem::GetInputEntity() {
        if (m_InputEntity == 0) {
            auto& inputEntity = EntityManager::GetInstance().CreateEntity("__GlobalInput__");
            m_InputEntity = inputEntity.GetID();
            inputEntity.AddComponent(InputComponent{});
        }
        return m_InputEntity;
    }
}