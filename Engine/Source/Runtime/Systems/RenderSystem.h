#pragma once

#include <d3d11.h>
#include <vector>
#include <memory>
#include <wrl/client.h>

#include "Include/Types.h"

#include "RenderGraph/RenderData.h"

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    class RenderGraph;

    class RenderSystem {
    public:
        RenderSystem();
        ~RenderSystem();
        
        void Initialize();
        void Tick(float fDeltaTime);

    private:
        // Per-frame render context (shared with RenderGraph / RenderPasses)
        RenderContext m_RenderContext{};

        // RenderGraph
        std::unique_ptr<RenderGraph> m_RenderGraph;

        void InitializeRenderGraph();

        // Queue building
        void _UpdateCamera(EntityID entityID);
        void _UpdateRenderQueue(EntityID entityID);
        void _AddRenderEntity(EntityID entityID);
    };
}
