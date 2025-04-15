#include "AssetManager.h"

#include "Entities/EntityManager.h"

#include "Managers/DeviceManager.h"
#include "Managers/EffectManager.h"

#include "Utilities/StringUtils.h"

#include "Components/TransformComponent.h"
#include "Components/FlexibleComponent.h"
#include "Components/LandscapeComponent.h"
#include "Components/LandscapeRegionComponent.h"
#include "Components/NVFlexComponent.h"

#include "LAssert.h"

#include "IMesh.h"
#include "IModel.h"
#include "IScene.h"
#include "ISkeleton.h"
#include "IMaterial.h"
#include "IAnimation.h"
#include "ILandScape.h"
#include "IFlex.h"

#include "DirectXTex/DirectXTex/DirectXTex.h"
#include "FX11/inc/d3dx11effect.h"

#include <unordered_set>

namespace Zongine {
    void AssetManager::LoadModel(Entity& entity, const std::string& path) {
        MODEL_DESC desc{ path.c_str() };
        MODEL_SOURCE source{};

        entity.AddComponent<TransformComponent>(TransformComponent{});

        ::LoadModel(&desc, &source);

        entity.AddComponent<SkeletonComponent>(SkeletonComponent{ source.szSkeletonPath });

        return;
    }

    void AssetManager::LoadMesh(Entity& entity, const std::string& path) {
        std::filesystem::path filePath = path;

        entity.AddComponent<TransformComponent>(TransformComponent{});
        entity.AddComponent<MeshComponent>(MeshComponent { path });

        auto mesh = GetMeshAsset(path);
        std::vector<DriverInfo> driversInfo;
        std::unordered_set<uint32_t> drivers;
        for (const auto& bone : mesh->Bones) {
            if (bone.Name.starts_with("fbr") && drivers.find(bone.nParentIndex) == drivers.end()) {
                driversInfo.push_back(DriverInfo{ bone.nParentIndex });
                drivers.insert(bone.nParentIndex);
            }
        }
        if (!driversInfo.empty()) {
            entity.AddComponent<FlexibleComponent>(FlexibleComponent{ driversInfo });
        }

        if (TryReplaceExtension(filePath, ".JsonInspack")) {
            auto materialPath = filePath.string();
            entity.AddComponent<MaterialComponent>(MaterialComponent{ materialPath });

            auto material = GetModelMaterialAsset(materialPath);
            std::vector<std::string> paths;
            std::transform(material->Subsets.begin(), material->Subsets.end(), std::back_inserter(paths),
                [](const auto& subset) -> std::string {
                    return subset.ShaderName;
                });
            entity.AddComponent<ShaderComponent>(ShaderComponent{ paths });
        }
        if (TryReplaceExtension(filePath, ".mesh.flx")) {
            auto flexPath = filePath.string();
            auto& flexComponent = entity.AddComponent<NvFlexComponent>(NvFlexComponent{false, flexPath, path });
        }
    }

    void AssetManager::LoadMesh(Entity& entity, const std::string& path, const std::string& socketName) {
        LoadMesh(entity, path);
        auto& transform = entity.GetComponent<TransformComponent>();
        transform.BindType = BIND_TYPE::Socket;
        transform.TargetName = socketName;
    }

    void AssetManager::LoadScene(Entity& entity, const std::string& path) {
        SCENE_DESC desc{ path.c_str() };
        SCENE_SOURCE source{};

        ::LoadScene(&desc, &source);

        auto& landscape = entity.AddChild("Landscape");
        landscape.AddComponent<TransformComponent>(TransformComponent{});
        landscape.AddComponent<LandscapeComponent>(LandscapeComponent{ source.szLandscape });

        auto landscapeAsset = GetLandscapeAsset(source.szDir, source.szMapName);
        for (auto& row : landscapeAsset->Regions) {
            for (auto& region : row) {
                auto& regionEntity = landscape.AddChild("Region");
                regionEntity.AddComponent<TransformComponent>(TransformComponent{});
                regionEntity.AddComponent<LandscapeRegionComponent>(LandscapeRegionComponent{});
                regionEntity.AddComponent<ShaderComponent>(ShaderComponent{ { region.Material.ShaderName } });
            }
        }
    }

    std::shared_ptr<MeshAsset> AssetManager::GetMeshAsset(const std::string& path) {
        auto& mesh = m_MeshCache[path];
        if (!mesh)
            mesh = _LoadMesh(path);
        return mesh;
    }

