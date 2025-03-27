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

    void Engine::Initialize(HINSTANCE hInstance) {
        m_nLastTime = timeGetTime();

        // Manager initialization
        auto entityManager = std::make_shared<EntityManager>();
        auto windowManager = std::make_shared<WindowManager>();
        auto deviceManager = std::make_shared<DeviceManager>();
        auto resourceManager = std::make_shared<ResourceManager>();
        auto stateManager = std::make_shared<StateManager>();
        auto effectManager = std::make_shared<EffectManager>();

        ManagerList managerList{
            entityManager,
            windowManager,
            deviceManager,
            resourceManager,
            stateManager,
            effectManager
        };

        windowManager->Initialize({ hInstance, L"Zongine", L"Zongine", 0, 0, 1280, 720 });
        deviceManager->Initialize({ windowManager });
        resourceManager->Initialize({ entityManager, deviceManager, effectManager });
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

        auto& camera = root.AddChild();
        camera.AddComponent<CameraComponent>(CameraComponent{});
        auto& cameraTransform = camera.AddComponent<TransformComponent>(TransformComponent{});
        cameraTransform.Position = { 0.0f, 40.0f, -50.0f };

        auto& player = root.AddChild();
        resourceManager->LoadModel(player, "data/source/player/F1/部件/Mdl/F1.mdl");
        player.AddComponent<AnimationComponent>(AnimationComponent{ "data/source/player/F1/动作/F1b01ty普通待机01.ani" });

        auto& playerTransform = root.GetComponent<TransformComponent>();
        playerTransform.Position = { 0.0f, 0, 50.0f };

        auto& head = player.AddChild();
        auto& body = player.AddChild();
        auto& hand = player.AddChild();
        auto& leg = player.AddChild();
        auto& belt = player.AddChild();

        resourceManager->LoadMesh(head, "data/source/player/F1/部件/F1_0000_head.mesh");
        resourceManager->LoadMesh(body, "data/source/player/F1/部件/F1_2206_body.mesh");
        resourceManager->LoadMesh(hand, "data/source/player/F1/部件/F1_2206_hand.mesh");
        resourceManager->LoadMesh(leg, "data/source/player/F1/部件/F1_2206_leg.mesh");
        resourceManager->LoadMesh(belt, "data/source/player/F1/部件/F1_2206_belt.mesh");

        auto& face = head.AddChild();
        auto& hat = head.AddChild();
        auto& weapon = hand.AddChild();

        resourceManager->LoadMesh(face, "data/source/player/F1/部件/f1_new_face.Mesh", "s_face");
        resourceManager->LoadMesh(hat, "data/source/player/F1/部件/F1_2206_hat.mesh", "s_hat");
        resourceManager->LoadMesh(weapon, "data/source/item/weapon/brush/RH_brush_001.Mesh", "s_rh");

        renderSystem->Initialize(managerList);
        inputSystem->Initialize(managerList);
        cameraSystem->Initialize(managerList);
        transformSystem->Initialize(managerList);
        animationSystem->Initialize(managerList);

        m_bRunning = true;
    }

    void Engine::Run() {
        while (m_bRunning) {
            uint64_t nTime = timeGetTime();
            Tick(nTime - m_nLastTime);
            m_nLastTime = nTime;
        }
    }

    void Engine::Tick(uint64_t fDeltaTime) {
        MSG msg{};
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        inputSystem->Tick(fDeltaTime);
        animationSystem->Tick(fDeltaTime);
        transformSystem->Tick(fDeltaTime);
        cameraSystem->Tick(fDeltaTime);
        renderSystem->Tick(fDeltaTime);
    }
}
