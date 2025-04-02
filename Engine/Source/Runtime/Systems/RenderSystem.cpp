#include "RenderSystem.h"

#include "Managers/Mananger.h"

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
    bool RenderSystem::Initialize(const ManagerList& info) {
        m_EntityManager = info.entityManager;
        m_DeviceManager = info.deviceManager;
        m_StateManager = info.stateManager;
        m_EffectManager = info.effectManager;
        m_ResourceManger = info.assetManager;

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

        auto mesh = m_ResourceManger->GetMeshAsset(meshComponent.Path);
        auto material = m_ResourceManger->GetMaterialAsset(materialComponent.Path);

        auto runtimeMacro = RUNTIME_MACRO_MESH;
        if (mesh->InputLayout == INPUT_LAYOUT_CI_SKINMESH)
            runtimeMacro = RUNTIME_MACRO_SKIN_MESH;

        auto shader = m_ResourceManger->GetShaderAsset(runtimeMacro, shaderComponent.Paths);

        shader->TransformMatrix->SetMatrix(reinterpret_cast<const float*>(&transformComponent.World));
        shader->BonesMatrix->SetMatrixArray(reinterpret_cast<const float*>(meshComponent.Offset.data()), 0, meshComponent.Offset.size());

        auto& vertexBuffer = mesh->Vertex;
        auto& indexBuffer = mesh->Index;

        auto inputLayout = m_EffectManager->GetInputLayout(runtimeMacro);

        context->IASetInputLayout(inputLayout.Get());
        context->IASetVertexBuffers(0, 1, vertexBuffer.Buffer.GetAddressOf(), &vertexBuffer.uStride, &vertexBuffer.uOffset);
        context->IASetIndexBuffer(indexBuffer.Buffer.Get(), indexBuffer.eFormat, indexBuffer.uOffset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        for (int i = 0; i < mesh->Subsets.size(); i++) {
            auto& subsetMesh = mesh->Subsets[i];
            auto& subsetShader = shader->Subsets[i];
            auto& subsetMaterial = material->Subsets[i];

            for (auto& [var, texture] : subsetMaterial.Textures) {
                auto it = subsetShader.Variables.find(var);
                if (it == subsetShader.Variables.end())
                    continue;
                it->second->SetResource(texture.Texture.Get());
            }

            auto rasterizerState = m_StateManager->GetRasterizerState(subsetMaterial.Rasterizer);
            context->RSSetState(rasterizerState.Get());

            if (m_CameraBuffer)
                subsetShader.Effect->GetConstantBufferByName("CAMERA_MATRIX")->SetConstantBuffer(m_CameraBuffer.Get());

            auto effectPass = m_EffectManager->GetEffectPass(subsetShader.Effect, shader->Pass);
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


