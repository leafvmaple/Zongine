#pragma once

#include "Include/Types.h"

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <unordered_map>

using DirectX::XMFLOAT4X4;
using DirectX::XMMATRIX;

namespace Zongine {
    struct ManagerList;
    struct SkeletonAsset;
    struct MeshAsset;
    struct SkeletonComponent;
    struct AnimationComponent;

    class EntityManager;
    class ResourceManager;

    class AnimationSystem {
    public:
        bool Initialize(const ManagerList& info);
        void Tick(int nDeltaTime);

    private:
        std::shared_ptr<EntityManager> m_EntityManager{};
        std::shared_ptr<ResourceManager> m_ResourceManager{};

        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>>> m_SkeletonMeshMap{};

        void _UpdateSkeletonTransform(
            AnimationComponent& component,
            std::shared_ptr<SkeletonAsset> skeleton,
            std::vector<XMFLOAT4X4>& matrices,
            int skeletonIndex
        );

        void _UpdateMeshBonesTransform(EntityID entityID, const SkeletonComponent& skeleton, const AnimationComponent& animation);
    };
}