#pragma once

#include "BaseComponent.h"

#include <memory>
#include <vector>
#include <d3d11.h>

namespace Zongine {
    struct SUBSET_INFO {
        UINT uStartIndex{};
        UINT uIndexCount{};
    };

    struct SubMesh {
        struct VERTEX_BUFFER
        {
            std::shared_ptr<ID3D11Buffer> piBuffer{};
            UINT uStride{};
            UINT uOffset{};
        } VertexBuffer;

        struct INDEX_BUFFER
        {
            std::shared_ptr<ID3D11Buffer> piBuffer{};
            DXGI_FORMAT eFormat{ DXGI_FORMAT_UNKNOWN };
            UINT uOffset{};
        } IndexBuffer;

        std::vector<SUBSET_INFO> Subsets;
    };

    struct MeshComponent : public BaseComponent {
        std::vector<SubMesh> SubMeshes;
    };
}