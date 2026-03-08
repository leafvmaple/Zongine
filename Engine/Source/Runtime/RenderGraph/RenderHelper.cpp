#include "RenderHelper.h"

#include "../Managers/DeviceManager.h"
#include "../Managers/EffectManager.h"
#include "../Managers/StateManager.h"

#include "FX11/inc/d3dx11effect.h"
#include <iostream>

namespace Zongine {
    static bool g_firstRender = true;

    void SubmitRenderItem(ComPtr<ID3D11DeviceContext> context, const RenderItem& item) {
        SubmitRenderItem(context, item, item.Pass);
    }

    void SubmitRenderItem(ComPtr<ID3D11DeviceContext> context, const RenderItem& item, RENDER_PASS passOverride) {
        auto inputLayout = EffectManager::GetInstance().GetInputLayout(item.Macro);
        
        if (g_firstRender) {
            std::cout << "[RenderHelper] Submitting render item - Indices: " << item.Mesh->uIndexCount 
                      << ", StartIndex: " << item.Mesh->uStartIndex
                      << ", InputLayout: " << (inputLayout ? "OK" : "NULL")
                      << ", Macro: " << static_cast<int>(item.Macro) << std::endl;
        }
        
        context->IASetInputLayout(inputLayout.Get());
        context->IASetVertexBuffers(0,
            static_cast<UINT>(item.Vertex.Buffers.size()),
            item.Vertex.Buffers.data(),
            item.Vertex.Strides.data(),
            item.Vertex.Offsets.data());

        context->IASetIndexBuffer(item.Index.Buffer.Get(), item.Index.eFormat, item.Index.uOffset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        auto rasterizerState = StateManager::GetInstance().GetRasterizerState(item.Material->Rasterizer);
        context->RSSetState(rasterizerState.Get());

        auto blendState = StateManager::GetInstance().GetBlendState(item.Material->Blend);
        context->OMSetBlendState(blendState.Get(), nullptr, 0xFFFFFFFF);

        for (auto& [var, texture] : item.Material->Textures) {
            auto it = item.Shader->Variables.find(var);
            if (it == item.Shader->Variables.end())
                continue;
            it->second->SetResource(texture.Texture.Get());
        }

        auto effectPass = EffectManager::GetInstance().GetEffectPass(item.Shader->Effect, passOverride);
        
        if (g_firstRender) {
            std::cout << "[RenderHelper] EffectPass: " << (effectPass ? "OK" : "NULL") 
                      << ", Pass: " << static_cast<int>(passOverride) << std::endl;
        }
        
        if (!effectPass) {
            std::cerr << "[RenderHelper] ERROR: effectPass is NULL!" << std::endl;
            return;
        }
        
        effectPass->Apply(0, context.Get());

        context->DrawIndexed(item.Mesh->uIndexCount, item.Mesh->uStartIndex, 0);
        
        if (g_firstRender) {
            g_firstRender = false;
        }
    }
}
