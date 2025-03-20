#include "Engine.h"

#include "WindowManager.h"
#include "DeviceManager.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "StateManager.h"
#include "EffectManager.h"

#include "Systems/RenderSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/CameraSystem.h"

#include "Components/CameraComponent.h"
#include "components/MaterialComponent.h"
#include "components/TransformComponent.h"

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
        // Manager initialization
        auto entityManager = std::make_shared<EntityManager>();
        auto windowManager = std::make_shared<WindowManager>();
        auto deviceManager = std::make_shared<DeviceManager>();
        auto resourceManager = std::make_shared<ResourceManager>();
        auto shaderManager = std::make_shared<ShaderManager>();
        auto stateManager = std::make_shared<StateManager>();
        auto effectManager = std::make_shared<EffectManager>();

        windowManager->Initialize({ hInstance, L"Zongine", L"Zongine", 0, 0, 1280, 720 });
        deviceManager->Initialize({ windowManager });
        resourceManager->Initialize({ entityManager, deviceManager, effectManager });
        shaderManager->Initialize({ deviceManager });
        stateManager->Initialize({ deviceManager });
        effectManager->Initialize({ deviceManager });

        // System initialization
        renderSystem = std::make_unique<RenderSystem>();
        inputSystem = std::make_unique<InputSystem>();
        cameraSystem = std::make_unique<CameraSystem>();

        auto camera = entityManager->CreateEntity();
        auto cameraComponent = entityManager->AddComponent<CameraComponent>(camera.GetID(), CameraComponent{});

        auto& cameraTransform = entityManager->AddComponent<TransformComponent>(camera.GetID(), TransformComponent{});
        cameraTransform.Position = { 0.0f, 40.0f, -100.0f };

        auto player = entityManager->CreateEntity();

        auto head = player.AddChild();
        auto body = player.AddChild();
        auto hand = player.AddChild();
        auto leg = player.AddChild();
        auto belt = player.AddChild();

        resourceManager->LoadModel(head, "data/source/player/F1/部件/F1_0000_head.mesh");
        resourceManager->LoadModel(body, "data/source/player/F1/部件/F1_2206_body.mesh");
        resourceManager->LoadModel(hand, "data/source/player/F1/部件/F1_2206_hand.mesh");
        resourceManager->LoadModel(leg, "data/source/player/F1/部件/F1_2206_leg.mesh");
        resourceManager->LoadModel(belt, "data/source/player/F1/部件/F1_2206_belt.mesh");

        renderSystem->Initialize({ entityManager, deviceManager, shaderManager, stateManager, effectManager });
        inputSystem->Initialize({ windowManager });
        cameraSystem->Initialize({ entityManager, windowManager, deviceManager });

        m_bRunning = true;
    }

    void Engine::Run() {
        while (m_bRunning) {
            Tick(0.0f);
        }
    }

    void Engine::Tick(float fDeltaTime) {
        MSG msg{};
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        inputSystem->Tick(fDeltaTime);
        cameraSystem->Tick(fDeltaTime);
        renderSystem->Tick(fDeltaTime);
    }
}
