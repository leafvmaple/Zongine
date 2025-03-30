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
    class TransformSystem;
    class AnimationSystem;

    class WindowManager;
    class DeviceManager;

    class Engine {
    public:
        Engine();
        ~Engine();

        void Initialize(HWND wnd);
        void Tick();
        void Resize(int width, int height);

        bool IsRunning() const { return m_bRunning; }

    private:
        std::unique_ptr<RenderSystem> renderSystem{};
        std::unique_ptr<InputSystem> inputSystem{};
        std::unique_ptr<CameraSystem> cameraSystem{};
        std::unique_ptr<TransformSystem> transformSystem{};
        std::unique_ptr<AnimationSystem> animationSystem{};

        std::shared_ptr<WindowManager> windowManager{};
        std::shared_ptr<DeviceManager> deviceManager{};

        bool m_bRunning{ true };
        uint64_t m_nLastTime{};
    };
}