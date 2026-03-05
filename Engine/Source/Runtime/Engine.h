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
    // Forward declare Managers
    class WindowManager;
    class DeviceManager;
    class StateManager;
    class EffectManager;
    class AssetManager;
    class EventManager;
    class World;
    class IEditorBridge;

    // Forward declare Systems
    class RenderSystem;
    class InputSystem;
    class CameraSystem;
    class TransformSystem;
    class AnimationSystem;
    class PhysicsSystem;
    class NvFlexSystem;
    class CharacterControllerSystem;

    class Engine {
    public:
        Engine();
        ~Engine();

        // Unified API: Create window and initialize engine
        HWND CreateAndInitialize(HINSTANCE hInstance, const wchar_t* title, int width, int height);

        // Legacy API: Initialize with existing window (for backward compatibility)
        void Initialize(HWND wnd);
        void Uninitialize();

        void Tick();
        void Resize(int width, int height);

        bool IsRunning() const { return m_bRunning; }
        void Exit() { m_bRunning = false; };

        void SetEditorMode(bool enabled) { m_bEditorMode = enabled; }
        bool IsEditorMode() const { return m_bEditorMode; }

        EntityID GetRootEntity();

        void SubscribeEvent(const std::string& eventName, const std::function<void()>& callback);

        // Editor Bridge -- provides a decoupled interface for Editor UI
        IEditorBridge& GetEditorBridge();

    private:
        // === Managers (Engine owns them, initialization order matters) ===
        std::unique_ptr<WindowManager> m_WindowManager;
        std::unique_ptr<DeviceManager> m_DeviceManager;
        std::unique_ptr<StateManager> m_StateManager;
        std::unique_ptr<EffectManager> m_EffectManager;
        std::unique_ptr<AssetManager> m_AssetManager;
        std::unique_ptr<EventManager> m_EventManager;
        std::unique_ptr<World> m_World;

        // === Systems ===
        std::unique_ptr<RenderSystem> renderSystem{};
        std::unique_ptr<InputSystem> inputSystem{};
        std::unique_ptr<CameraSystem> cameraSystem{};
        std::unique_ptr<TransformSystem> transformSystem{};
        std::unique_ptr<AnimationSystem> animationSystem{};
        std::unique_ptr<PhysicsSystem> physicsSystem{};
        std::unique_ptr<NvFlexSystem> nvFlexSystem{};
        std::unique_ptr<CharacterControllerSystem> characterControllerSystem{};

        bool m_bRunning{ true };
        bool m_bEditorMode{ false };

        LARGE_INTEGER m_LastTime{};
        LARGE_INTEGER m_Frequency{};

        std::unique_ptr<IEditorBridge> m_EditorBridge;

        void _CreateManagers();
        void _DestroyManagers();
    };
}