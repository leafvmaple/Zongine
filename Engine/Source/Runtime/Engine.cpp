#include "Engine.h"

#include "Managers/Mananger.h"

#include "Systems/RenderSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/CameraSystem.h"
#include "Systems/TransformSystem.h"
#include "Systems/AnimationSystem.h"

#include "Components/CameraComponent.h"
#include "components/MaterialComponent.h"
#include "components/TransformComponent.h"
#include "Components/AnimationComponent.h"

#include "Entities/EntityManager.h"

#include <algorithm>
#include <string>
#include <iterator>

namespace Zongine {
    Engine::Engine() {
    }
    Engine::~Engine() {
    }

    void Engine::Initialize(HWND wnd) {
        m_nLastTime = timeGetTime();

        // Manager initialization
        entityManager = std::make_shared<EntityManager>();
        windowManager = std::make_shared<WindowManager>();
        deviceManager = std::make_shared<DeviceManager>();
        auto assetManager = std::make_shared<AssetManager>();
        auto stateManager = std::make_shared<StateManager>();
        auto effectManager = std::make_shared<EffectManager>();
        eventManager = std::make_shared<EventManager>();

        ManagerList managerList{
            entityManager,
            windowManager,
            deviceManager,
            assetManager,
            stateManager,
            effectManager
        };

        windowManager->Initialize(wnd);
        deviceManager->Initialize({ windowManager });
        assetManager->Initialize({ entityManager, deviceManager, effectManager });
        stateManager->Initialize({ deviceManager });
        effectManager->Initialize({ deviceManager });

        // System initialization
        renderSystem = std::make_unique<RenderSystem>();
        inputSystem = std::make_unique<InputSystem>();
        cameraSystem = std::make_unique<CameraSystem>();
        transformSystem = std::make_unique<TransformSystem>();
        animationSystem = std::make_unique<AnimationSystem>();

        auto& root = entityManager->GetRootEntity();
        root.AddComponent<TransformComponent>(TransformComponent{});

        auto& camera = root.AddChild("Camera");
        camera.AddComponent<CameraComponent>(CameraComponent{});
        auto& cameraTransform = camera.AddComponent<TransformComponent>(TransformComponent{});
        cameraTransform.Position = { 0.0f, 40.0f, -50.0f };

        auto& player = root.AddChild("Player");
        assetManager->LoadModel(player, "data/source/player/F1/部件/Mdl/F1.mdl");
        player.AddComponent<AnimationComponent>(AnimationComponent{ "data/source/player/F1/动作/F1b01ty普通待机01.ani" });

        auto& playerTransform = root.GetComponent<TransformComponent>();
        playerTransform.Position = { 0.0f, 0, 50.0f };

        auto& head = player.AddChild("Head");
        auto& body = player.AddChild("Body");
        auto& hand = player.AddChild("Hand");
        auto& leg = player.AddChild("Leg");
        auto& belt = player.AddChild("Belt");

        assetManager->LoadMesh(head, "data/source/player/F1/部件/F1_0000_head.mesh");
        assetManager->LoadMesh(body, "data/source/player/F1/部件/F1_2206_body.mesh");
        assetManager->LoadMesh(hand, "data/source/player/F1/部件/F1_2206_hand.mesh");
        assetManager->LoadMesh(leg, "data/source/player/F1/部件/F1_2206_leg.mesh");
        assetManager->LoadMesh(belt, "data/source/player/F1/部件/F1_2206_belt.mesh");

        auto& face = head.AddChild("Face");
        auto& hat = head.AddChild("Hat");
        auto& weapon = hand.AddChild("Weapon");

        assetManager->LoadMesh(face, "data/source/player/F1/部件/f1_new_face.Mesh", "s_face");
        assetManager->LoadMesh(hat, "data/source/player/F1/部件/F1_2206_hat.mesh", "s_hat");
        assetManager->LoadMesh(weapon, "data/source/item/weapon/brush/RH_brush_001.Mesh", "s_rh");

        renderSystem->Initialize(managerList);
        inputSystem->Initialize(managerList);
        cameraSystem->Initialize(managerList);
        transformSystem->Initialize(managerList);
        animationSystem->Initialize(managerList);

        // eventManager->Emit("ENTITIY_UPDATE");
    }

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
        cameraSystem->Tick(nDeltaTime);
        renderSystem->Tick(nDeltaTime);

        m_nLastTime = nTime;
    }

    void Engine::Resize(int width, int height) {
        windowManager->Resize(width, height);
        deviceManager->Resize();
    }

    Entity& Engine::GetRootEntity() {
        return entityManager->GetEntity(0);
    }

    Entity& Engine::GetEntity(EntityID id) {
        return entityManager->GetEntity(id);
    }

    void Engine::SubscribeEvent(const std::string& eventName, const std::function<void()>& callback) {
        eventManager->Subscribe(eventName, callback);
    }
}
