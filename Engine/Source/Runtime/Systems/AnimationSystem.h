#pragma once

#include <string>
#include <memory>
#include <unordered_map>

namespace Zongine {
    struct ManagerList;

    class EntityManager;
    class ResourceManager;

    class AnimationSystem {
    public:
        bool Initialize(const ManagerList& info);
        void Tick(float fDeltaTime);

    private:
        std::shared_ptr<EntityManager> m_EntityManager{};
        std::shared_ptr<ResourceManager> m_ResourceManager{};

        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>>> m_SkeletonMeshMap{};
    };
}