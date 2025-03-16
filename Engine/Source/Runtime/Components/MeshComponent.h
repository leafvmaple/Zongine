#pragma once

#include "BaseComponent.h"

#include <memory>
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    struct SUBSET_INFO {
        UINT uStartIndex{};
        UINT uIndexCount{};
    };

    struct SubMesh {
        struct VERTEX_BUFFER
        {
            ComPtr<ID3D11Buffer> piBuffer{};
            UINT uStride{};
            UINT uOffset{};
        } VertexBuffer;

        struct INDEX_BUFFER
        {
            ComPtr<ID3D11Buffer> piBuffer{};
            DXGI_FORMAT eFormat{ DXGI_FORMAT_UNKNOWN };
            UINT uOffset{};
        } IndexBuffer;

        std::vector<SUBSET_INFO> Subsets;
    };

    struct MeshComponent : public BaseComponent {
        std::vector<SubMesh> SubMeshes;
    };
}