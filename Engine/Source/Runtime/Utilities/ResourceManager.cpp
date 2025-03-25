#include "ResourceManager.h"

#include "Entities/EntityManager.h"
#include "Utilities/DeviceManager.h"
#include "Utilities/EffectManager.h"
#include "Utilities/StringUtils.h"

#include "Components/TransformComponent.h"


#include "LAssert.h"

#include "IMesh.h"
#include "IModel.h"
#include "ISkeleton.h"
#include "IMaterial.h"

#include "DirectXTex/DirectXTex/DirectXTex.h"
#include "FX11/inc/d3dx11effect.h"

namespace Zongine {
    void ResourceManager::LoadModel(Entity& entity, const std::string& path) {
        MODEL_DESC desc{ path.c_str() };
        MODEL_SOURCE source{};

        entity.AddComponent<TransformComponent>(TransformComponent{});

        ::LoadModel(&desc, &source);

        entity.AddComponent<SkeletonComponent>(SkeletonComponent{ source.szSkeletonPath });

        return;
    }

    void ResourceManager::LoadMesh(Entity& entity, const std::string& path) {
        std::filesystem::path filePath = path;

        entity.AddComponent<TransformComponent>(TransformComponent{});
        entity.AddComponent<MeshComponent>(MeshComponent { path });

        if (TryReplaceExtension(filePath, ".JsonInspack")) {
            auto materialPath = filePath.string();
            entity.AddComponent<MaterialComponent>(MaterialComponent{ materialPath });
            entity.AddComponent<ShaderComponent>(ShaderComponent{ materialPath });
        }
    }

    std::shared_ptr<ReferMaterial> ResourceManager::_LoadReferMaterial(const std::string& path) {
        auto& referMaterial = m_ReferMaterial[path];
        if (!referMaterial) {
            REFER_MATERIAL_DESC desc{ path.c_str() };
            REFER_MATERIAL_SOURCE configSource{};

            referMaterial = std::make_shared<ReferMaterial>();

            LoadReferMaterial(&desc, &configSource);

            referMaterial->Path = path;
            referMaterial->ShaderName = configSource.szShaderName;

            for (int i = 0; i < configSource.nParam; i++) {
                const auto& param = configSource.pParam[i];
                referMaterial->Textures[param.szRegister] = { param.szName, _LoadTexture(param.szValue) };
            }
        }

        return referMaterial;
    }

