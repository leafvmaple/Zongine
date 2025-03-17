#include "RenderSystem.h"

#include "Entities/EntityManager.h"
#include "Utilities/DeviceManager.h"
#include "Utilities/EffectManager.h"

#include "Components/ShaderComponent.h"
#include "Components/CameraComponent.h"
#include "Components/MeshComponent.h"
#include "Components/TransformComponent.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "FX11/inc/d3dx11effect.h"

#include "LAssert.h"

namespace Zongine {
    __declspec(align(16)) struct SHARED_SHADER_COMMON
    {
        __declspec(align(16)) struct SWITCH
        {
            int bEnableSunLight = 0;
            int bEnableConvertMap = 0;
            int bEnableIBL = 0;
            int bEnableFog = 0;
        };

        SWITCH Switch;
        CAMERA Camera;
    };

    bool RenderSystem::Initialize(const RenderSystemInfo& info) {
        m_EntityManager = info.entityManager;
        m_WindowManager = info.deviceManager;
        m_ShaderManager = info.shaderManager;
        m_StateManager = info.stateManager;
        m_EffectManager = info.effectManager;

        _InitializeConstantBuffer();

        return true;
    }

    void RenderSystem::Tick(float fDeltaTime) {
        auto context = m_WindowManager->GetImmediateContext();
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
        auto device = m_WindowManager->GetDevice();
        D3D11_BUFFER_DESC bufferDesc{};

        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = sizeof(SHARED_SHADER_COMMON);
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = 0;

        device->CreateBuffer(&bufferDesc, nullptr, m_SharedShaderCommonBuffer.GetAddressOf());
    }

    void RenderSystem::_UpdateConstantBuffer() {

    }

    void RenderSystem::_UpdateEffect() {
        const auto& entities = m_EntityManager->GetEntities<ShaderComponent>();
        for (const auto& [entityID, shaderComponent] : entities) {
            auto effect = m_EffectManager->LoadEffect(shaderComponent.Macro, shaderComponent.ShaderPath);
            ID3DX11EffectPass* effectPass = m_EffectManager->GetEffectPass(effect, RENDER_PASS::COLOR);
        }
    }
}


