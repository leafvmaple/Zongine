#pragma once

#include "Include/Enums.h"

#include "Components/MeshComponent.h"
#include "Components/ShaderComponent.h"
#include "Components/MaterialComponent.h"
#include "Components/SkeletonComponent.h"

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

    using Microsoft::WRL::ComPtr;

    struct SubMesh;

    struct SubsetMeshAsset {
        UINT uStartIndex{};
        UINT uIndexCount{};
    };

    struct BONE {
        UINT nBoneIndex{};
        std::string Name{};
        DirectX::XMFLOAT4X4 Offset{};
        std::vector<UINT> Children{};
    };

    struct SOCKET {
        std::string Name{};
        UINT nParentBoneIndex{};
        DirectX::XMFLOAT4X4 Offset{};
    };

    struct MeshAsset {
        std::string Path{};

        struct VERTEX_BUFFER {
            ComPtr<ID3D11Buffer> Buffer{};
            UINT uStride{};
            UINT uOffset{};
        } Vertex;

        struct INDEX_BUFFER {
            ComPtr<ID3D11Buffer> Buffer{};
            DXGI_FORMAT eFormat{ DXGI_FORMAT_UNKNOWN };
            UINT uOffset{};
        } Index;

        std::vector<SubsetMeshAsset> Subsets;
        std::vector<BONE> Bones;
        std::vector<SOCKET> Sockets;

        std::unordered_map<std::string, UINT> BoneMap;
    };

    struct _Texture {
        std::string Name;
        ComPtr<ID3D11ShaderResourceView> Texture;
    };

    struct ReferMaterial {
        std::string Path{};
        std::string ShaderName{};

        RASTERIZER_STATE_ID Rasterizer{};

        std::unordered_map<std::string, _Texture> Textures{};
    };

    struct MaterialAsset {
        std::string Path{};
        std::vector<ReferMaterial> Subsets;
    };

    struct SkeletonBone {
        std::string Name{};
        std::vector<int> Children{};
    };

    struct SkeletonAsset {
        std::string Path;
        std::vector<SkeletonBone> Bones;
    };

    struct SubsetShader {
        std::string ShaderPath{};

        ComPtr<ID3DX11Effect> Effect{};
        std::unordered_map<std::string, ID3DX11EffectShaderResourceVariable*> Variables{};
    };

    struct ShaderAsset {
        std::vector<SubsetShader> Subsets{};

        RUNTIME_MACRO Macro{};
        RENDER_PASS Pass{};

        ComPtr<ID3D11Buffer> ModelBuffer{};

        std::vector<ComPtr<ID3D11Buffer>> SubsetBuffers{};
    };

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

        void LoadModel(Entity& entity, const std::string& path);
        void LoadMesh(Entity& entity, const std::string& path);

        std::shared_ptr<MeshAsset> GetMeshAsset(const std::string& path) {
            auto& mesh = m_MeshCache[path];
            if (!mesh)
                mesh = _LoadMesh(path);
            return mesh;
        }

        std::shared_ptr<MaterialAsset> GetMaterialAsset(const std::string& path) {
            auto& material = m_MaterialCache[path];
            if (!material)
                material = _LoadMaterial(path);
            return material;
        }

        std::shared_ptr<SkeletonAsset> GetSkeletonAsset(const std::string& path) {
            auto& skeleton = m_SkeletonCache[path];
            if (!skeleton)
                skeleton = _LoadSkeleton(path);
            return skeleton;
        }

        std::shared_ptr<ShaderAsset> GetShaderAsset(RUNTIME_MACRO macro, const std::string& path) {
            auto& shader = m_ShaderCache[path];
            if (!shader)
                shader = _LoadShader(macro, path);
            return shader;
        }

    private:
        std::shared_ptr<ReferMaterial> _LoadReferMaterial(const std::string& path);
        ComPtr<ID3D11ShaderResourceView> _LoadTexture(const std::string& path);

        std::shared_ptr<MeshAsset> _LoadMesh(const std::string& path);
        std::shared_ptr<MaterialAsset> _LoadMaterial(const std::string& path);
        std::shared_ptr<SkeletonAsset> _LoadSkeleton(const std::string& path);
        std::shared_ptr<ShaderAsset> _LoadShader(RUNTIME_MACRO macro, const std::string& path);

        bool _LoadBone(MeshAsset* mesh, const MESH_SOURCE& source);
        bool _LoadSocket(MeshAsset* mesh, const MESH_SOURCE& source);
        bool _LoadVertexBuffer(MeshAsset* mesh, const MESH_SOURCE& source);
        bool _LoadIndexBuffer(MeshAsset* mesh, const MESH_SOURCE& source);

        std::shared_ptr<DeviceManager> m_DeviceManager{};
        std::shared_ptr<EntityManager> m_EntityManager{};
        std::shared_ptr<EffectManager> m_EffectManager{};

        // Cache
        std::unordered_map<std::string, std::shared_ptr<MeshAsset>> m_MeshCache{};
        std::unordered_map<std::string, std::shared_ptr<MaterialAsset>> m_MaterialCache{};
        std::unordered_map<std::string, std::shared_ptr<SkeletonAsset>> m_SkeletonCache{};
        std::unordered_map<std::string, std::shared_ptr<ShaderAsset>> m_ShaderCache{};

        std::unordered_map<std::string, MaterialComponent> m_MaterialComponents{};
        std::unordered_map<std::string, SkeletonComponent> m_SkeletonComponents{};
        std::array<std::unordered_map<std::string, SubsetShader>, RUNTIME_MACRO_COUNT> m_SubsetShaderCache{};

        std::unordered_map<std::string, std::shared_ptr<ReferMaterial>> m_ReferMaterial{};
        std::unordered_map<std::string, ComPtr<ID3D11ShaderResourceView>> m_Texture{};
    };
}