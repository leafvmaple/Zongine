#pragma once

#include "BaseComponent.h"

#include <memory>
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    struct SubsetMesh {
        UINT uStartIndex{};
        UINT uIndexCount{};
    };

    struct MeshComponent : public BaseComponent {

        struct VERTEX_BUFFER
        {
            ComPtr<ID3D11Buffer> Buffer{};
            UINT uStride{};
            UINT uOffset{};
        } VertexBuffer;

        struct INDEX_BUFFER
        {
            ComPtr<ID3D11Buffer> Buffer{};
            DXGI_FORMAT eFormat{ DXGI_FORMAT_UNKNOWN };
            UINT uOffset{};
        } IndexBuffer;

        std::vector<SubsetMesh> Subsets;
    };
}