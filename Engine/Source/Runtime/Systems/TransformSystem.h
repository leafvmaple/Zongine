#pragma once

#include "Entities/EntityManager.h"

#include <DirectXMath.h>
#include <memory>

namespace Zongine {
    using DirectX::XMMATRIX;

    struct TransformSystemDesc {
        std::shared_ptr<EntityManager> entityManager;
    };

    class TransformSystem {
    public:
        bool Initialize(const TransformSystemDesc& info) {
            m_EntityManager = info.entityManager;

            return true;
        }

        void Tick(float fDeltaTime);

    private:
        void _UpdateWorldTransformRecursive(Entity entity, const XMMATRIX& parentMatrix);

        std::shared_ptr<EntityManager> m_EntityManager{};
    };
}