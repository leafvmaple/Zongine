#pragma once

#include <d3d11.h>
#include <memory>

namespace Zongine {
    struct WindowEvent;
	class WindowManager;

	struct InputSystemInitInfo {
        std::shared_ptr<WindowManager> windowManager{};
	};

    class InputSystem {
    public:
        void Initialize(const InputSystemInitInfo& info);
        void Tick(float fDeltaTime);

        void OnWindowEvent(const WindowEvent& event) {};
    };
}