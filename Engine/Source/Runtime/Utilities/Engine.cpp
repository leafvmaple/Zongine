#include "Engine.h"

#include "ResourceManager.h"
#include "WindowManager.h"

#include "Entities/EntityManager.h"

namespace Zongine {
    void Engine::Initialize(HINSTANCE hInstance) {
        WindowManagerDesc desc{
            
        };

        renderSystem = std::make_unique<RenderSystem>();

        renderSystem->Initialize({});

        GResourceManager.Initialize({ renderSystem->GetDevice() });
        GWindowManager.Initialize(desc);

        auto entity = GEntityManager.CreateEntity();
        GResourceManager.CreateMeshComponent(entity, "data/source/player/F1/²¿¼þ/F1_2206_body.mesh");
    }

    void Engine::Run() {
        renderSystem->Tick(0.0f);
    }
}
