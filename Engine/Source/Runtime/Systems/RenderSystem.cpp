#include "RenderSystem.h"

#include "Entities/EntityManager.h"

#include "Components/MeshComponent.h"
#include "Components/TransformComponent.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "LAssert.h"

namespace Zongine {

    static const D3D_FEATURE_LEVEL FEATURE_LEVEL_ARRAY_0[] =
    {
        D3D_FEATURE_LEVEL_11_0,
    };

    static const D3D_FEATURE_LEVEL FEATURE_LEVEL_ARRAY_1[] =
    {
        D3D_FEATURE_LEVEL_11_1,
    };

    void RenderSystem::Initialize(RenderSystemInitInfo initInfo) {
        UINT uCreateDeviceFlag{};
        ID3D11Device* piDevice{};
        ID3D11DeviceContext* piImmediateContext{};

#if defined(DEBUG) || defined(_DEBUG)  
        uCreateDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, uCreateDeviceFlag,
            FEATURE_LEVEL_ARRAY_0, _countof(FEATURE_LEVEL_ARRAY_0),
            D3D11_SDK_VERSION,
            &piDevice, nullptr, &piImmediateContext
        );

        m_piDevice = std::shared_ptr<ID3D11Device>(piDevice,
            [](ID3D11Device* ptr) {
                ptr->Release();
            });
        m_piImmediateContext = std::shared_ptr<ID3D11DeviceContext>(piImmediateContext,
            [](ID3D11DeviceContext* ptr) {
                ptr->Release();
            });
    }

    void RenderSystem::Tick(float fDeltaTime) {
        auto entities = GEntityManager.GetEntities();
        for (auto& entity : entities) {
            auto& meshComponent = entity.GetComponent<MeshComponent>();
            auto& transformComponent = entity.GetComponent<TransformComponent>();

            for (auto& subMesh : meshComponent.SubMeshes) {
                auto& vertexBuffer = subMesh.VertexBuffer;
                auto& indexBuffer = subMesh.IndexBuffer;
                auto& subsets = subMesh.Subsets;
                auto rawBuffer = vertexBuffer.piBuffer.get();
                m_piImmediateContext->IASetVertexBuffers(0, 1, &rawBuffer, &vertexBuffer.uStride, &vertexBuffer.uOffset);
                m_piImmediateContext->IASetIndexBuffer(indexBuffer.piBuffer.get(), indexBuffer.eFormat, indexBuffer.uOffset);
                m_piImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                for (auto& subset : subsets) {
                    m_piImmediateContext->DrawIndexed(subset.uIndexCount, subset.uStartIndex, 0);
                }
            }
        }
    }

}


