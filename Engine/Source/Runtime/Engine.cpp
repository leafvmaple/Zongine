#include "Engine.h"

#include "Managers/WindowManager.h"
#include "Managers/DeviceManager.h"
#include "Managers/StateManager.h"
#include "Managers/AssetManager.h"
#include "Managers/EventManager.h"
#include "Managers/EffectManager.h"

#include "Systems/RenderSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/CameraSystem.h"
#include "Systems/TransformSystem.h"
#include "Systems/AnimationSystem.h"
#include "Systems/PhysicsSystem.h"
#include "Systems/NvFlexSystem.h"
#include "Systems/CharacterControllerSystem.h"

#include "Components/CameraComponent.h"
#include "Components/MaterialComponent.h"
#include "Components/TransformComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/InputComponent.h"
#include "Components/CharacterControllerComponent.h"

#include "Entities/World.h"

#include <algorithm>
#include <string>
#include <iterator>
#include <thread>
#include <chrono>

namespace Zongine {
    Engine::Engine() {
        QueryPerformanceFrequency(&m_Frequency);
        QueryPerformanceCounter(&m_LastTime);
        _CreateManagers();
    }

    Engine::~Engine() {
        _DestroyManagers();
    }

    void Engine::_CreateManagers() {
        // Creation order matters: later managers may depend on earlier ones
        m_EventManager = std::make_unique<EventManager>();
        SingleManager<EventManager>::Register(m_EventManager.get());

        m_WindowManager = std::make_unique<WindowManager>();
        SingleManager<WindowManager>::Register(m_WindowManager.get());

        m_World = std::make_unique<World>();
        SingleManager<World>::Register(m_World.get());

        m_DeviceManager = std::make_unique<DeviceManager>();
        SingleManager<DeviceManager>::Register(m_DeviceManager.get());

        m_StateManager = std::make_unique<StateManager>();
        SingleManager<StateManager>::Register(m_StateManager.get());

        m_EffectManager = std::make_unique<EffectManager>();
        SingleManager<EffectManager>::Register(m_EffectManager.get());

        m_AssetManager = std::make_unique<AssetManager>();
        SingleManager<AssetManager>::Register(m_AssetManager.get());
    }

    void Engine::_DestroyManagers() {
        // Unregister before reset to avoid dangling pointer access
        // Destroy in reverse order of creation
        SingleManager<AssetManager>::Unregister();
        m_AssetManager.reset();

        SingleManager<EffectManager>::Unregister();
        m_EffectManager.reset();

        SingleManager<StateManager>::Unregister();
        m_StateManager.reset();

        SingleManager<DeviceManager>::Unregister();
        m_DeviceManager.reset();

        SingleManager<World>::Unregister();
        m_World.reset();

        SingleManager<WindowManager>::Unregister();
        m_WindowManager.reset();

        SingleManager<EventManager>::Unregister();
        m_EventManager.reset();
    }

    HWND Engine::CreateAndInitialize(HINSTANCE hInstance, const wchar_t* title, int width, int height) {
        HWND hWnd = m_WindowManager->CreateGameWindow(hInstance, title, width, height);
        Initialize(hWnd);
        return hWnd;
    }

    void Engine::Initialize(HWND wnd) {
        QueryPerformanceCounter(&m_LastTime);

        m_WindowManager->Initialize(wnd);
        m_DeviceManager->Initialize();
        m_StateManager->Initialize();
        m_EffectManager->Initialize();

        // System initialization
        renderSystem = std::make_unique<RenderSystem>();
        inputSystem = std::make_unique<InputSystem>();
        cameraSystem = std::make_unique<CameraSystem>();
        transformSystem = std::make_unique<TransformSystem>();
        animationSystem = std::make_unique<AnimationSystem>();
        physicsSystem = std::make_unique<PhysicsSystem>();
        nvFlexSystem = std::make_unique<NvFlexSystem>();
        characterControllerSystem = std::make_unique<CharacterControllerSystem>();

        auto& world = World::GetInstance();
        EntityID root = world.GetRootEntity();
        world.Assign<TransformComponent>(root, TransformComponent{});

        EntityID camera = world.AddChild(root, "Camera");
        EntityID player = world.AddChild(root, "Player");

        m_AssetManager->LoadPlayer(player, "Settings/player.json");

        world.Assign<CameraComponent>(camera, CameraComponent{});
        auto& cameraTransform = world.Assign<TransformComponent>(camera, TransformComponent{});
        cameraTransform.Position = { 0.0f, 50.f, -200.0f };

        inputSystem->Initialize();
        characterControllerSystem->Initialize();
        cameraSystem->Initialize();
        nvFlexSystem->Initialize();
        renderSystem->Initialize();
    }

    void Engine::Uninitialize() {
        nvFlexSystem->Uninitialize();
        m_bRunning = false;
    }

    constexpr float targetFrameTime = 1.0f / 60.0f;

    void Engine::Tick() {
        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);
        float deltaTime = static_cast<float>(now.QuadPart - m_LastTime.QuadPart) / static_cast<float>(m_Frequency.QuadPart);

        inputSystem->Tick(deltaTime);
        
        MSG msg{};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                Exit();
                return;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        characterControllerSystem->Tick(deltaTime);
        animationSystem->Tick(deltaTime);
        transformSystem->Tick(deltaTime);
        physicsSystem->Tick(deltaTime);
        nvFlexSystem->Tick(deltaTime);
        cameraSystem->Tick(deltaTime);
        renderSystem->Tick(deltaTime);

        LARGE_INTEGER afterFrame{};
        QueryPerformanceCounter(&afterFrame);
        float frameProcessTime = static_cast<float>(afterFrame.QuadPart - now.QuadPart) / static_cast<float>(m_Frequency.QuadPart);

        if (frameProcessTime < targetFrameTime) {
            auto sleepMs = static_cast<int>((targetFrameTime - frameProcessTime) * 1000.0f);
            if (sleepMs > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
        }

        m_LastTime = now;
    }

    void Engine::Resize(int width, int height) {
        m_WindowManager->Resize(width, height);
        m_DeviceManager->Resize();
    }

    EntityID Engine::GetRootEntity() {
        return World::GetInstance().GetRootEntity();
    }

    void Engine::SubscribeEvent(const std::string& eventName, const std::function<void()>& callback) {
        m_EventManager->Subscribe(eventName, callback);
    }
}