    std::shared_ptr<MaterialAsset> AssetManager::GetModelMaterialAsset(const std::string& path) {
        auto& material = m_MaterialCache[path];
        if (!material)
            material = _LoadModelMaterial(path);
        return material;
    }

    std::shared_ptr<SkeletonAsset> AssetManager::GetSkeletonAsset(const std::string& path) {
        auto& skeleton = m_SkeletonCache[path];
        if (!skeleton)
            skeleton = _LoadSkeleton(path);
        return skeleton;
    }

    std::shared_ptr<ShaderAsset> AssetManager::GetShaderAsset(RUNTIME_MACRO macro, const std::vector<std::string>& paths) {
        auto& shader = m_ShaderCache[macro][paths];
        if (!shader)
            shader = _LoadShader(macro, paths);
        return shader;
    }

    std::shared_ptr<AnimationAsset> AssetManager::GetAnimationAsset(const std::string& path) {
        auto& animation = m_AnimationCache[path];
        if (!animation)
            animation = _LoadAnimation(path);
        return animation;
    }

    std::shared_ptr<LandscapeAsset> AssetManager::GetLandscapeAsset(const std::string& dir, const std::string name) {
        auto& landscape = m_LandScapeCache[dir];
        if (!landscape)
            landscape = _LoadLandscape(dir, name);
        return landscape;
    }

    std::shared_ptr<NvFlexAsset> AssetManager::GetNvFlexAsset(const std::string& path) {
        auto& flex = m_NvFlexCache[path];
        if (!flex)
            flex = _LoadNvFlex(path);
        return flex;
    }

