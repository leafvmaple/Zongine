#pragma once

#include "Systems/RenderSystem.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace Zongine {
    class Engine {
    public:
        void Initialize(HINSTANCE hInstance);
        void Run();

    private:
        std::unique_ptr<RenderSystem> renderSystem;
    };
}