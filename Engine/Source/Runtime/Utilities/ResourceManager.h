#pragma once

#include "Components/MeshComponent.h"

#include <memory>
#include <string>
#include <wrl/client.h>

struct MESH_SOURCE;

namespace Zongine {
    class Entity;
    class EntityManager;
    class DeviceManager;

    struct SubMesh;

    struct ResourceManagerInfo {
        std::shared_ptr<EntityManager> entityManager{};
        std::shared_ptr<DeviceManager> windowManager{};
    };

    class ResourceManager {
    public:
        void Initialize(const ResourceManagerInfo& info) {
            m_WindowManager = info.windowManager;
			m_EntityManager = info.entityManager;
        };

        void CreateMeshComponent(const Entity& entity, const std::string& path);

    private:
        bool _CreateVertexBuffer(SubMesh& mesh, const MESH_SOURCE& source);
        template<typename T>
        bool _CreateIndexBuffer(SubMesh& mesh, const MESH_SOURCE& source, DXGI_FORMAT eFormat);

        std::shared_ptr<DeviceManager> m_WindowManager{};
        std::shared_ptr<EntityManager> m_EntityManager{};
    };
}