    ComPtr<ID3D11ShaderResourceView> ResourceManager::_LoadTexture(const std::string& path) {
        auto& texture = m_Texture[path];
        if (!texture) {
            ScratchImage LoadedImage{};
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            ComPtr<ID3D11Resource> resource{};

            std::filesystem::path filePath = path;
            TryReplaceExtension(filePath, ".dds");

            auto extension = filePath.extension();
            auto device = m_DeviceManager->GetDevice();

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

    std::shared_ptr<MeshAsset> ResourceManager::_LoadMesh(const std::string& path) {
        MESH_DESC desc{ path.c_str() };
        MESH_SOURCE source{};
        auto mesh = std::make_shared<MeshAsset>();

        ::LoadMesh(&desc, &source);

        mesh->Path = path;
        _LoadBone(mesh.get(), source);
        _LoadSocket(mesh.get(), source);

        _LoadVertexBuffer(mesh.get(), source);
        _LoadIndexBuffer(mesh.get(), source);

        return mesh;
    }

    std::shared_ptr<MaterialAsset> ResourceManager::_LoadMaterial(const std::string& path) {
        MODEL_MATERIAL_DESC desc{ path.c_str() };
        MODEL_MATERIAL_SOURCE source;
        auto material = std::make_shared<MaterialAsset>();

        LoadModelMaterial(&desc, &source);

        const auto& Group = source.pLOD[0].pGroup[0];
        for (int i = 0; i < Group.nSubset; i++)
        {
            const auto& Subset = Group.pSubset[i];

            auto referMaterial = _LoadReferMaterial(Subset.Define.szName);
            auto& refer = material->Subsets.emplace_back(*referMaterial);

            for (int j = 0; j < Subset.nTexture; j++)
            {
                const auto& texture = Subset.pTexture[j];

                for (auto& [key, value] : refer.Textures)
                    if (value.Name == texture.szName)
                        value.Texture = _LoadTexture(texture.szValue);
            }

            refer.Rasterizer = RASTERIZER_STATE_CULL_CLOCKWISE;
            if (Subset.nTwoSideFlag)
                refer.Rasterizer = RASTERIZER_STATE_CULL_NONE;
        }

        return material;
    }

    std::shared_ptr<SkeletonAsset> ResourceManager::_LoadSkeleton(const std::string& path) {
        SKELETON_DESC desc{ path.c_str() };
        SKELETON_SOURCE source{};

        auto skeleton = std::make_shared<SkeletonAsset>();
        ::LoadSkeleton(&desc, &source);
        skeleton->Path = path;

        for (int i = 0; i < source.nBoneCount; i++) {
            const auto& skeletonSource = source.pBones[i];
            auto& bone = skeleton->Bones.emplace_back(SkeletonBone{ skeletonSource.szBoneName });
            for (int i = 0; i < skeletonSource.nChildCount; i++) {
                bone.Children.push_back(skeletonSource.pChildren[i]);
            }
        }
        return skeleton;
    }

    std::shared_ptr<ShaderAsset> ResourceManager::_LoadShader(RUNTIME_MACRO macro, const std::string& path) {
        D3D11_BUFFER_DESC bufferDesc{};
        auto shader = std::make_shared<ShaderAsset>();

        const auto& material = GetMaterialAsset(path);

        std::vector<std::string> paths;
        std::transform(material->Subsets.begin(), material->Subsets.end(), std::back_inserter(paths),
            [](const auto& subset) -> std::string {
                return subset.ShaderName;
            });

        auto device = m_DeviceManager->GetDevice();

        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.ByteWidth = sizeof(XMMATRIX);
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        device->CreateBuffer(&bufferDesc, nullptr, shader->ModelBuffer.GetAddressOf());

        for (const auto& path : paths) {
            auto& cache = m_SubsetShaderCache[macro][path];
            if (cache.ShaderPath.empty()) {
                cache.ShaderPath = path;
                cache.Effect = m_EffectManager->LoadEffect(macro, path);
                m_EffectManager->LoadVariables(cache.Effect, cache.Variables);
            }
            cache.Effect->GetConstantBufferByName("MODEL_MATRIX")->SetConstantBuffer(shader->ModelBuffer.Get());
            shader->Subsets.push_back(cache);
        }

        return shader;
    }

    bool ResourceManager::_LoadBone(MeshAsset* mesh, const MESH_SOURCE& source) {
        for (int i = 0; i < source.nBonesCount; i++) {
            const auto& boneSource = source.pBones[i];
            auto& bone = mesh->Bones.emplace_back(BONE{ (UINT)i, boneSource.szName, boneSource.mOffset });

            mesh->BoneMap[boneSource.szName] = i;
        }

        for (int i = 0; i < source.nBonesCount; i++) {
            const auto& boneSource = source.pBones[i];
            auto& bone = mesh->Bones[i];
            for (int j = 0; j < boneSource.nChildCount; j++) {
                auto it = mesh->BoneMap.find(boneSource.pChildNames[j]);
                if (it != mesh->BoneMap.end()) {
                    bone.Children.push_back(it->second);
                }
            }
        }

        return true;
    }

    bool ResourceManager::_LoadSocket(MeshAsset* mesh, const MESH_SOURCE& source) {
        for (int i = 0; i < source.nSocketCount; i++) {
            const auto& socketSource = source.pSockets[i];
            mesh->Sockets.emplace_back(SOCKET{ socketSource.szName, mesh->BoneMap[socketSource.szParentName], socketSource.mOffset });
        }

        return true;
    }

    bool ResourceManager::_LoadVertexBuffer(MeshAsset* mesh, const MESH_SOURCE& source) {
        D3D11_BUFFER_DESC desc{};
        D3D11_SUBRESOURCE_DATA data{};

        desc.ByteWidth = source.nVertexSize * source.nVerticesCount;;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        data.pSysMem = source.pVertices;

        mesh->Vertex.uStride = source.nVertexSize;

        m_DeviceManager->GetDevice()->CreateBuffer(&desc, &data, mesh->Vertex.Buffer.GetAddressOf());

        return true;
    }

    bool ResourceManager::_LoadIndexBuffer(MeshAsset* mesh, const MESH_SOURCE& source) {
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

        m_DeviceManager->GetDevice()->CreateBuffer(&desc, &data, mesh->Index.Buffer.GetAddressOf());

        return true;
    }
}