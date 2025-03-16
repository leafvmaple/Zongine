#pragma once

#include <memory>
#include <windows.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace Zongine {
    class RenderSystem;
    class InputSystem;
    class CameraSystem;

    class Engine {
    public:
        Engine();
        ~Engine();

        void Initialize(HINSTANCE hInstance);
        void Run();

    private:
        std::unique_ptr<RenderSystem> renderSystem{};
        std::unique_ptr<InputSystem> inputSystem{};
        std::unique_ptr<CameraSystem> cameraSystem{};
    };
}