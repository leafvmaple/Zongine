#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace Zongine {
    struct ManagerList;
    struct SkeletonAsset;
    struct MeshAsset;

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

        void _InitialSkeletonMeshMap(std::vector<int>& map, const SkeletonAsset* skeleton, const MeshAsset* mesh);
    };
}