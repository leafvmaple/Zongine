#pragma once

#include "Include/Enums.h"

#include "Components/MeshComponent.h"
#include "Components/ShaderComponent.h"
#include "Components/MaterialComponent.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <array>
#include <wrl/client.h>

struct MESH_SOURCE;

namespace Zongine {
    class Entity;
    class EntityManager;
    class DeviceManager;
    class EffectManager;

    struct SubMesh;

    struct ResourceManagerInfo {
        std::shared_ptr<EntityManager> entityManager{};
        std::shared_ptr<DeviceManager> windowManager{};
        std::shared_ptr<EffectManager> effectManager{};
    };

    class ResourceManager {
    public:
        void Initialize(const ResourceManagerInfo& info) {
            m_DeviceManager = info.windowManager;
            m_EntityManager = info.entityManager;
            m_EffectManager = info.effectManager;
        };

        MeshComponent LoadMesh(const std::string& path);
        MaterialComponent LoadMaterial(const std::string& path);
        ShaderComponent LoadShader(RUNTIME_MACRO macro, const std::vector<std::string>& paths);

    private:
        std::shared_ptr<ReferMaterial> _LoadReferMaterial(const std::string& path);
        ComPtr<ID3D11ShaderResourceView> _LoadTexture(const std::string& path);
        void _LoadConstantBuffer(ID3DX11EffectConstantBuffer*& effectBuffer, ComPtr<ID3DX11Effect> effect, const char* szBuffer);

        bool _CreateVertexBuffer(MeshComponent& mesh, const MESH_SOURCE& source);
        template<typename T>
        bool _CreateIndexBuffer(MeshComponent& mesh, const MESH_SOURCE& source, DXGI_FORMAT eFormat);

        std::shared_ptr<DeviceManager> m_DeviceManager{};
        std::shared_ptr<EntityManager> m_EntityManager{};
        std::shared_ptr<EffectManager> m_EffectManager{};

        // Cache
        std::unordered_map<std::string, MeshComponent> m_MeshComponents{};
        std::unordered_map<std::string, MaterialComponent> m_MaterialComponents{};
        std::array<std::unordered_map<std::string, SubsetShader>, RUNTIME_MACRO_COUNT> m_SubsetShaderCache{};

        std::unordered_map<std::string, std::shared_ptr<ReferMaterial>> m_ReferMaterial{};
        std::unordered_map<std::string, ComPtr<ID3D11ShaderResourceView>> m_Texture{};
    };
}