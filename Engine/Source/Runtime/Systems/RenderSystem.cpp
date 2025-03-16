#include "RenderSystem.h"

#include "Entities/EntityManager.h"
#include "Utilities/DeviceManager.h"

#include "Components/MeshComponent.h"
#include "Components/TransformComponent.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "LAssert.h"

namespace Zongine {
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


