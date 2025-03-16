#include "InputSystem.h"

#include "Utilities/WindowManager.h"

namespace Zongine {
	void InputSystem::Initialize(const InputSystemInitInfo& info) {
		info.windowManager->AddEventCallback(std::bind(&InputSystem::OnWindowEvent, this, std::placeholders::_1));
	}
	void InputSystem::Tick(float fDeltaTime) {
	}
}