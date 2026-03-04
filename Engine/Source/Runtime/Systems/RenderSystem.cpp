#include "RenderSystem.h"

#include "Entities/World.h"
#include "Managers/DeviceManager.h"
#include "Managers/AssetManager.h"
#include "Managers/EffectManager.h"
#include "Managers/StateManager.h"

#include "Components/CameraComponent.h"
#include "Components/ShaderComponent.h"
#include "Components/MeshComponent.h"
#include "Components/NVFlexComponent.h"
#include "Components/MaterialComponent.h"
#include "Components/TransformComponent.h"
#include "Components/LandscapeRegionComponent.h"

#include "RenderGraph/RenderGraph.h"
#include "RenderGraph/RenderPasses.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "FX11/inc/d3dx11effect.h"

#include "LAssert.h"

using namespace DirectX;

constexpr int SHARED_BUFFER_SLOT = 13;

namespace Zongine {

    RenderSystem::RenderSystem() = default;
    
    RenderSystem::~RenderSystem() = default;

    void RenderSystem::Initialize() {
        D3D11_BUFFER_DESC desc{};

        auto device = DeviceManager::GetInstance().GetDevice();

        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = sizeof(CAMERA_CONST);
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        device->CreateBuffer(&desc, nullptr, m_RenderContext.CameraBuffer.GetAddressOf());
        
        // Initialize RenderGraph
        InitializeRenderGraph();
    }
    
    void RenderSystem::InitializeRenderGraph() {
        m_RenderGraph = std::make_unique<RenderGraph>();
        
        auto& deviceMgr = DeviceManager::GetInstance();
        const auto& viewport = deviceMgr.GetViewport();
        UINT width = static_cast<UINT>(viewport.Width);
        UINT height = static_cast<UINT>(viewport.Height);
        
        // Import external resources
        m_RenderGraph->ImportRenderTarget("SwapChainRT", deviceMgr.GetSwapChainRTV());
        m_RenderGraph->ImportDepthStencil("DepthStencil", deviceMgr.GetDepthStencilView());
        m_RenderGraph->ImportRenderTarget("MainRT", deviceMgr.GetMainRTV());
        m_RenderGraph->ImportRenderTarget("OITAccumulation", deviceMgr.GetOITAccRTV());
        m_RenderGraph->ImportRenderTarget("OITWeight", deviceMgr.GetOITWeightRTV());
        
        // Add render passes — passes no longer need a RenderSystem pointer
        // 0. Clear SwapChain - Black background
        auto clearSwapChain = m_RenderGraph->AddPass<ClearPass>("ClearSwapChain", "SwapChainRT", Colors::Black);
        clearSwapChain->SetClearDepth(false);
        
        // 1. Clear main render target and depth buffer - Black background
        auto clearMainRT = m_RenderGraph->AddPass<ClearPass>("ClearMainRT", "MainRT", Colors::Black);
        
        // 2. Opaque Pass
        auto opaquePass = m_RenderGraph->AddPass<OpaquePass>("OpaquePass");
        
        // 3. Clear OIT buffers
        auto clearOITAcc = m_RenderGraph->AddPass<ClearPass>("ClearOITAcc", "OITAccumulation", Colors::Black);
        clearOITAcc->SetClearDepth(false);
        
        auto clearOITWeight = m_RenderGraph->AddPass<ClearPass>("ClearOITWeight", "OITWeight", Colors::White);
        clearOITWeight->SetClearDepth(false);
        
        // 4. OIT Pass
        auto oitPass = m_RenderGraph->AddPass<OITPass>("OITPass");
        
        // 5. Composite Pass
        auto compositePass = m_RenderGraph->AddPass<CompositePass>("CompositePass");
        
        // 6. Present Pass
        auto presentPass = m_RenderGraph->AddPass<PresentPass>("PresentPass");
        
        // Share the per-frame RenderContext with the graph
        m_RenderGraph->SetRenderContext(&m_RenderContext);

        // Compile render graph
        m_RenderGraph->Compile();
    }

    void RenderSystem::Tick(float fDeltaTime) {
        auto context = DeviceManager::GetInstance().GetImmediateContext();
        const auto& viewport = DeviceManager::GetInstance().GetViewport();
        context->RSSetViewports(1, &viewport);
        
        auto rootID = World::GetInstance().GetRootEntity();

        // 1. Update camera constant buffer (separated from queue building)
        _UpdateCamera(rootID);

        // 2. Build render queues
        m_RenderContext.OpaqueQueue.clear();
        m_RenderContext.OITQueue.clear();
        m_RenderContext.TerrainQueue.clear();
        _UpdateRenderQueue(rootID);
        
        // 3. Execute RenderGraph
        if (m_RenderGraph) {
            m_RenderGraph->Execute(context);
        }
    }

