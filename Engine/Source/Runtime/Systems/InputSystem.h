#pragma once

#include <d3d11.h>
#include "Include/Types.h"

namespace Zongine {
    struct WindowEvent;

    /**
     * Input System - Collects input events and updates InputComponent
     * 
     * Workflow:
     * 1. OnWindowEvent collects Windows messages
     * 2. Tick updates global InputComponent
     * 3. Clear transient events at end of frame
     */
    class InputSystem {
    public:
        bool Initialize();
        void Tick(float fDeltaTime);

        void OnWindowEvent(const WindowEvent& event);

        // Get or create global input entity
        EntityID GetInputEntity();

    private:
        EntityID m_InputEntity = 0;
    };
}