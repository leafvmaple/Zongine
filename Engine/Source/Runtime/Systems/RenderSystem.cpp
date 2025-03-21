#include "RenderSystem.h"

#include "Entities/EntityManager.h"
#include "Utilities/DeviceManager.h"
#include "Utilities/EffectManager.h"
#include "Utilities/StateManager.h"
#include "Utilities/ShaderManager.h"

#include "Components/ShaderComponent.h"
#include "Components/MeshComponent.h"
#include "components/MaterialComponent.h"
#include "Components/TransformComponent.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "FX11/inc/d3dx11effect.h"

#include "LAssert.h"

using namespace DirectX;

constexpr int SHARED_BUFFER_SLOT = 13;

namespace Zongine {
    bool RenderSystem::Initialize(const RenderSystemInfo& info) {
        m_EntityManager = info.entityManager;
        m_DeviceManager = info.deviceManager;
        m_ShaderManager = info.shaderManager;
        m_StateManager = info.stateManager;
        m_EffectManager = info.effectManager;

        return true;
    }

    void RenderSystem::Tick(float fDeltaTime) {
        auto swapChain = m_DeviceManager->GetSwapChain();
        auto context = m_DeviceManager->GetImmediateContext();
        auto renderTargetView = m_DeviceManager->GetRenderTargetView();
        auto depthStencilView = m_DeviceManager->GetDepthStencilView();

        context->ClearRenderTargetView(renderTargetView.Get(), reinterpret_cast<const float*>(&Colors::White));
        context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        m_RenderQueue.clear();
        _UpdateRenderQueue(m_EntityManager->GetRootEntity());

        for (auto& entity: m_RenderQueue) {
            TickEntity(context, entity);
        }

        swapChain->Present(0, 0);
    }

    void RenderSystem::TickEntity(ComPtr<ID3D11DeviceContext> context, const Entity& entity) {
        D3D11_MAPPED_SUBRESOURCE resource{};

        auto& meshComponent = entity.GetComponent<MeshComponent>();
        auto& transformComponent = entity.GetComponent<TransformComponent>();
        auto& shaderComponent = entity.GetComponent<ShaderComponent>();
        auto& materialComponent = entity.GetComponent<MaterialComponent>();

        auto& buffer = shaderComponent.ModelBuffer;

        context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        memcpy(resource.pData, &transformComponent.World, sizeof(transformComponent.World));
        context->Unmap(buffer.Get(), 0);

        auto& vertexBuffer = meshComponent.VertexBuffer;
        auto& indexBuffer = meshComponent.IndexBuffer;

        // TODO
        auto inputLayout = m_ShaderManager->GetInputLayout(INPUT_LAYOUT_CI_SKINMESH);

        context->IASetInputLayout(inputLayout.Get());
        context->IASetVertexBuffers(0, 1, vertexBuffer.Buffer.GetAddressOf(), &vertexBuffer.uStride, &vertexBuffer.uOffset);
        context->IASetIndexBuffer(indexBuffer.Buffer.Get(), indexBuffer.eFormat, indexBuffer.uOffset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        for (int i = 0; i < meshComponent.Subsets.size(); i++) {
            auto& subsetMesh = meshComponent.Subsets[i];
            auto& subsetShader = shaderComponent.Subsets[i];
            auto& subsetMaterial = materialComponent.Subsets[i];

            for (auto& [var, texture] : subsetMaterial->Textures) {
                auto it = subsetShader.Variables.find(var);
                if (it == subsetShader.Variables.end())
                    continue;
                it->second->SetResource(texture.Texture.Get());
            }

            auto rasterizerState = m_StateManager->GetRasterizerState(subsetMaterial->Rasterizer);
            context->RSSetState(rasterizerState.Get());

            if (m_CameraBuffer)
                subsetShader.Effect->GetConstantBufferByName("CAMERA_MATRIX")->SetConstantBuffer(m_CameraBuffer.Get());

            auto effectPass = m_EffectManager->GetEffectPass(subsetShader.Effect, shaderComponent.Pass);
            effectPass->Apply(0, context.Get());

            context->DrawIndexed(subsetMesh.uIndexCount, subsetMesh.uStartIndex, 0);
        }
    }

    void RenderSystem::_UpdateRenderQueue(Entity& entity) {
        if (m_EntityManager->HasComponent<MeshComponent>(entity.GetID())) {
            m_RenderQueue.push_back(entity);
        }

        if (m_EntityManager->HasComponent<CameraComponent>(entity.GetID())) {
            D3D11_MAPPED_SUBRESOURCE resource{};
            auto context = m_DeviceManager->GetImmediateContext();

            auto& cameraComponent = entity.GetComponent<CameraComponent>();
            m_CameraBuffer = cameraComponent.Buffer;

            context->Map(m_CameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
            memcpy(resource.pData, &cameraComponent.Camera, sizeof(CAMERA));
            context->Unmap(m_CameraBuffer.Get(), 0);
        }

        for (auto& id : entity.GetChildren()) {
            auto& child = m_EntityManager->GetEntity(id);

            _UpdateRenderQueue(child);
        }
    }
}


