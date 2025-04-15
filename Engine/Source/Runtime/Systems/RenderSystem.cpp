#include "RenderSystem.h"

#include "Entities/EntityManager.h"
#include "Managers/DeviceManager.h"
#include "Managers/AssetManager.h"
#include "Managers/EffectManager.h"
#include "Managers/StateManager.h"

#include "Components/ShaderComponent.h"
#include "Components/MeshComponent.h"
#include "Components/NVFlexComponent.h"
#include "components/MaterialComponent.h"
#include "Components/TransformComponent.h"
#include "Components/LandscapeRegionComponent.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "FX11/inc/d3dx11effect.h"

#include "LAssert.h"

using namespace DirectX;

constexpr int SHARED_BUFFER_SLOT = 13;

namespace Zongine {
    void RenderSystem::Tick(float fDeltaTime) {
        auto swapChain = DeviceManager::GetInstance().GetSwapChain();
        auto context = DeviceManager::GetInstance().GetImmediateContext();
        auto renderTargetView = DeviceManager::GetInstance().GetRenderTargetView();
        auto depthStencilView = DeviceManager::GetInstance().GetDepthStencilView();

        context->ClearRenderTargetView(renderTargetView.Get(), reinterpret_cast<const float*>(&Colors::White));
        context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        m_ActorRenderQueue.clear();
        _UpdateRenderQueue(EntityManager::GetInstance().GetRootEntity());

        for (auto& entity: m_ActorRenderQueue) {
            TickEntity(context, entity);
        }

        swapChain->Present(0, 0);
    }

    void RenderSystem::TickEntity(ComPtr<ID3D11DeviceContext> context, const Entity& entity) {
        D3D11_MAPPED_SUBRESOURCE resource{};
        XMFLOAT4X4A inverseTransform{};

        auto& meshComponent = entity.GetComponent<MeshComponent>();
        auto& transformComponent = entity.GetComponent<TransformComponent>();
        auto& shaderComponent = entity.GetComponent<ShaderComponent>();
        auto& materialComponent = entity.GetComponent<MaterialComponent>();

        auto mesh =  AssetManager::GetInstance().GetMeshAsset(meshComponent.Path);
        auto material = AssetManager::GetInstance().GetModelMaterialAsset(materialComponent.Path);

        auto runtimeMacro = mesh->Macro;

        auto shader = AssetManager::GetInstance().GetShaderAsset(runtimeMacro, shaderComponent.Paths);

        if (entity.HasComponent<NvFlexComponent>()) {
            auto& flexComponent = entity.GetComponent<NvFlexComponent>();
            auto flex = AssetManager::GetInstance().GetNvFlexAsset(flexComponent.Path);

            HRESULT hr = context->Map(flex->Buffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
            memcpy(resource.pData, flexComponent.FlexVertices.data(), sizeof(FLEX_VERTEX_EXT) * flexComponent.FlexVertices.size());
            context->Unmap(flex->Buffers[1].Get(), 0);
        }

        auto& vertexBuffer = mesh->Vertex;
        auto& indexBuffer = mesh->Index;

        auto inputLayout = EffectManager::GetInstance().GetInputLayout(runtimeMacro);

        context->IASetInputLayout(inputLayout.Get());
        if (runtimeMacro == RUNTIME_MACRO_FLEX_MESH) {
            auto flexComponent = entity.GetComponent<NvFlexComponent>();
            auto flex = AssetManager::GetInstance().GetNvFlexAsset(flexComponent.Path);

            ID3D11Buffer* rawBuffers[2] = { flex->Buffers[0].Get(), flex->Buffers[1].Get()};
            context->IASetVertexBuffers(0, 2, rawBuffers, flex->uStride, flex->uOffset);
        }
        else {
            context->IASetVertexBuffers(0, 1, vertexBuffer.Buffer.GetAddressOf(), &vertexBuffer.uStride, &vertexBuffer.uOffset);
        }
        context->IASetIndexBuffer(indexBuffer.Buffer.Get(), indexBuffer.eFormat, indexBuffer.uOffset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        XMStoreFloat4x4A(&inverseTransform, XMMatrixInverse(nullptr, XMLoadFloat4x4(&transformComponent.World)));

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

            auto rasterizerState = StateManager::GetInstance().GetRasterizerState(subsetMaterial.Rasterizer);
            context->RSSetState(rasterizerState.Get());

            if (m_CameraBuffer)
                subsetShader.CameraConst->SetConstantBuffer(m_CameraBuffer.Get());

            // TODO
            subsetShader.ModelConst->GetMemberByName("MATRIX_M")->SetRawValue(&transformComponent.World, 0, sizeof(transformComponent.World));
            subsetShader.ModelConst->GetMemberByName("MATRIX_INV_M")->SetRawValue(&inverseTransform, 0, sizeof(inverseTransform));

            subsetShader.ModelConst->GetMemberByName("MATRIX_BONES")->SetRawValue(
                meshComponent.SkinningTransforms.data(), 0, meshComponent.SkinningTransforms.size() * sizeof(DirectX::XMFLOAT4X4));

            subsetShader.SubsetConst->SetRawValue(&subsetMaterial.Const, 0, sizeof(SKIN_SUBSET_CONST));

            auto effectPass = EffectManager::GetInstance().GetEffectPass(subsetShader.Effect, shader->Pass);
            effectPass->Apply(0, context.Get());

            context->DrawIndexed(subsetMesh.uIndexCount, subsetMesh.uStartIndex, 0);
        }
    }

    void RenderSystem::TickTerrain(ComPtr<ID3D11DeviceContext> context, const Entity& entity) {

    }

    void RenderSystem::_UpdateRenderQueue(Entity& entity) {
        if (entity.HasComponent<MeshComponent>()) {
            m_ActorRenderQueue.push_back(entity);
        }
        if (entity.HasComponent<LandscapeRegionComponent>()) {
            m_TerrainRenderQueue.push_back(entity);
        }

        if (entity.HasComponent<CameraComponent>()) {
            D3D11_MAPPED_SUBRESOURCE resource{};

            auto context = DeviceManager::GetInstance().GetImmediateContext();

            auto& cameraComponent = entity.GetComponent<CameraComponent>();
            m_CameraBuffer = cameraComponent.Buffer;

            context->Map(m_CameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
            memcpy(resource.pData, &cameraComponent.Matrix, sizeof(CAMERA_CONST));
            context->Unmap(m_CameraBuffer.Get(), 0);
        }

        for (auto& id : entity.GetChildren()) {
            auto& child = EntityManager::GetInstance().GetEntity(id);

            _UpdateRenderQueue(child);
        }
    }
}


