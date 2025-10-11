#pragma once

#include <d3d11.h>
#include <vector>
#include <memory>
#include <wrl/client.h>

#include "Entities/Entity.h"

#include "../Include/AssetData.h"

#include "components/CameraComponent.h"

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    class RenderGraph;

    class RenderSystem {
    public:
        // Public structures for RenderPass access
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

        RenderSystem();
        ~RenderSystem();
        
        void Initialize();
        void Tick(float fDeltaTime);
        
        // RenderGraph related interfaces
        void InitializeRenderGraph();
        void RenderOpaqueQueue(ComPtr<ID3D11DeviceContext> context);
        void RenderOITQueue(ComPtr<ID3D11DeviceContext> context);
        
        // Get render queues for RenderPass
        const std::vector<RenderEntity>& GetOpaqueQueue() const { return m_OpaqueRenderQueue; }
        const std::vector<RenderEntity>& GetOITQueue() const { return m_OITRenderQueue; }

    private:
        std::vector<RenderEntity> m_OpaqueRenderQueue{};
        std::vector<RenderEntity> m_OITRenderQueue{};

        std::vector<Entity> m_TerrainRenderQueue{};

        ComPtr<ID3D11Buffer> m_CameraBuffer{};
        
        // RenderGraph
        std::unique_ptr<RenderGraph> m_RenderGraph;

        void TickRenderEntity(const RenderEntity& renderEntity);
        void TickRenderEntity(const RenderEntity& renderEntity, RENDER_PASS pass);

        void TickTerrain(ComPtr<ID3D11DeviceContext> context, const Entity& entity);

        void _UpdateRenderQueue(Entity& entity);

        void _AddRenderEntity(Entity& entity);
    };
}
