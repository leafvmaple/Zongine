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

#include "Components/CameraComponent.h"
#include "components/MaterialComponent.h"
#include "components/TransformComponent.h"
#include "Components/AnimationComponent.h"

#include "Entities/EntityManager.h"

#include <algorithm>
#include <string>
#include <iterator>
#include <thread>
#include <chrono>

namespace Zongine {
    Engine::Engine() {
    }
    Engine::~Engine() {
    }

    void Engine::Initialize(HWND wnd) {
        m_nLastTime = timeGetTime();

        auto& assetManager = AssetManager::GetInstance();

        WindowManager::GetInstance().Initialize(wnd);
        DeviceManager::GetInstance().Initialize();
        StateManager::GetInstance().Initialize();
        EffectManager::GetInstance().Initialize();

        // System initialization
        renderSystem = std::make_unique<RenderSystem>();
        inputSystem = std::make_unique<InputSystem>();
        cameraSystem = std::make_unique<CameraSystem>();
        transformSystem = std::make_unique<TransformSystem>();
        animationSystem = std::make_unique<AnimationSystem>();
        physicsSystem = std::make_unique<PhysicsSystem>();
        nvFlexSystem = std::make_unique<NvFlexSystem>();

        auto& root = EntityManager::GetInstance().GetRootEntity();
        assetManager.LoadScene(root, "data/source/maps/稻香村/稻香村.jsonmap");
        root.AddComponent<TransformComponent>(TransformComponent{});

        auto& camera = root.AddChild("Camera");
        auto& player = root.AddChild("Player");

        camera.AddComponent<CameraComponent>(CameraComponent{});
        auto& cameraTransform = camera.AddComponent<TransformComponent>(TransformComponent{});
        cameraTransform.Position = { 0.0f, 50.f, -200.0f };

        assetManager.LoadModel(player, "data/source/player/F1/部件/Mdl/F1.mdl");
        player.AddComponent<AnimationComponent>(AnimationComponent{ "data/source/player/F1/动作/F1b02dj打坐b.ani" });

        auto& playerTransform = player.GetComponent<TransformComponent>();
        playerTransform.Position = { 0.0f, 0, 50.0f };

        auto& head = player.AddChild("Head");
        auto& body = player.AddChild("Body");
        //auto& hand = player.AddChild("Hand");
        //auto& leg = player.AddChild("Leg");
        //auto& belt = player.AddChild("Belt");

        assetManager.LoadMesh(head, "data/source/player/F1/部件/F1_5407h_a_head.mesh");
        assetManager.LoadMesh(body, "data/source/player/F1/部件/F1_5407h_body.mesh");
        //assetManager.LoadMesh(hand, "data/source/player/F1/部件/F1_2206_hand.mesh");
        //assetManager.LoadMesh(leg, "data/source/player/F1/部件/F1_2206_leg.mesh");
        //assetManager.LoadMesh(belt, "data/source/player/F1/部件/F1_2206_belt.mesh");

        auto& face = head.AddChild("Face");
        //auto& hat = head.AddChild("Hat");
        //auto& weapon = hand.AddChild("Weapon");

        assetManager.LoadMesh(face, "data/source/player/F1/部件/f1_new_face.Mesh", "s_face");
        //assetManager.LoadMesh(hat, "data/source/player/F1/部件/F1_2206_hat.mesh", "s_hat");
        //assetManager.LoadMesh(weapon, "data/source/item/weapon/brush/RH_brush_001.Mesh", "s_rh");

        inputSystem->Initialize();
        cameraSystem->Initialize();
        nvFlexSystem->Initialize();
        renderSystem->Initialize();

        // eventManager->Emit("ENTITIY_UPDATE");
    }

    void Engine::Uninitialize() {
        nvFlexSystem->Uninitialize();
        m_bRunning = false;
    }

    constexpr uint64_t targetFrameTime = 1000 / 60;

    void Engine::Tick() {
        uint64_t nTime = timeGetTime();
        uint64_t nDeltaTime = nTime - m_nLastTime;

        MSG msg{};
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        inputSystem->Tick(nDeltaTime);
        animationSystem->Tick(nDeltaTime);
        transformSystem->Tick(nDeltaTime);
        physicsSystem->Tick(nDeltaTime);
        nvFlexSystem->Tick(nDeltaTime);
        cameraSystem->Tick(nDeltaTime);
        renderSystem->Tick(nDeltaTime);

        uint64_t frameProcessTime = timeGetTime() - nTime;

        if (frameProcessTime < targetFrameTime) {
            std::this_thread::sleep_for(std::chrono::milliseconds(targetFrameTime - frameProcessTime));
        }

        m_nLastTime = nTime;
    }

    void Engine::Resize(int width, int height) {
        WindowManager::GetInstance().Resize(width, height);
        DeviceManager::GetInstance().Resize();
    }

    Entity& Engine::GetRootEntity() {
        return EntityManager::GetInstance().GetEntity(0);
    }

    Entity& Engine::GetEntity(EntityID id) {
        return EntityManager::GetInstance().GetEntity(id);
    }

    void Engine::SubscribeEvent(const std::string& eventName, const std::function<void()>& callback) {
        EventManager::GetInstance().Subscribe(eventName, callback);
    }
}
