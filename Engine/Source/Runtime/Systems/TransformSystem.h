#pragma once

#include "Entities/EntityManager.h"

#include <DirectXMath.h>
#include <memory>

namespace Zongine {
    using DirectX::XMMATRIX;

    struct ManagerList;

    class TransformSystem {
    public:
        bool Initialize(const ManagerList& info);

        void Tick(float fDeltaTime);

    private:
        void _UpdateWorldTransformRecursive(Entity entity, const XMMATRIX& parentMatrix);

        std::shared_ptr<EntityManager> m_EntityManager{};
    };
}