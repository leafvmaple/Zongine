#include "RenderPasses.h"
#include "RenderGraph.h"
#include "../Managers/DeviceManager.h"
#include "../Managers/StateManager.h"
#include "../Managers/EffectManager.h"

namespace Zongine {

    // ==================== OpaquePass ====================
    OpaquePass::OpaquePass(RenderSystem* renderSystem)
        : m_RenderSystem(renderSystem) {
    }

    void OpaquePass::Setup(RenderGraph& graph) {
        // Output to main render target
        AddOutput("MainRT");
        // Use depth buffer
        AddInput("DepthStencil");
        AddOutput("DepthStencil");
    }

    void OpaquePass::Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) {
        if (!m_bEnabled || !m_RenderSystem) {
            return;
        }

        auto rtv = graph.GetRenderTarget("MainRT");
        auto dsv = graph.GetDepthStencil("DepthStencil");

        if (!rtv || !dsv) {
            return;
        }

        // Set render target
        context->OMSetRenderTargets(1, rtv.GetAddressOf(), dsv.Get());

        // Render opaque objects
        m_RenderSystem->RenderOpaqueQueue(context);
    }

    // ==================== OITPass ====================
    OITPass::OITPass(RenderSystem* renderSystem)
        : m_RenderSystem(renderSystem) {
    }

    void OITPass::Setup(RenderGraph& graph) {
        // Input main render target
        AddInput("MainRT");
        // Output to OIT accumulation and weight buffers
        AddOutput("OITAccumulation");
        AddOutput("OITWeight");
        // Use depth buffer
        AddInput("DepthStencil");
    }

    void OITPass::Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) {
        if (!m_bEnabled || !m_RenderSystem) {
            return;
        }

        auto accRTV = graph.GetRenderTarget("OITAccumulation");
        auto weightRTV = graph.GetRenderTarget("OITWeight");
        auto dsv = graph.GetDepthStencil("DepthStencil");

        if (!accRTV || !weightRTV || !dsv) {
            return;
        }

        ID3D11RenderTargetView* rtvs[] = { accRTV.Get(), weightRTV.Get() };
        context->OMSetRenderTargets(2, rtvs, dsv.Get());

        // Render transparent objects
        m_RenderSystem->RenderOITQueue(context);
    }

    // ==================== CompositePass ====================
    CompositePass::CompositePass() {
    }

    void CompositePass::Setup(RenderGraph& graph) {
        // Input all textures to be composited
        AddInput("MainRT");
        AddInput("OITAccumulation");
        AddInput("OITWeight");
        // Output to swap chain
        AddOutput("SwapChainRT");
    }

    void CompositePass::Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) {
        if (!m_bEnabled) {
            return;
        }

        auto swapChainRTV = graph.GetRenderTarget("SwapChainRT");
        auto mainSRV = graph.GetShaderResource("MainRT");
        auto accSRV = graph.GetShaderResource("OITAccumulation");
        auto weightSRV = graph.GetShaderResource("OITWeight");

        if (!swapChainRTV || !mainSRV || !accSRV || !weightSRV) {
            return;
        }

        // Set render target
        context->OMSetRenderTargets(1, swapChainRTV.GetAddressOf(), nullptr);

        // Set blend state
        context->OMSetBlendState(
            StateManager::GetInstance().GetBlendState(BLEND_STATE_COPY).Get(),
            nullptr,
            0xFFFFFFFF
        );

        // Set sampler
        context->PSSetSamplers(
            0,
            1,
            StateManager::GetInstance().GetSamplerState(SAMPLER_STATE_LINEAR3_CLAMP).GetAddressOf()
        );

        // Set shader resources
        ID3D11ShaderResourceView* srvs[] = { mainSRV.Get(), accSRV.Get(), weightSRV.Get() };
        context->PSSetShaderResources(0, 3, srvs);

        // Set fullscreen triangle rendering state
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        context->IASetInputLayout(nullptr);
        context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

        // Apply OIT composite effect
        EffectManager::GetInstance().ApplyOIT();

        // Draw fullscreen quad
        context->Draw(4, 0);

        // Clear shader resource bindings
        ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr };
        context->PSSetShaderResources(0, 3, nullSRVs);
    }

    // ==================== ClearPass ====================
    ClearPass::ClearPass(const std::string& targetName, const XMFLOAT4& clearColor)
        : m_TargetName(targetName), m_ClearColor(clearColor) {
    }

    void ClearPass::Setup(RenderGraph& graph) {
        AddOutput(m_TargetName);
        if (m_bClearDepth) {
            AddOutput("DepthStencil");
        }
    }

    void ClearPass::Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) {
        if (!m_bEnabled) {
            return;
        }

        auto rtv = graph.GetRenderTarget(m_TargetName);
        if (rtv) {
            context->ClearRenderTargetView(rtv.Get(), &m_ClearColor.x);
        }

        if (m_bClearDepth) {
            auto dsv = graph.GetDepthStencil("DepthStencil");
            if (dsv) {
                context->ClearDepthStencilView(
                    dsv.Get(),
                    D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                    1.0f,
                    0
                );
            }
        }
    }

    // ==================== PresentPass ====================
    PresentPass::PresentPass() {
    }

    void PresentPass::Setup(RenderGraph& graph) {
        // Present depends on SwapChainRT being written
        AddInput("SwapChainRT");
    }

    void PresentPass::Execute(ComPtr<ID3D11DeviceContext> context, RenderGraph& graph) {
        if (!m_bEnabled) {
            return;
        }

        // Present to screen
        auto swapChain = DeviceManager::GetInstance().GetSwapChain();
        if (swapChain) {
            swapChain->Present(0, 0);
        }
    }
}
