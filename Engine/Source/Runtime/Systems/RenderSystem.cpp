#include "RenderSystem.h"

#include "Entities/EntityManager.h"
#include "Utilities/DeviceManager.h"
#include "Utilities/EffectManager.h"

#include "Components/ShaderComponent.h"
#include "Components/MeshComponent.h"
#include "Components/TransformComponent.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "FX11/inc/d3dx11effect.h"

#include "LAssert.h"

constexpr int SHARED_COMMON_SLOT = 0;

namespace Zongine {
    bool RenderSystem::Initialize(const RenderSystemInfo& info) {
        m_EntityManager = info.entityManager;
        m_DeviceManager = info.deviceManager;
        m_ShaderManager = info.shaderManager;
        m_StateManager = info.stateManager;
        m_EffectManager = info.effectManager;

        _InitializeConstantBuffer();

        return true;
    }

    void RenderSystem::Tick(float fDeltaTime) {
        auto context = m_DeviceManager->GetImmediateContext();
        auto entities = m_EntityManager->GetEntities();

        for (auto& [entityID, entity]:entities) {
            auto& meshComponent = entity.GetComponent<MeshComponent>();
            auto& transformComponent = entity.GetComponent<TransformComponent>();

            for (auto& subMesh : meshComponent.SubMeshes) {
                auto& vertexBuffer = subMesh.VertexBuffer;
                auto& indexBuffer = subMesh.IndexBuffer;
                auto& subsets = subMesh.Subsets;
                context->IASetVertexBuffers(0, 1, vertexBuffer.piBuffer.GetAddressOf(), &vertexBuffer.uStride, &vertexBuffer.uOffset);
                context->IASetIndexBuffer(indexBuffer.piBuffer.Get(), indexBuffer.eFormat, indexBuffer.uOffset);
                context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                for (auto& subset : subsets) {
                    context->DrawIndexed(subset.uIndexCount, subset.uStartIndex, 0);
                }
            }
        }
    }

    void RenderSystem::_InitializeConstantBuffer() {
        auto device = m_DeviceManager->GetDevice();
        D3D11_BUFFER_DESC bufferDesc{};

        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = sizeof(SHARED_SHADER_COMMON);
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = 0;

        device->CreateBuffer(&bufferDesc, nullptr, m_SharedShaderCommonBuffer.GetAddressOf());
    }

    void RenderSystem::_UpdateConstantBuffer() {
        D3D11_MAPPED_SUBRESOURCE resource{};
        auto context = m_DeviceManager->GetImmediateContext();

        auto entities = m_EntityManager->GetEntities<CameraComponent>();
        auto cameraComponent = entities.begin()->second;

        m_SharedShaderCommon.Camera = cameraComponent.Camera;

        context->Map(m_SharedShaderCommonBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        memcpy(resource.pData, &m_SharedShaderCommon, sizeof(SHARED_SHADER_COMMON));

        context->Unmap(m_SharedShaderCommonBuffer.Get(), 0);

        context->VSSetConstantBuffers(SHARED_COMMON_SLOT, 1, m_SharedShaderCommonBuffer.GetAddressOf());
        context->HSSetConstantBuffers(SHARED_COMMON_SLOT, 1, m_SharedShaderCommonBuffer.GetAddressOf());
        context->DSSetConstantBuffers(SHARED_COMMON_SLOT, 1, m_SharedShaderCommonBuffer.GetAddressOf());
        context->PSSetConstantBuffers(SHARED_COMMON_SLOT, 1, m_SharedShaderCommonBuffer.GetAddressOf());
        context->GSSetConstantBuffers(SHARED_COMMON_SLOT, 1, m_SharedShaderCommonBuffer.GetAddressOf());
        context->CSSetConstantBuffers(SHARED_COMMON_SLOT, 1, m_SharedShaderCommonBuffer.GetAddressOf());
    }

    void RenderSystem::_UpdateEffect() {
        const auto& entities = m_EntityManager->GetEntities<ShaderComponent>();
        for (const auto& [entityID, shaderComponent] : entities) {
            auto effect = m_EffectManager->LoadEffect(shaderComponent.Macro, shaderComponent.ShaderPath);
            ID3DX11EffectPass* effectPass = m_EffectManager->GetEffectPass(effect, RENDER_PASS::COLOR);
        }
    }
}


