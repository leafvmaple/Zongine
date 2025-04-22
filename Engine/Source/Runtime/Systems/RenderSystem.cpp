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

    void RenderSystem::Initialize() {
        D3D11_BUFFER_DESC desc{};

        auto device = DeviceManager::GetInstance().GetDevice();

        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = sizeof(CAMERA_CONST);
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        device->CreateBuffer(&desc, nullptr, m_CameraBuffer.GetAddressOf());
    }

    void RenderSystem::Tick(float fDeltaTime) {
        ID3D11ShaderResourceView* nullSRVs[3]{};

        auto swapChain = DeviceManager::GetInstance().GetSwapChain();
        auto context = DeviceManager::GetInstance().GetImmediateContext();
        auto swapChainRTV = DeviceManager::GetInstance().GetSwapChainRTV();
        auto depthStencilView = DeviceManager::GetInstance().GetDepthStencilView();
        auto mainSRV = DeviceManager::GetInstance().GetMainSRV();
        auto accSRV = DeviceManager::GetInstance().GetOITAccSRV();
        auto weightSRV = DeviceManager::GetInstance().GetOITWeightSRV();
        auto mainRTV = DeviceManager::GetInstance().GetMainRTV();
        auto accRTV = DeviceManager::GetInstance().GetOITAccRTV();
        auto weightRTV = DeviceManager::GetInstance().GetOITWeightRTV();

        ID3D11RenderTargetView* RTVs[] = { accRTV.Get(), weightRTV.Get() };
        ID3D11ShaderResourceView* SRVs[] = { mainSRV.Get(), accSRV.Get(), weightSRV.Get() };

        context->ClearRenderTargetView(swapChainRTV.Get(), reinterpret_cast<const float*>(&Colors::White));
        context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        context->ClearRenderTargetView(accRTV.Get(), reinterpret_cast<const float*>(&Colors::Black));
        context->ClearRenderTargetView(weightRTV.Get(), reinterpret_cast<const float*>(&Colors::White));

        m_GBufferRenderQueue.clear();
        m_OITRenderQueue.clear();
        _UpdateRenderQueue(EntityManager::GetInstance().GetRootEntity());

        context->OMSetRenderTargets(1, mainRTV.GetAddressOf(), depthStencilView.Get());
        for (auto& renderEntity : m_GBufferRenderQueue) {
            TickRenderEntity(renderEntity, RENDER_PASS::COLOR);
        }

        context->OMSetRenderTargets(ARRAYSIZE(RTVs), RTVs, depthStencilView.Get());
        for (auto& renderEntity : m_OITRenderQueue) {
            TickRenderEntity(renderEntity, RENDER_PASS::OIT);
        }

        context->OMSetRenderTargets(1, swapChainRTV.GetAddressOf(), depthStencilView.Get());
        context->PSSetSamplers(0, 1, StateManager::GetInstance().GetSamplerState(SAMPLER_STATE_LINEAR3_CLAMP).GetAddressOf());

        context->OMSetBlendState(StateManager::GetInstance().GetBlendState(BLEND_STATE_COPY).Get(), nullptr, 0xFFFFFFFF);

        EffectManager::GetInstance().ApplyOIT();

        context->PSSetShaderResources(0, ARRAYSIZE(SRVs), SRVs);
        context->Draw(6, 0);
        context->PSSetShaderResources(0, ARRAYSIZE(SRVs), nullSRVs);

        swapChain->Present(0, 0);
    }

    void RenderSystem::TickRenderEntity(const RenderEntity& renderEntity, RENDER_PASS pass) {
        auto context = DeviceManager::GetInstance().GetImmediateContext();

        auto inputLayout = EffectManager::GetInstance().GetInputLayout(renderEntity.Macro);
        context->IASetInputLayout(inputLayout.Get());
        context->IASetVertexBuffers(0,
            renderEntity.Vertex.Buffers.size(),
            renderEntity.Vertex.Buffers.data(),
            renderEntity.Vertex.Strides.data(),
            renderEntity.Vertex.Offsets.data());

        context->IASetIndexBuffer(renderEntity.Index.Buffer.Get(), renderEntity.Index.eFormat, renderEntity.Index.uOffset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        auto rasterizerState = StateManager::GetInstance().GetRasterizerState(renderEntity.Material->Rasterizer);
        context->RSSetState(rasterizerState.Get());

        auto blendState = StateManager::GetInstance().GetBlendState(renderEntity.Material->Blend);
        context->OMSetBlendState(blendState.Get(), nullptr, 0xFFFFFFFF);

        for (auto& [var, texture] : renderEntity.Material->Textures) {
            auto it = renderEntity.Shader->Variables.find(var);
            if (it == renderEntity.Shader->Variables.end())
                continue;
            it->second->SetResource(texture.Texture.Get());
        }

        auto effectPass = EffectManager::GetInstance().GetEffectPass(renderEntity.Shader->Effect, pass);
        effectPass->Apply(0, context.Get());

        context->DrawIndexed(renderEntity.Mesh->uIndexCount, renderEntity.Mesh->uStartIndex, 0);
    }

    void RenderSystem::TickTerrain(ComPtr<ID3D11DeviceContext> context, const Entity& entity) {

    }

    void RenderSystem::_UpdateRenderQueue(Entity& entity) {
        if (entity.HasComponent<MeshComponent>()) {
            _AddRenderEntity(entity);
        }
        if (entity.HasComponent<LandscapeRegionComponent>()) {
            m_TerrainRenderQueue.push_back(entity);
        }

        if (entity.HasComponent<CameraComponent>()) {
            D3D11_MAPPED_SUBRESOURCE resource{};

            auto context = DeviceManager::GetInstance().GetImmediateContext();

            auto& cameraComponent = entity.GetComponent<CameraComponent>();

            context->Map(m_CameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
            memcpy(resource.pData, &cameraComponent.Matrix, sizeof(CAMERA_CONST));
            context->Unmap(m_CameraBuffer.Get(), 0);
        }

        for (auto& id : entity.GetChildren()) {
            auto& child = EntityManager::GetInstance().GetEntity(id);

            _UpdateRenderQueue(child);
        }
    }

    void RenderSystem::_AddRenderEntity(Entity& entity) {
        VertexVector vertex{};
        XMFLOAT4X4A inverseTransform{};
        D3D11_MAPPED_SUBRESOURCE resource{};

        auto context = DeviceManager::GetInstance().GetImmediateContext();

        auto& transformComponent = entity.GetComponent<TransformComponent>();
        auto& meshComponent = entity.GetComponent<MeshComponent>();
        auto& shaderComponent = entity.GetComponent<ShaderComponent>();
        auto& materialComponent = entity.GetComponent<MaterialComponent>();

        auto mesh = AssetManager::GetInstance().GetMeshAsset(meshComponent.Path);
        auto material = AssetManager::GetInstance().GetModelMaterialAsset(materialComponent.Path);
        auto shader = AssetManager::GetInstance().GetShaderAsset(mesh->Macro, shaderComponent.Paths);

        auto& vertexBuffer = mesh->Vertex;
        auto& indexBuffer = mesh->Index;

        XMStoreFloat4x4A(&inverseTransform, XMMatrixInverse(nullptr, XMLoadFloat4x4(&transformComponent.World)));

        vertex.Buffers.push_back(vertexBuffer.Buffer.Get());
        vertex.Strides.push_back(vertexBuffer.uStride);
        vertex.Offsets.push_back(vertexBuffer.uOffset);

        if (entity.HasComponent<NvFlexComponent>()) {
            auto& flexComponent = entity.GetComponent<NvFlexComponent>();
            auto flex = AssetManager::GetInstance().GetNvFlexAsset(flexComponent.Path);

            context->Map(flex->Buffers.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
            memcpy(resource.pData, flexComponent.FlexVertices.data(), sizeof(FLEX_VERTEX_EXT) * flexComponent.FlexVertices.size());
            context->Unmap(flex->Buffers.Get(), 0);

            vertex.Buffers.push_back(flex->Buffers.Get());
            vertex.Strides.push_back(flex->uStride);
            vertex.Offsets.push_back(flex->uOffset);
        }

        for (int i = 0; i < mesh->Subsets.size(); i++) {
            auto& subsetMesh = mesh->Subsets[i];
            auto& subsetShader = shader->Subsets[i];
            auto& subsetMaterial = material->Subsets[i];
            RenderEntity subsetEntity{};

            subsetEntity.Mesh = &mesh->Subsets[i];
            subsetEntity.Shader = &shader->Subsets[i];
            subsetEntity.Material = &material->Subsets[i];

            subsetEntity.Macro = mesh->Macro;
            subsetEntity.Index = mesh->Index;
            subsetEntity.Vertex = vertex;

            subsetShader.CameraConst->SetConstantBuffer(m_CameraBuffer.Get());

            subsetShader.ModelConst->GetMemberByName("MATRIX_M")->SetRawValue(&transformComponent.World, 0, sizeof(transformComponent.World));
            subsetShader.ModelConst->GetMemberByName("MATRIX_BONES")->SetRawValue(
                meshComponent.SkinningTransforms.data(), 0, meshComponent.SkinningTransforms.size() * sizeof(DirectX::XMFLOAT4X4)
            );

            if (mesh->Macro == RUNTIME_MACRO_FLEX_MESH)
                subsetShader.ModelConst->GetMemberByName("MATRIX_INV_M")->SetRawValue(&inverseTransform, 0, sizeof(inverseTransform));

            subsetShader.SubsetConst->SetRawValue(&subsetMaterial.Const, 0, sizeof(SKIN_SUBSET_CONST));

            if (subsetMaterial.Blend == BLEND_STATE_OIT)
                m_OITRenderQueue.emplace_back(subsetEntity);
            else
                m_GBufferRenderQueue.emplace_back(subsetEntity);
        }
    }

}


