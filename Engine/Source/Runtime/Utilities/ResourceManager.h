#pragma once

#include "Components/MeshComponent.h"

#include <memory>
#include <string>

struct MESH_SOURCE;

namespace Zongine {
    class Entity;
    struct SubMesh;

    struct ResourceInitInfo {
        std::shared_ptr<ID3D11Device> piDevice;
    };

    class ResourceManager {
    public:
        void Initialize(const ResourceInitInfo& initInfo) {
            m_piDevice = initInfo.piDevice;
        }

        void CreateMeshComponent(const Entity& entity, const std::string& path);

    private:
        bool _CreateVertexBuffer(SubMesh& mesh, const MESH_SOURCE& source);
        template<typename T>
        bool _CreateIndexBuffer(SubMesh& mesh, const MESH_SOURCE& source, DXGI_FORMAT eFormat);

        std::shared_ptr<ID3D11Device> m_piDevice{};
    };

    extern ResourceManager GResourceManager;
}