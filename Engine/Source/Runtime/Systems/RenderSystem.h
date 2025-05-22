#pragma once

#include <d3d11.h>
#include <vector>
#include <wrl/client.h>

#include "Entities/Entity.h"

#include "../Include/AssetData.h"

#include "components/CameraComponent.h"

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    class RenderSystem {
    public:
        void Initialize();
        void Tick(float fDeltaTime);

    private:
        struct VertexVector {
            std::vector<ID3D11Buffer*> Buffers{};
            std::vector<UINT> Strides{};
            std::vector<UINT> Offsets{};
        };

        struct RenderEntity {
            RENDER_PASS Pass{};
            RUNTIME_MACRO Macro{};

            VertexVector Vertex{};
            INDEX_BUFFER Index{};;

            SubsetMeshAsset* Mesh{};
            SubsetShaderAsset* Shader{};
            ReferenceMaterialAsset* Material{};
        };

        std::vector<RenderEntity> m_OpaqueRenderQueue{};
        std::vector<RenderEntity> m_OITRenderQueue{};

        std::vector<Entity> m_TerrainRenderQueue{};

        ComPtr<ID3D11Buffer> m_CameraBuffer{};

        void TickRenderEntity(const RenderEntity& renderEntity);
        void TickRenderEntity(const RenderEntity& renderEntity, RENDER_PASS pass);

        void TickTerrain(ComPtr<ID3D11DeviceContext> context, const Entity& entity);

        void _UpdateRenderQueue(Entity& entity);

        void _AddRenderEntity(Entity& entity);
    };
}
