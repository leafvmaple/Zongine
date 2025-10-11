#pragma once

#include "RenderPass.h"
#include "../Systems/RenderSystem.h"
#include <DirectXMath.h>

namespace Zongine {
    using namespace DirectX;

    // Opaque Pass - Render opaque objects
    class OpaquePass : public RenderPass {
    public:
        OpaquePass(RenderSystem* renderSystem);

        void Setup(RenderGraph& graph) override;
        void Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) override;

    private:
        RenderSystem* m_RenderSystem{ nullptr };
    };

    // OIT Pass - Order Independent Transparency
    class OITPass : public RenderPass {
    public:
        OITPass(RenderSystem* renderSystem);

        void Setup(RenderGraph& graph) override;
        void Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) override;

    private:
        RenderSystem* m_RenderSystem{ nullptr };
    };

    // Composite Pass - Composite final image
    class CompositePass : public RenderPass {
    public:
        CompositePass();

        void Setup(RenderGraph& graph) override;
        void Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) override;
    };

    // Clear Pass - Clear RenderTarget and DepthStencil
    class ClearPass : public RenderPass {
    public:
        ClearPass(const std::string& targetName, const XMFLOAT4& clearColor = XMFLOAT4(0, 0, 0, 1));

        void Setup(RenderGraph& graph) override;
        void Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) override;

        void SetClearColor(const XMFLOAT4& color) { m_ClearColor = color; }
        void SetClearDepth(bool clear) { m_bClearDepth = clear; }

    private:
        std::string m_TargetName;
        XMFLOAT4 m_ClearColor;
        bool m_bClearDepth{ true };
    };

    // Present Pass - Present to SwapChain
    class PresentPass : public RenderPass {
    public:
        PresentPass();

        void Setup(RenderGraph& graph) override;
        void Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) override;
    };
}
