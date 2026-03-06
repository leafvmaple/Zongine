#pragma once

// RenderData.h -- Pure data structures for the rendering pipeline.
// No system/manager dependencies. This file breaks the circular dependency
// between RenderSystem and RenderPasses.

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

#include "../Include/AssetData.h"
#include "../Include/Types.h"

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    // Vertex buffer binding group
    struct VertexVector {
        std::vector<ID3D11Buffer*> Buffers{};
        std::vector<UINT> Strides{};
        std::vector<UINT> Offsets{};
    };

    // A single drawable unit (one subset of a mesh)
    struct RenderItem {
        RENDER_PASS Pass{};
        RUNTIME_MACRO Macro{};

        VertexVector Vertex{};
        INDEX_BUFFER Index{};

        SubsetMeshAsset* Mesh{};
        SubsetShaderAsset* Shader{};
        ReferenceMaterialAsset* Material{};
    };

    // Per-frame render state shared between RenderSystem and RenderPasses.
    // Built by RenderSystem each frame, consumed read-only by RenderPasses.
    struct RenderContext {
        std::vector<RenderItem> OpaqueQueue{};
        std::vector<RenderItem> OITQueue{};
        std::vector<EntityID> TerrainQueue{};
        ComPtr<ID3D11Buffer> CameraBuffer{};
    };
}
