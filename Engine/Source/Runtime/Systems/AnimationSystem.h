#pragma once

#include "Include/Types.h"

#include <string>
#include <memory>
#include <vector>
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

    class AnimationSystem {
    public:
        void Tick(int nDeltaTime);

    private:
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>>> m_SkeletonMeshMap{};

        void _UpdateSkeletonSpaceRecursive(
            AnimationComponent& component,
            std::shared_ptr<SkeletonAsset> skeleton,
            std::vector<XMFLOAT4X4>& localTransform,
            int skeletonIndex
        );

        void _MapSkeletonTransformsToMesh(EntityID entityID, const SkeletonComponent& skeleton, const AnimationComponent& animation);
    };
}