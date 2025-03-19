#include "ResourceManager.h"

#include "Entities/EntityManager.h"
#include "Utilities/DeviceManager.h"
#include "Utilities/EffectManager.h"
#include "Utilities/StringUtils.h"

#include "LAssert.h"

#include "IMesh.h"
#include "IMaterial.h"

#include "DirectXTex/DirectXTex/DirectXTex.h"
#include "FX11/inc/d3dx11effect.h"

namespace Zongine {
    MeshComponent ResourceManager::LoadMesh(const std::string& path) {
        auto& component = m_MeshComponents[path];
        if (component.Subsets.empty()) {
            MESH_DESC desc{ path.c_str() };
            MESH_SOURCE source{};

            ::LoadMesh(&desc, &source);

            _CreateVertexBuffer(component, source);
            _CreateIndexBuffer<WORD>(component, source, DXGI_FORMAT_R16_UINT);
        }
        return component;
    }

    MaterialComponent ResourceManager::LoadMaterial(const std::string& path) {

        auto& component = m_MaterialComponents[path];

        if (component.Subsets.empty()) {
            MODEL_MATERIAL_DESC desc{ path.c_str() };
            MODEL_MATERIAL_SOURCE source;

            LoadModelMaterial(&desc, &source);

            const auto& Group = source.pLOD[0].pGroup[0];
            for (int i = 0; i < Group.nSubset; i++)
            {
                const auto& Subset = Group.pSubset[i];

                auto referMaterial = _LoadReferMaterial(Subset.Define.szName);

                for (int j = 0; j < Subset.nTexture; j++)
                {
                    const auto& texture = Subset.pTexture[j];

                    for (auto& [key, value] : referMaterial->Textures)
                        if (value.Name == texture.szName)
                            value.Texture = _LoadTexture(texture.szValue);
                }

                component.Subsets.emplace_back(referMaterial);
            }
        }

        return component;
    }

    ShaderComponent ResourceManager::LoadShader(RUNTIME_MACRO macro, const std::vector<std::string>& paths) {
        ShaderComponent component{};
        for (const auto& path : paths) {
            auto& cache = m_SubsetShaderCache[macro][path];
            if (cache.ShaderPath.empty()) {
                cache.ShaderPath = path;
                cache.Effect = m_EffectManager->LoadEffect(macro, path);
                m_EffectManager->LoadVariables(cache.Effect, cache.Variables);
            }
            component.Subsets.push_back(cache);
        }

        return component;
    }

    std::shared_ptr<ReferMaterial> ResourceManager::_LoadReferMaterial(const std::string& path) {
        
        auto referMaterial = m_ReferMaterial[path];
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

    bool ResourceManager::_CreateVertexBuffer(MeshComponent& mesh, const MESH_SOURCE& source) {
        D3D11_BUFFER_DESC desc{};
        D3D11_SUBRESOURCE_DATA data{};

        desc.ByteWidth = source.nVertexSize * source.nVerticesCount;;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        data.pSysMem = source.pVertices;

        m_DeviceManager->GetDevice()->CreateBuffer(&desc, &data, mesh.VertexBuffer.piBuffer.GetAddressOf());

        return true;
    }

    template<typename T>
    bool ResourceManager::_CreateIndexBuffer(MeshComponent& mesh, const MESH_SOURCE& source, DXGI_FORMAT nFormat) {
        D3D11_BUFFER_DESC desc{};
        D3D11_SUBRESOURCE_DATA data{};
        std::vector<T> indices{};
        UINT nOffset{};

        for (int i = 0; i < source.nSubsetCount; i++) {
            mesh.Subsets.emplace_back(SubsetMesh{ source.pSubsetVertexCount[i], nOffset });
            nOffset += source.pSubsetVertexCount[i];
        }
        for (int i = 0; i < source.nIndexCount; i++)
            indices.emplace_back(static_cast<T>(source.pIndices[i]));

        desc.ByteWidth = sizeof(T) * (UINT)indices.size();
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        data.pSysMem = indices.data();

        m_DeviceManager->GetDevice()->CreateBuffer(&desc, &data, mesh.IndexBuffer.piBuffer.GetAddressOf());

        return true;
    }
}