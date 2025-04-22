#pragma once

#include <memory>
#include <windows.h>
#include <string>
#include <functional>

#include "Include/Types.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace Zongine {
    class Entity;

    class RenderSystem;
    class InputSystem;
    class CameraSystem;
    class TransformSystem;
    class AnimationSystem;
    class PhysicsSystem;
    class NvFlexSystem;

    class Engine {
    public:
        Engine();
        ~Engine();

        void Initialize(HWND wnd);
        void Uninitialize();

        void Tick();
        void Resize(int width, int height);

        bool IsRunning() const { return m_bRunning; }
        void Exit() { m_bRunning = false; };

        Entity& GetRootEntity();
        Entity& GetEntity(EntityID id);

        void SubscribeEvent(const std::string& eventName, const std::function<void()>& callback);

    private:
        std::unique_ptr<RenderSystem> renderSystem{};
        std::unique_ptr<InputSystem> inputSystem{};
        std::unique_ptr<CameraSystem> cameraSystem{};
        std::unique_ptr<TransformSystem> transformSystem{};
        std::unique_ptr<AnimationSystem> animationSystem{};
        std::unique_ptr<PhysicsSystem> physicsSystem{};
        std::unique_ptr<NvFlexSystem> nvFlexSystem{};

        bool m_bRunning{ true };
        uint64_t m_nLastTime{};
    };
}