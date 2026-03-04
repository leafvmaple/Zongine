#pragma once

// RenderHelper.h — Stateless draw-call submission utilities.
// Extracted from RenderSystem::TickRenderEntity to decouple
// RenderPasses from RenderSystem.

#include "RenderData.h"
#include <d3d11.h>
#include <wrl/client.h>

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    // Submit a single RenderItem using its own Pass type.
    void SubmitRenderItem(ComPtr<ID3D11DeviceContext> context, const RenderItem& item);

    // Submit a single RenderItem with an explicit pass override.
    void SubmitRenderItem(ComPtr<ID3D11DeviceContext> context, const RenderItem& item, RENDER_PASS passOverride);
}
