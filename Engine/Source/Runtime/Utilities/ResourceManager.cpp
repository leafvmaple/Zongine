#include "ResourceManager.h"

#include "Entities/EntityManager.h"
#include "Utilities/DeviceManager.h"
#include "IMesh.h"

namespace Zongine {
    void ResourceManager::CreateMeshComponent(const Entity& entity, const std::string& path) {
        MESH_DESC desc{ path.c_str() };
        MESH_SOURCE source{};

        LoadMesh(&desc, &source);

        auto& mesh = m_EntityManager->AddComponent<MeshComponent>(entity.GetID(), MeshComponent{});
        auto& subMesh = mesh.SubMeshes.emplace_back();

        _CreateVertexBuffer(subMesh, source);
        _CreateIndexBuffer<WORD>(subMesh, source, DXGI_FORMAT_R16_UINT);
    }

    bool ResourceManager::_CreateVertexBuffer(SubMesh& mesh, const MESH_SOURCE& source) {
        D3D11_BUFFER_DESC desc{};
        D3D11_SUBRESOURCE_DATA data{};

        desc.ByteWidth = source.nVertexSize * source.nVerticesCount;;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        data.pSysMem = source.pVertices;

        m_WindowManager->GetDevice()->CreateBuffer(&desc, &data, mesh.VertexBuffer.piBuffer.GetAddressOf());

        return true;
    }

    template<typename T>
    bool ResourceManager::_CreateIndexBuffer(SubMesh& mesh, const MESH_SOURCE& source, DXGI_FORMAT nFormat) {
        D3D11_BUFFER_DESC desc{};
        D3D11_SUBRESOURCE_DATA data{};
        std::vector<T> indices{};
        UINT nOffset{};

        for (int i = 0; i < source.nSubsetCount; i++) {
            mesh.Subsets.emplace_back(SUBSET_INFO{ source.pSubsetVertexCount[i], nOffset });
            nOffset += source.pSubsetVertexCount[i];
        }
        for (int i = 0; i < source.nIndexCount; i++)
            indices.emplace_back(static_cast<T>(source.pIndices[i]));

        desc.ByteWidth = sizeof(T) * (UINT)indices.size();
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        data.pSysMem = indices.data();

        m_WindowManager->GetDevice()->CreateBuffer(&desc, &data, mesh.IndexBuffer.piBuffer.GetAddressOf());

        return true;
    }

}