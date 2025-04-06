#pragma once

#include "Mananger.h"

#include "Include/AssetData.h"

#include "Utilities/StringUtils.h"

#include "Components/MeshComponent.h"
#include "Components/ShaderComponent.h"
#include "Components/MaterialComponent.h"
#include "Components/SkeletonComponent.h"

#include <array>
#include <memory>

struct MESH_SOURCE;
struct MATERIAL_SOURCE;
struct LANDSCAPE_REGION;

namespace Zongine {
    class Entity;

    using Microsoft::WRL::ComPtr;

    class AssetManager : public SingleManager<AssetManager> {
    public:
        void LoadModel(Entity& entity, const std::string& path);
        void LoadMesh(Entity& entity, const std::string& path);
        void LoadMesh(Entity& entity, const std::string& path, const std::string& socketName);
        void LoadScene(Entity& entity, const std::string& path);

        std::shared_ptr<MeshAsset> GetMeshAsset(const std::string& path);
        std::shared_ptr<MaterialAsset> GetModelMaterialAsset(const std::string& path);
        std::shared_ptr<SkeletonAsset> GetSkeletonAsset(const std::string& path);
        std::shared_ptr<ShaderAsset> GetShaderAsset(RUNTIME_MACRO macro, const std::vector<std::string>& paths);
        std::shared_ptr<AnimationAsset> GetAnimationAsset(const std::string& path);
        std::shared_ptr<LandscapeAsset> GetLandscapeAsset(const std::string& dir, const std::string name);

        const std::vector<int>& GetMeshSkeletonMap(const std::string& skeletonPath, const std::string& meshPath);

    private:

        ComPtr<ID3D11ShaderResourceView> _LoadTexture(const std::string& path);
        template<typename T>
        ComPtr<ID3D11ShaderResourceView> _LoadTexture(T* pData, UINT nWidth, UINT nHeight);

        std::shared_ptr<ReferenceMaterialAsset> _LoadReferenceMaterial(const std::string& path);
        std::shared_ptr<MeshAsset> _LoadMesh(const std::string& path);
        std::shared_ptr<MaterialAsset> _LoadModelMaterial(const std::string& path);
        std::shared_ptr<SkeletonAsset> _LoadSkeleton(const std::string& path);
        std::shared_ptr<ShaderAsset> _LoadShader(RUNTIME_MACRO macro, const std::vector<std::string>& paths);
        std::shared_ptr<AnimationAsset> _LoadAnimation(const std::string& path);
        std::shared_ptr<LandscapeAsset> _LoadLandscape(const std::string& dir, const std::string name);

        bool _LoadMaterial(MaterialAsset* material, const MATERIAL_SOURCE& source);

        bool _LoadBone(MeshAsset* mesh, const MESH_SOURCE& source);
        bool _LoadSocket(MeshAsset* mesh, const MESH_SOURCE& source);
        bool _LoadVertexBuffer(MeshAsset* mesh, const MESH_SOURCE& source);
        bool _LoadIndexBuffer(MeshAsset* mesh, const MESH_SOURCE& source);

        bool _LoadLandscapeRegion(LandscapeRegionAsset* region, MaterialAsset* material, const LANDSCAPE_REGION& source);

        // Cache
        std::unordered_map<std::string, std::shared_ptr<MeshAsset>> m_MeshCache{};
        std::unordered_map<std::string, std::shared_ptr<MaterialAsset>> m_MaterialCache{};
        std::unordered_map<std::string, std::shared_ptr<ReferenceMaterialAsset>> m_ReferMaterialCache{};

        std::unordered_map<std::string, std::shared_ptr<SkeletonAsset>> m_SkeletonCache{};
        std::array<std::unordered_map<std::vector<std::string>, std::shared_ptr<ShaderAsset>>, RUNTIME_MACRO_COUNT> m_ShaderCache{};
        std::unordered_map<std::string, std::shared_ptr<AnimationAsset>> m_AnimationCache{};
        std::unordered_map<std::string, ComPtr<ID3D11ShaderResourceView>> m_TextureCache{};
        std::unordered_map<std::string, std::shared_ptr<LandscapeAsset>> m_LandScapeCache{};

        // Map
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>>> m_MeshSkeletonMap{};
    };
}