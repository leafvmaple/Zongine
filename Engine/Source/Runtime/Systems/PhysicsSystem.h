#pragma once

#include "Managers/Mananger.h"

#include <memory>
#include <vector>
#include <DirectXMath.h>

namespace Zongine {
    class EntityManager;
    class AssetManager;

    struct MeshComponent;
    struct FlexibleComponent;
    struct BONE;
    struct MeshAsset;

    class PhysicsSystem {
    public:
        bool Initialize(const ManagerList& info);

        void Tick(float fDeltaTime);

    private:
        void _InitializeDrivenBones(
            int parentIndex,
            std::vector<uint32_t>& drivenBones,
            const std::vector<BONE>& bones
        );

        std::shared_ptr<EntityManager> m_EntityManager{};
        std::shared_ptr<AssetManager> m_AssetManager{};
    };
}