    const std::vector<int>& AssetManager::GetMeshSkeletonMap(const std::string& skeletonPath, const std::string& meshPath) {
        auto& map = m_MeshSkeletonMap[skeletonPath][meshPath];
        if (map.empty()) {
            auto skeleton = AssetManager::GetSkeletonAsset(skeletonPath);
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

    ComPtr<ID3D11ShaderResourceView> AssetManager::_LoadTexture(const std::string& path) {
        auto& texture = m_TextureCache[path];
        if (!texture) {
            ScratchImage LoadedImage{};
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            ComPtr<ID3D11Resource> resource{};

            std::filesystem::path filePath = path;
            TryReplaceExtension(filePath, ".dds");

            auto extension = filePath.extension();
            auto device = DeviceManager::GetInstance().GetDevice();

            if (extension == ".dds")
                LoadFromDDSFile(filePath.wstring().c_str(), DDS_FLAGS_NONE, nullptr, LoadedImage);
            else if (extension == ".tga")
                LoadFromTGAFile(filePath.wstring().c_str(), nullptr, LoadedImage);

            CHECK_HRESULT_RET(CreateTexture(device.Get(), LoadedImage.GetImages(), LoadedImage.GetImageCount(), LoadedImage.GetMetadata(), resource.GetAddressOf()), nullptr);
            resource.As(&texture);

            srvDesc.Format = LoadedImage.GetMetadata().format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

            srvDesc.Texture2D.MipLevels = -1;
            srvDesc.Texture2D.MostDetailedMip = 0;

            device->CreateShaderResourceView(resource.Get(), &srvDesc, texture.GetAddressOf());
        }
        return texture;
    }

    template<typename T>
    ComPtr<ID3D11ShaderResourceView> AssetManager::_LoadTexture(T* pData, UINT nWidth, UINT nHeight) {
        ComPtr<ID3D11Texture2D> texture{};
        ComPtr<ID3D11ShaderResourceView> shaderResourceView{};
        D3D11_TEXTURE2D_DESC desc{};
        D3D11_SUBRESOURCE_DATA data{};

        desc.ArraySize = 1;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.Format = DXGI_FORMAT_R32_FLOAT;
        desc.Width = nWidth;
        desc.Height = nHeight;
        desc.MipLevels = 1;
        desc.MiscFlags = 0;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;

        data.pSysMem = pData;
        data.SysMemPitch = nWidth * sizeof(T);
        data.SysMemSlicePitch = 0;

        auto device = DeviceManager::GetInstance().GetDevice();

        device->CreateTexture2D(&desc, &data, texture.GetAddressOf());
        device->CreateShaderResourceView(texture.Get(), nullptr, shaderResourceView.GetAddressOf());

        return shaderResourceView;
    }

    std::shared_ptr<ReferenceMaterialAsset> AssetManager::_LoadReferenceMaterial(const std::string& path) {
        auto& referMaterial = m_ReferMaterialCache[path];
        if (!referMaterial) {
            REFER_MATERIAL_DESC desc{ path.c_str() };
            REFER_MATERIAL_SOURCE configSource{};

            referMaterial = std::make_shared<ReferenceMaterialAsset>();

            LoadReferMaterial(&desc, &configSource);

            referMaterial->Path = path;
            referMaterial->ShaderName = configSource.szShaderName;

            for (unsigned int i = 0; i < configSource.nParam; i++) {
                const auto& param = configSource.pParam[i];
                referMaterial->Textures[param.szRegister] = { param.szName, _LoadTexture(param.szValue) };
            }
        }

        return referMaterial;
    }

    std::shared_ptr<MeshAsset> AssetManager::_LoadMesh(const std::string& path) {
        std::filesystem::path filePath = path;
        MESH_DESC desc{ path.c_str() };
        MESH_SOURCE source{};
        auto mesh = std::make_shared<MeshAsset>();

        ::LoadMesh(&desc, &source);

        if (source.nVertexFVF & FVF_SKIN)
            mesh->Macro = RUNTIME_MACRO_SKIN_MESH;
        else
            mesh->Macro = RUNTIME_MACRO_MESH;

        mesh->Path = path;
        _LoadBone(mesh.get(), source);
        _LoadSocket(mesh.get(), source);

        _LoadVertexBuffer(mesh.get(), source);
        _LoadIndexBuffer(mesh.get(), source);

        if (TryReplaceExtension(filePath, ".mesh.flx")) {
            auto flex = GetNvFlexAsset(filePath.string());
            mesh->Macro = RUNTIME_MACRO_FLEX_MESH;

            _LoadNvFlexBuffer(flex.get(), mesh.get(), source);
        }

        mesh->Vertices.resize(source.nVerticesCount);
        memcpy(mesh->Vertices.data(), source.pVertices, sizeof(VERTEX) * source.nVerticesCount);

        return mesh;
    }

    std::shared_ptr<MaterialAsset> AssetManager::_LoadModelMaterial(const std::string& path) {
        MODEL_MATERIAL_DESC desc{ path.c_str() };
        MODEL_MATERIAL_SOURCE source;
        auto material = std::make_shared<MaterialAsset>();

        LoadModelMaterial(&desc, &source);

        const auto& Group = source.pLOD[0].pGroup[0];
        for (unsigned int i = 0; i < Group.nSubset; i++)
            _LoadMaterial(material.get(), Group.pSubset[i]);

        return material;
    }

    std::shared_ptr<SkeletonAsset> AssetManager::_LoadSkeleton(const std::string& path) {
        SKELETON_DESC desc{ path.c_str() };
        SKELETON_SOURCE source{};

        auto skeleton = std::make_shared<SkeletonAsset>();
        ::LoadSkeleton(&desc, &source);
        skeleton->Path = path;

        for (unsigned int i = 0; i < source.nBoneCount; i++) {
            const auto& skeletonSource = source.pBones[i];
            auto& bone = skeleton->Bones.emplace_back(SkeletonBone{ skeletonSource.szBoneName });
            for (int i = 0; i < skeletonSource.nChildCount; i++) {
                bone.Children.push_back(skeletonSource.pChildren[i]);
            }
        }

        skeleton->nRootBoneIndex = source.pRootBoneIndies[0];

        return skeleton;
    }

    std::shared_ptr<ShaderAsset> AssetManager::_LoadShader(RUNTIME_MACRO macro, const std::vector<std::string>& paths) {
        auto shader = std::make_shared<ShaderAsset>();

        for (const auto& path : paths) {
            SubsetShader subsetShader{};

            subsetShader.ShaderPath = path;
            subsetShader.Effect = EffectManager::GetInstance().LoadEffect(macro, path);
            EffectManager::GetInstance().LoadVariables(subsetShader.Effect, subsetShader.Variables);

            // TODO
            subsetShader.ModelConst = subsetShader.Effect->GetConstantBufferByName("MODEL_CONST");
            subsetShader.SubsetConst = subsetShader.Effect->GetConstantBufferByName("SUBSET_CONST");
            subsetShader.CameraConst = subsetShader.Effect->GetConstantBufferByName("CAMERA_MATRIX");

            shader->Subsets.emplace_back(subsetShader);
        }

        return shader;
    }

    std::shared_ptr<AnimationAsset> AssetManager::_LoadAnimation(const std::string& path) {
        ANIMATION_DESC desc{ path.c_str() };
        ANIMATION_SOURCE source{};

        auto animation = std::make_shared<AnimationAsset>();
        ::LoadAnimation(&desc, &source);

        animation->Path = path;
        animation->FrameLength = source.fFrameLength;
        animation->FrameRate = 1000 / source.fFrameLength;
        animation->AnimationLength = source.nAnimationLength;

        for (int i = 0; i < source.nBonesCount; i++) {
            const auto& boneSRT = source.pBoneRTS[i];
            std::vector<AnimationSRT> clip;
            for (int j = 0; j < source.nFrameCount; j++) {
                const auto& SRT = boneSRT[j];
                clip.emplace_back(AnimationSRT{ SRT.Translation, SRT.Scale, SRT.Rotation, SRT.SRotation });
            }
            animation->Clip.emplace_back(clip);
            // animation->BoneFlag.push_back(source.pFlag[i]);
        }

        return animation;
    }

    std::shared_ptr<LandscapeAsset> AssetManager::_LoadLandscape(const std::string& dir, const std::string name) {
        LANDSCAPE_DESC desc{ dir.c_str(), name.c_str() };
        LANDSCAPE_SOURCE source{};
        MaterialAsset material{};

        auto landscape = std::make_shared<LandscapeAsset>();
        ::LoadLandscape(&desc, &source);

        landscape->Dir = dir;
        landscape->Name = name;

        for (int i = 0; i < source.nMaterialCount; i++) {
            assert(source.pMaterials[i].nLODCount > 0);
            _LoadMaterial(&material, source.pMaterials[i].pLOD[0]);
        }

        for (unsigned int y = 0; y < source.RegionTableSize.y; y++) {
            auto& regions = landscape->Regions.emplace_back();
            for (uint32_t x = 0; x < source.RegionTableSize.x; x++) {
                auto& region = regions.emplace_back();
                auto& sourceRegion = source.pRegionTable[y * source.RegionTableSize.x + x];

                _LoadLandscapeRegion(&region, &material, sourceRegion);

                region.Terrain.Origin = { x * source.RegionSize, y * source.RegionSize };
            }
        }

        return landscape;
    }

    std::shared_ptr<NvFlexAsset> AssetManager::_LoadNvFlex(const std::string& path) {
        std::filesystem::path filePath = path;
        FLEX_DESC desc{ path.c_str() };
        FLEX_SOURCE source{};
        auto flex = std::make_shared<NvFlexAsset>();

        ::LoadFlex(&desc, &source);

        flex->Path = path;
        return flex;
    }

    bool AssetManager::_LoadMaterial(MaterialAsset* material, const MATERIAL_SOURCE& source) {
        auto referMaterial = _LoadReferenceMaterial(source.Define.szName);
        auto& refer = material->Subsets.emplace_back(*referMaterial);

        for (unsigned int j = 0; j < source.nTexture; j++)
        {
            const auto& texture = source.pTexture[j];

            for (auto& [key, value] : refer.Textures)
                if (value.Name == texture.szName)
                    value.Texture = _LoadTexture(texture.szValue);
        }

        refer.Rasterizer = RASTERIZER_STATE_CULL_CLOCKWISE;
        if (source.nTwoSideFlag)
            refer.Rasterizer = RASTERIZER_STATE_CULL_NONE;

        refer.nBlendMode = source.nBlendMode;

        refer.Const.AlphaReference = source.nAlphaRef / 255.f;
        refer.Const.AlphaReference2 = source.nAlphaRef2 / 255.f;
        refer.Const.EnableAlphaTest = (refer.nBlendMode == BLEND_MASKED || refer.nBlendMode == BLEND_SOFTMASKED);

        return true;
    }

    bool AssetManager::_LoadBone(MeshAsset* mesh, const MESH_SOURCE& source) {
        for (int i = 0; i < source.nBonesCount; i++) {
            const auto& boneSource = source.pBones[i];
            auto& bone = mesh->Bones.emplace_back(BONE{ 0, boneSource.szName, boneSource.mOffset });

            XMStoreFloat4x4(&bone.PhysicsPoseTransform, XMMatrixInverse(nullptr, XMLoadFloat4x4(&boneSource.mInvPxPose)));

            mesh->BoneMap[boneSource.szName] = i;
        }

        for (int i = 0; i < source.nBonesCount; i++) {
            const auto& boneSource = source.pBones[i];
            auto& bone = mesh->Bones[i];
            for (int j = 0; j < boneSource.nChildCount; j++) {
                auto it = mesh->BoneMap.find(boneSource.pChildNames[j]);
                if (it != mesh->BoneMap.end()) {
                    int childIndex = it->second;
                    bone.Children.push_back(childIndex);
                    mesh->Bones[childIndex].nParentIndex = i;
                }
            }
        }

        return true;
    }

    bool AssetManager::_LoadSocket(MeshAsset* mesh, const MESH_SOURCE& source) {
        for (int i = 0; i < source.nSocketCount; i++) {
            const auto& socketSource = source.pSockets[i];
            mesh->Sockets.emplace_back(SOCKET{ socketSource.szName, mesh->BoneMap[socketSource.szParentName], socketSource.mOffset });
        }

        return true;
    }

    bool AssetManager::_LoadVertexBuffer(MeshAsset* mesh, const MESH_SOURCE& source) {
        D3D11_BUFFER_DESC desc{};
        D3D11_SUBRESOURCE_DATA data{};

        desc.ByteWidth = source.nVertexSize * source.nVerticesCount;;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        data.pSysMem = source.pVertices;

        mesh->Vertex.uStride = source.nVertexSize;

        DeviceManager::GetInstance().GetDevice()->CreateBuffer(&desc, &data, mesh->Vertex.Buffer.GetAddressOf());

        return true;
    }

    bool AssetManager::_LoadIndexBuffer(MeshAsset* mesh, const MESH_SOURCE& source) {
        D3D11_BUFFER_DESC desc{};
        D3D11_SUBRESOURCE_DATA data{};
        UINT nOffset{};

        for (int i = 0; i < source.nSubsetCount; i++) {
            mesh->Subsets.emplace_back(SubsetMeshAsset{ nOffset, source.pSubsetVertexCount[i] });
            nOffset += source.pSubsetVertexCount[i];
        }

        desc.ByteWidth = sizeof(source.pIndices[0]) * source.nIndexCount;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        data.pSysMem = source.pIndices;

        mesh->Index.eFormat = DXGI_FORMAT_R32_UINT;

        DeviceManager::GetInstance().GetDevice()->CreateBuffer(&desc, &data, mesh->Index.Buffer.GetAddressOf());

        return true;
    }

    bool AssetManager::_LoadNvFlexBuffer(NvFlexAsset* flex, MeshAsset* mesh, const MESH_SOURCE& source) {
        D3D11_BUFFER_DESC desc{};

        desc.ByteWidth = sizeof(FLEX_VERTEX_EXT) * source.nVerticesCount;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        flex->uStride = sizeof(FLEX_VERTEX_EXT);

        DeviceManager::GetInstance().GetDevice()->CreateBuffer(&desc, nullptr, flex->Buffers.GetAddressOf());

        for (int i = 0; i < source.nVerticesCount; i++) {
            const auto& vertex = source.pVertices[i];
            const auto& diffuse = vertex.Color;

            float invMass = 0.f;
            if (diffuse.a >= 255)
                invMass = 0.f;
            else if (diffuse.a == 0)
                invMass = 1.f;
            else if (diffuse.a >= 100)
                invMass = std::pow(1.10f, 100.f - diffuse.a);
            else
                invMass = std::pow(1.025f, 100.f - diffuse.a);

            if (invMass != 0) {
                flex->VertexParticleMap.emplace_back((int)flex->VertexParticleMap.size());
            }

            flex->InvMass.emplace_back(invMass);
        }

        return true;
    }

    bool AssetManager::_LoadLandscapeRegion(LandscapeRegionAsset* region, MaterialAsset* material, const LANDSCAPE_REGION& source) {

        for (int i = 0; i < source.nHeightData; i++) {
            auto& row = region->HeightData.emplace_back();
            for (int j = 0; j < source.nHeightData; j++)
                row.push_back(source.pHeightData[i * source.nHeightData + j]);
        }

        region->HeightTexture = _LoadTexture(source.pHeightData, source.nHeightData, source.nHeightData);

        /*for (int k = 0; k < source.nMaterial; k++)
            region->Materials.push_back(material->Subsets[source.pMaterialIDs[k]]);*/

        // TODO: Add support for multiple materials
        region->Material = material->Subsets[source.pMaterialIDs[0]];

        return true;
    }
}