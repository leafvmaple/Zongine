#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

namespace Zongine {
    class EntityManager;

    struct ManagerList;

    class CameraSystem {
    public:
        bool Initialize(const ManagerList& info);
        void Tick(float fDeltaTime);
    private:
        std::shared_ptr<EntityManager> m_EntityManager{};
    };
}