    // ---------- Camera update (extracted from _UpdateRenderQueue) ----------

    void RenderSystem::_UpdateCamera(EntityID entityID) {
        auto& world = World::GetInstance();

        if (world.Has<CameraComponent>(entityID)) {
            D3D11_MAPPED_SUBRESOURCE resource{};
            auto context = DeviceManager::GetInstance().GetImmediateContext();
            auto& cameraComponent = world.Get<CameraComponent>(entityID);

            context->Map(m_RenderContext.CameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
            memcpy(resource.pData, &cameraComponent.Matrix, sizeof(CAMERA_CONST));
            context->Unmap(m_RenderContext.CameraBuffer.Get(), 0);
        }

        for (auto& childID : world.GetChildren(entityID)) {
            _UpdateCamera(childID);
        }
    }

    // ---------- Render queue building ----------

    void RenderSystem::_UpdateRenderQueue(EntityID entityID) {
        auto& world = World::GetInstance();
        if (world.Has<MeshComponent>(entityID)) {
            _AddRenderEntity(entityID);
        }
        if (world.Has<LandscapeRegionComponent>(entityID)) {
            m_RenderContext.TerrainQueue.push_back(entityID);
        }

        for (auto& childID : world.GetChildren(entityID)) {
            _UpdateRenderQueue(childID);
        }
    }

    void RenderSystem::_AddRenderEntity(EntityID entityID) {
        auto& world = World::GetInstance();
        VertexVector vertex{};
        XMFLOAT4X4A inverseTransform{};
        D3D11_MAPPED_SUBRESOURCE resource{};

        auto context = DeviceManager::GetInstance().GetImmediateContext();

        auto& transformComponent = world.Get<TransformComponent>(entityID);
        auto& meshComponent = world.Get<MeshComponent>(entityID);
        auto& shaderComponent = world.Get<ShaderComponent>(entityID);
        auto& materialComponent = world.Get<MaterialComponent>(entityID);

        auto mesh = AssetManager::GetInstance().GetMeshAsset(meshComponent.Path);
        auto material = AssetManager::GetInstance().GetModelMaterialAsset(materialComponent.Path);
        auto shader = AssetManager::GetInstance().GetShaderAsset(mesh->Macro, shaderComponent.Paths);

        auto& vertexBuffer = mesh->Vertex;
        auto& indexBuffer = mesh->Index;

        XMStoreFloat4x4A(&inverseTransform, XMMatrixInverse(nullptr, XMLoadFloat4x4(&transformComponent.World)));

        vertex.Buffers.push_back(vertexBuffer.Buffer.Get());
        vertex.Strides.push_back(vertexBuffer.uStride);
        vertex.Offsets.push_back(vertexBuffer.uOffset);

        if (world.Has<NvFlexComponent>(entityID)) {
            auto& flexComponent = world.Get<NvFlexComponent>(entityID);
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
            RenderItem subsetItem{};

            subsetItem.Mesh = &mesh->Subsets[i];
            subsetItem.Shader = &shader->Subsets[i];
            subsetItem.Material = &material->Subsets[i];

            subsetItem.Macro = mesh->Macro;
            subsetItem.Index = mesh->Index;
            subsetItem.Vertex = vertex;

            subsetItem.Pass = RENDER_PASS::COLOR;
            if (subsetMaterial.Blend == BLEND_STATE_SOFTMASKED)
                subsetItem.Pass = RENDER_PASS::COLORSOFTMASK;
            else if (subsetMaterial.Blend == BLEND_STATE_OIT)
                subsetItem.Pass = RENDER_PASS::OIT;

            subsetShader.CameraConst->SetConstantBuffer(m_RenderContext.CameraBuffer.Get());

            subsetShader.ModelConst->GetMemberByName("MATRIX_M")->SetRawValue(&transformComponent.World, 0, sizeof(transformComponent.World));
            subsetShader.ModelConst->GetMemberByName("MATRIX_BONES")->SetRawValue(
                meshComponent.SkinningTransforms.data(), 0, meshComponent.SkinningTransforms.size() * sizeof(DirectX::XMFLOAT4X4)
            );

            if (mesh->Macro == RUNTIME_MACRO_FLEX_MESH)
                subsetShader.ModelConst->GetMemberByName("MATRIX_INV_M")->SetRawValue(&inverseTransform, 0, sizeof(inverseTransform));

            subsetShader.SubsetConst->SetRawValue(&subsetMaterial.Const, 0, sizeof(SKIN_SUBSET_CONST));

            if (subsetMaterial.Blend == BLEND_STATE_OIT)
                m_RenderContext.OITQueue.emplace_back(subsetItem);
            else
                m_RenderContext.OpaqueQueue.emplace_back(subsetItem);
        }
    }

}
