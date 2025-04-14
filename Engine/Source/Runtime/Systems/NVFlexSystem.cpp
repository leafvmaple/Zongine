#include "NVFlexSystem.h"

#include "../Managers/DeviceManager.h"
#include "../Managers/AssetManager.h"
#include "../Entities/EntityManager.h"

#include "../Components/NVFlexComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SkeletonComponent.h"
#include "../Components/MeshComponent.h"

#pragma comment(lib, "NvFlexDebugD3D_x64.lib")
#pragma comment(lib, "NvFlexExtDebugD3D_x64.lib")

void FlexErorCallback(NvFlexErrorSeverity type, const char* msg, const char* file, int line) {
    switch (type) {
    default:
        break;
    }
}

namespace Zongine {
    using DirectX::XMVector3Transform;
    using DirectX::XMVECTOR;
    using DirectX::XMVectorSetW;
    using DirectX::XMVectorSelect;
    using DirectX::XMVectorSelectControl;
    using DirectX::XMLoadFloat4x4;
    using DirectX::XMVectorScale;
    using DirectX::XMVECTOR;
    using DirectX::XMVectorAdd;

    void NvFlexSystem::Initialize() {
        NvFlexInitDesc initDesc{};
        NvFlexSolverDesc solverDesc{};
        
        initDesc.computeType = eNvFlexD3D11;
        initDesc.deviceIndex = -1;
        initDesc.enableExtensions = true;
        initDesc.renderDevice = DeviceManager::GetInstance().GetDevice().Get();
        initDesc.renderContext = DeviceManager::GetInstance().GetImmediateContext().Get();
        initDesc.runOnRenderContext = true;

        m_FlexLib = NvFlexInit(NV_FLEX_VERSION, FlexErorCallback, &initDesc);

        NvFlexSetSolverDescDefaults(&solverDesc);

        m_Solver = NvFlexCreateSolver(m_FlexLib, &solverDesc);
        _InitializeParams();

        auto& entities = EntityManager::GetInstance().GetEntities<NvFlexComponent>();
        for (auto& [entityID, flexComponent] : entities) {
            flexComponent.Initialize();
            flexComponent.Content = std::make_unique<NvFlexContent>(m_FlexLib, flexComponent.Phases.size());
        }
    }

    void NvFlexSystem::Uninitialize() {
        if (m_Solver)
            NvFlexDestroySolver(m_Solver);

        if (m_FlexLib)
            NvFlexShutdown(m_FlexLib);
    }

    void NvFlexSystem::Tick(int nDeltaTime) {
        auto& entities = EntityManager::GetInstance().GetEntities<NvFlexComponent>();
        for (auto& [entityID, flexComponent] : entities) {
            auto& entity = EntityManager::GetInstance().GetEntity(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();
            auto& meshComponent = entity.GetComponent<MeshComponent>();

            auto mesh = AssetManager::GetInstance().GetMeshAsset(meshComponent.Path);

            auto& particles = flexComponent.Content->Particles;
            auto& phases = flexComponent.Content->Phases;

            if (!Init) {

                particles.map();
                phases.map();

                for (int particleID = 0; particleID < flexComponent.ParticleVertices.size(); particleID++) {
                    auto vertexID = flexComponent.ParticleVertices[particleID];
                    const auto& vertex = mesh->Vertices[vertexID];

                    DirectX::XMVECTOR position = DirectX::XMVectorZero();
                    DirectX::XMVECTOR vertexPos = XMLoadFloat3(&vertex.Position);

                    for (int i = 0; i < 4; i++) {
                        auto boneIndex = vertex.BoneIndices[i];
                        if (boneIndex != 0xFF) {
                            auto weight = vertex.BoneWeights[i];
                            const auto& skinTransform = meshComponent.SkinningTransforms[boneIndex];
                            auto transform = XMVectorScale(XMVector3Transform(vertexPos, XMLoadFloat4x4(&skinTransform)), weight / FLEX_NORMALIZE_SCLAE);
                            position = XMVectorAdd(position, transform);
                        }
                    }

                    position = XMVector3Transform(position, XMLoadFloat4x4(&transformComponent.World));
                    XMStoreFloat4(&particles[particleID], XMVectorSetW(position, flexComponent.FlexVertices[vertexID].MixFactor));
                }

                for (int i = 0; i < flexComponent.Phases.size(); i++)
                    phases[i] = flexComponent.Phases[i];

                particles.unmap();
                phases.unmap();

                Init = true;
            }

            NvFlexSetParticles(m_Solver, particles.buffer, nullptr);
            NvFlexSetPhases(m_Solver, phases.buffer, nullptr);
            NvFlexSetVelocities(m_Solver, flexComponent.Content->Velocities.buffer, nullptr);
            NvFlexSetParams(m_Solver, m_FlexParams.get());

            NvFlexUpdateSolver(m_Solver, nDeltaTime / 1000.f, 1, false);

            NvFlexGetParticles(m_Solver, particles.buffer, nullptr);
            NvFlexGetVelocities(m_Solver, flexComponent.Content->Velocities.buffer, nullptr);

            particles.map();

            auto test2 = particles[1000];

            auto inverseWorld = XMMatrixInverse(nullptr, XMLoadFloat4x4(&transformComponent.World));

            for (int particleID = 0; particleID < flexComponent.ParticleVertices.size(); particleID++) {
                auto vertexID = flexComponent.ParticleVertices[particleID];
                auto& flexVertex = flexComponent.FlexVertices[vertexID];

                auto position = XMVectorSetW(XMLoadFloat4(&particles[particleID]), 1.0f);

                auto resultPos = XMVectorSelect(
                    XMVectorScale(XMVector4Transform(position, inverseWorld), FLEX_NORMALIZE_SCLAE),
                    XMLoadFloat4(&flexVertex.FlexPosition),
                    XMVectorSelectControl(0, 0, 0, 1)
                );

                XMStoreFloat4(&flexVertex.FlexPosition, resultPos);
            }

            particles.unmap();
        }
    }

    void NvFlexSystem::_InitializeParams() {

        m_FlexParams = std::make_unique<NvFlexParams>();

        m_FlexParams->numIterations = 4;
        m_FlexParams->gravity[0] = 0.0f;
        m_FlexParams->gravity[1] = -9.81f;
        m_FlexParams->gravity[2] = 0.0f;
        m_FlexParams->radius = 0.05f;

        m_FlexParams->dynamicFriction = 0.165f;
        m_FlexParams->drag = 0.3;
        m_FlexParams->lift = 0.0f;

        m_FlexParams->damping = 0.2f;
        m_FlexParams->collisionDistance = 0.0125f;

        m_FlexParams->maxSpeed = FLT_MAX;
        m_FlexParams->maxAcceleration = FLT_MAX;
    }
}