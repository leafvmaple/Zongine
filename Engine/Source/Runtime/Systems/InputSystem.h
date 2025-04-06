#pragma once

#include <d3d11.h>

namespace Zongine {
    struct WindowEvent;

    class InputSystem {
    public:
        bool Initialize();
        void Tick(float fDeltaTime);

        void OnWindowEvent(const WindowEvent& event);
    };
}