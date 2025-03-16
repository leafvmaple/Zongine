#include "Engine.h"

#include "WindowManager.h"
#include "DeviceManager.h"
#include "ResourceManager.h"
#include "ShaderManager.h"
#include "StateManager.h"

#include "Systems/RenderSystem.h"
#include "Systems/InputSystem.h"

#include "Entities/EntityManager.h"

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

        windowManager->Initialize({ hInstance, L"Zongine", L"Zongine", 0, 0, 1280, 720 });
        deviceManager->Initialize({ windowManager });
        resourceManager->Initialize({ entityManager, deviceManager });
        shaderManager->Initialize({ deviceManager });
        stateManager->Initialize({ deviceManager });

        // System initialization
        renderSystem = std::make_unique<RenderSystem>();
        inputSystem = std::make_unique<InputSystem>();

        renderSystem->Initialize({ entityManager, deviceManager, shaderManager, stateManager });
        inputSystem->Initialize({ windowManager });

        auto entity = entityManager->CreateEntity();
        resourceManager->CreateMeshComponent(entity, "data/source/player/F1/²¿¼þ/F1_2206_body.mesh");
    }

    void Engine::Run() {
        renderSystem->Tick(0.0f);
    }
}
