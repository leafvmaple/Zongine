#pragma once

#include "Entities/EntityManager.h"

#include <DirectXMath.h>
#include <memory>

namespace Zongine {
    struct ManagerList;

    class AssetManager;

    struct ManagerList;

    class TransformSystem {
    public:
        bool Initialize(const ManagerList& info);

        void Tick(float fDeltaTime);

    private:
        void _UpdateWorldTransformRecursive(Entity entity, const DirectX::XMFLOAT4X4& parentMatrix);

        std::shared_ptr<EntityManager> m_EntityManager{};
        std::shared_ptr<AssetManager> m_ResourceManger{};
    };
}