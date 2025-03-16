#include "RenderSystem.h"

#include "Entities/EntityManager.h"
#include "Utilities/DeviceManager.h"

#include "Components/CameraComponent.h"
#include "Components/MeshComponent.h"
#include "Components/TransformComponent.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

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

}


