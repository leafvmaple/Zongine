#include "ResourceManager.h"

#include "Entities/EntityManager.h"
#include "IMesh.h"

namespace Zongine {
    ResourceManager GResourceManager;

    void ResourceManager::CreateMeshComponent(const Entity& entity, const std::string& path) {
        MESH_DESC desc{ path.c_str() };
        MESH_SOURCE source{};

        LoadMesh(&desc, &source);

        auto& mesh = GEntityManager.AddComponent<MeshComponent>(entity.GetID(), MeshComponent{});
        auto& subMesh = mesh.SubMeshes.emplace_back();

        _CreateVertexBuffer(subMesh, source);
        _CreateIndexBuffer<WORD>(subMesh, source, DXGI_FORMAT_R16_UINT);

    }

    bool ResourceManager::_CreateVertexBuffer(SubMesh& mesh, const MESH_SOURCE& source) {
        D3D11_BUFFER_DESC desc{};
        D3D11_SUBRESOURCE_DATA data{};
        ID3D11Buffer* piBuffer{};

        desc.ByteWidth = source.nVertexSize * source.nVerticesCount;;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        data.pSysMem = source.pVertices;

        m_piDevice->CreateBuffer(&desc, &data, &piBuffer);

        mesh.VertexBuffer.piBuffer = std::shared_ptr<ID3D11Buffer>(piBuffer,
            [](ID3D11Buffer* ptr) {
                ptr->Release();
            });

        return true;
    }

    template<typename T>
    bool ResourceManager::_CreateIndexBuffer(SubMesh& mesh, const MESH_SOURCE& source, DXGI_FORMAT nFormat) {
        D3D11_BUFFER_DESC desc{};
        D3D11_SUBRESOURCE_DATA data{};
        std::vector<T> Indices{};
        UINT nOffset{};
        ID3D11Buffer* piBuffer{};

        for (int i = 0; i < source.nSubsetCount; i++)
        {
            mesh.Subsets.emplace_back(SUBSET_INFO{ source.pSubsetVertexCount[i], nOffset });
            nOffset += source.pSubsetVertexCount[i];
        }
        for (int i = 0; i < source.nIndexCount; i++)
            Indices.emplace_back(static_cast<T>(source.pIndices[i]));

        desc.ByteWidth = sizeof(T) * Indices.size();
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        data.pSysMem = Indices.data();

        m_piDevice->CreateBuffer(&desc, &data, &piBuffer);

        mesh.IndexBuffer.piBuffer = std::shared_ptr<ID3D11Buffer>(piBuffer,
            [](ID3D11Buffer* ptr) {
                ptr->Release();
            });

        return true;
    }

}