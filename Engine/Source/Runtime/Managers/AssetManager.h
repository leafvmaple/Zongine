#pragma once

#include "Include/Enums.h"

#include "Utilities/StringUtils.h"

#include "Components/MeshComponent.h"
#include "Components/ShaderComponent.h"
#include "Components/MaterialComponent.h"
#include "Components/SkeletonComponent.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <array>
#include <DirectXMath.h>
#include <wrl/client.h>

#include "FX11/inc/d3dx11effect.h"

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
        UINT nParentIndex{};
        std::string Name{};
        DirectX::XMFLOAT4X4 InversePoseTransform{};
        DirectX::XMFLOAT4X4 PhysicsPoseTransform{};
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
        } Vertex{};

        struct INDEX_BUFFER {
            ComPtr<ID3D11Buffer> Buffer{};
            DXGI_FORMAT eFormat{ DXGI_FORMAT_UNKNOWN };
            UINT uOffset{};
        } Index{};

        INPUT_LAYOUT InputLayout{};

        std::vector<SubsetMeshAsset> Subsets{};
        std::vector<BONE> Bones{};
        std::vector<SOCKET> Sockets{};

        std::unordered_map<std::string, UINT> BoneMap{};
    };

    struct _Texture {
        std::string Name{};
        ComPtr<ID3D11ShaderResourceView> Texture{};
    };

    __declspec(align(16)) struct SKIN_SUBSET_CONST
    {
        DirectX::XMFLOAT4A ModelColor;
        BOOL        EnableAlphaTest;
        float       AlphaReference;
        float       AlphaReference2;
    };

    struct ReferMaterial {
        std::string Path{};
        std::string ShaderName{};

        uint32_t nBlendMode;
        RASTERIZER_STATE_ID Rasterizer{};

        SKIN_SUBSET_CONST Const{};

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

        int nRootBoneIndex{};
    };

    struct SubsetShader {
        std::string ShaderPath{};

        ComPtr<ID3DX11Effect> Effect{};
        ID3DX11EffectVariable* SubsetConst{};
        std::unordered_map<std::string, ID3DX11EffectShaderResourceVariable*> Variables{};
    };

    struct ShaderAsset {
        std::vector<SubsetShader> Subsets{};

        RENDER_PASS Pass{};

        ID3DX11EffectMatrixVariable* TransformMatrix{};
        ID3DX11EffectMatrixVariable* BonesMatrix{};

        std::vector<ComPtr<ID3D11Buffer>> SubsetBuffers{};
    };

    struct AnimationSRT {
        DirectX::XMFLOAT3 Translation{ 0.f, 0.f, 0.f };
        DirectX::XMFLOAT3 Scale{ 1.f, 1.f, 1.f };
        DirectX::XMFLOAT4 Rotation{ 0.f, 0.f, 0.f, 1.f };
        DirectX::XMFLOAT4 SRotation{ 0.f, 0.f, 0.f, 1.f };
    };

    struct AnimationAsset {
        std::string Path{};
        float FrameRate{};     // Frame per second
        float FrameLength{};   // ms

        int AnimationLength{};   // ms

        std::vector<int> BoneFlag{};
        std::vector<std::vector<AnimationSRT>> Clip{};
    };

    struct AssetManagerDesc {
        std::shared_ptr<EntityManager> entityManager{};
        std::shared_ptr<DeviceManager> windowManager{};
        std::shared_ptr<EffectManager> effectManager{};
    };

    class AssetManager {
    public:
        void Initialize(const AssetManagerDesc& info) {
            m_DeviceManager = info.windowManager;
            m_EntityManager = info.entityManager;
            m_EffectManager = info.effectManager;
        };

        void LoadModel(Entity& entity, const std::string& path);
        void LoadMesh(Entity& entity, const std::string& path);
        void LoadMesh(Entity& entity, const std::string& path, const std::string& socketName);

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

        std::shared_ptr<ShaderAsset> GetShaderAsset(RUNTIME_MACRO macro, const std::vector<std::string>& paths) {
            auto& shader = m_ShaderCache[macro][paths];
            if (!shader)
                shader = _LoadShader(macro, paths);
            return shader;
        }

        std::shared_ptr<AnimationAsset> GetAnimationAsset(const std::string& path) {
            auto& animation = m_AnimationCache[path];
            if (!animation)
                animation = _LoadAnimation(path);
            return animation;
        }

        const std::vector<int>& GetMeshSkeletonMap(const std::string& skeletonPath, const std::string& meshPath) {
            auto& map = m_MeshSkeletonMap[skeletonPath][meshPath];
            if (map.empty()) {
                auto skeleton = GetSkeletonAsset(skeletonPath);
                auto mesh = GetMeshAsset(meshPath);
                map.resize(mesh->BoneMap.size(), -1);
                for (auto [boneName, index] : mesh->BoneMap) {
                    auto it = std::find_if(skeleton->Bones.begin(), skeleton->Bones.end(), [boneName](const SkeletonBone& bone) {
                        return bone.Name == boneName;
                        });
                    if (it != skeleton->Bones.end()) {
                        map[index] = static_cast<int>(std::distance(skeleton->Bones.begin(), it));
                    }
                }
            }
            return map;
        }

    private:
        std::shared_ptr<ReferMaterial> _LoadReferMaterial(const std::string& path);
        ComPtr<ID3D11ShaderResourceView> _LoadTexture(const std::string& path);

        std::shared_ptr<MeshAsset> _LoadMesh(const std::string& path);
        std::shared_ptr<MaterialAsset> _LoadMaterial(const std::string& path);
        std::shared_ptr<SkeletonAsset> _LoadSkeleton(const std::string& path);
        std::shared_ptr<ShaderAsset> _LoadShader(RUNTIME_MACRO macro, const std::vector<std::string>& paths);
        std::shared_ptr<AnimationAsset> _LoadAnimation(const std::string& path);

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
        std::unordered_map<std::string, std::shared_ptr<ReferMaterial>> m_ReferMaterialCache{};

        std::unordered_map<std::string, std::shared_ptr<SkeletonAsset>> m_SkeletonCache{};
        std::array<std::unordered_map<std::vector<std::string>, std::shared_ptr<ShaderAsset>>, RUNTIME_MACRO_COUNT> m_ShaderCache{};
        std::unordered_map<std::string, std::shared_ptr<AnimationAsset>> m_AnimationCache{};
        std::unordered_map<std::string, ComPtr<ID3D11ShaderResourceView>> m_TextureCache{};

        // Map
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>>> m_MeshSkeletonMap{};
    };
}