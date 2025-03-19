#include "RenderSystem.h"

#include "Entities/EntityManager.h"
#include "Utilities/DeviceManager.h"
#include "Utilities/EffectManager.h"
#include "Utilities/StateManager.h"

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

constexpr int SHARED_BUFFER_SLOT = 0;

namespace Zongine {
    bool RenderSystem::Initialize(const RenderSystemInfo& info) {
        m_EntityManager = info.entityManager;
        m_DeviceManager = info.deviceManager;
        m_ShaderManager = info.shaderManager;
        m_StateManager = info.stateManager;
        m_EffectManager = info.effectManager;

        _InitializeSharedBuffer();

        return true;
    }

    void RenderSystem::Tick(float fDeltaTime) {
        auto context = m_DeviceManager->GetImmediateContext();
        auto& entities = m_EntityManager->GetEntities();

        for (auto& [entityID, entity]:entities) {
            if (!m_EntityManager->HasComponent<MeshComponent>(entity.GetID()))
                continue;
            auto& meshComponent = entity.GetComponent<MeshComponent>();
            auto& transformComponent = entity.GetComponent<TransformComponent>();
            auto& shaderComponent = entity.GetComponent<ShaderComponent>();
            auto& materialComponent = entity.GetComponent<MaterialComponent>();

            _UpdateModelBuffer(entity);

            auto& vertexBuffer = meshComponent.VertexBuffer;
            auto& indexBuffer = meshComponent.IndexBuffer;

            context->IASetVertexBuffers(0, 1, vertexBuffer.piBuffer.GetAddressOf(), &vertexBuffer.uStride, &vertexBuffer.uOffset);
            context->IASetIndexBuffer(indexBuffer.piBuffer.Get(), indexBuffer.eFormat, indexBuffer.uOffset);
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

                auto effectPass = m_EffectManager->GetEffectPass(subsetShader.Effect, shaderComponent.Pass);
                effectPass->Apply(0, m_DeviceManager->GetImmediateContext().Get());

                context->DrawIndexed(subsetMesh.uIndexCount, subsetMesh.uStartIndex, 0);
            }
        }
    }

    void RenderSystem::_InitializeSharedBuffer() {
        auto device = m_DeviceManager->GetDevice();
        D3D11_BUFFER_DESC bufferDesc{};

        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = sizeof(SHARED_SHADER_COMMON);
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = 0;

        device->CreateBuffer(&bufferDesc, nullptr, m_SharedBuffer.GetAddressOf());
    }

    void RenderSystem::_UpdateSharedBuffer() {
        D3D11_MAPPED_SUBRESOURCE resource{};

        auto context = m_DeviceManager->GetImmediateContext();
        auto entities = m_EntityManager->GetEntities<CameraComponent>();
        const auto& cameraComponent = entities.begin()->second;

        m_SharedShaderCommon.Camera = cameraComponent.Camera;

        context->Map(m_SharedBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        memcpy(resource.pData, &m_SharedShaderCommon, sizeof(SHARED_SHADER_COMMON));
        context->Unmap(m_SharedBuffer.Get(), 0);

        context->VSSetConstantBuffers(SHARED_BUFFER_SLOT, 1, m_SharedBuffer.GetAddressOf());
        context->HSSetConstantBuffers(SHARED_BUFFER_SLOT, 1, m_SharedBuffer.GetAddressOf());
        context->DSSetConstantBuffers(SHARED_BUFFER_SLOT, 1, m_SharedBuffer.GetAddressOf());
        context->PSSetConstantBuffers(SHARED_BUFFER_SLOT, 1, m_SharedBuffer.GetAddressOf());
        context->GSSetConstantBuffers(SHARED_BUFFER_SLOT, 1, m_SharedBuffer.GetAddressOf());
        context->CSSetConstantBuffers(SHARED_BUFFER_SLOT, 1, m_SharedBuffer.GetAddressOf());
    }

    void RenderSystem::_UpdateModelBuffer(const Entity& entity) {
        D3D11_MAPPED_SUBRESOURCE resource{};

        auto context = m_DeviceManager->GetImmediateContext();

        auto& transformComponent = entity.GetComponent<TransformComponent>();
        auto& shaderComponent = entity.GetComponent<ShaderComponent>();

        auto& rotation = transformComponent.Rotation;
        auto& buffer = shaderComponent.ModelBuffer;

        auto matrix = XMMatrixTransformation(
            g_XMZero,
            g_XMIdentityR3,
            XMLoadFloat3(&transformComponent.Scale),
            g_XMZero,
            XMQuaternionRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z)),
            XMLoadFloat3(&transformComponent.Position)
        );
#if 0

        context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        memcpy(resource.pData, &matrix, sizeof(XMFLOAT4X4));
        context->Unmap(buffer.Get(), 0);

        context->VSSetConstantBuffers(1, 1, buffer.GetAddressOf());
        context->HSSetConstantBuffers(1, 1, buffer.GetAddressOf());
        context->DSSetConstantBuffers(1, 1, buffer.GetAddressOf());
        context->PSSetConstantBuffers(1, 1, buffer.GetAddressOf());
        context->GSSetConstantBuffers(1, 1, buffer.GetAddressOf());
        context->CSSetConstantBuffers(1, 1, buffer.GetAddressOf());

#endif


    }
}


