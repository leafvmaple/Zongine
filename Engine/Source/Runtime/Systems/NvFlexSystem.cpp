#include "NvFlexSystem.h"

#include "../Managers/DeviceManager.h"
#include "../Managers/AssetManager.h"
#include "../Entities/EntityManager.h"

#include "../Components/NVFlexComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SkeletonComponent.h"
#include "../Components/MeshComponent.h"

#include "NVFlex/core/perlin.h"

#pragma comment(lib, "NvFlexDebugD3D_x64.lib")
#pragma comment(lib, "NvFlexExtDebugD3D_x64.lib")

void FlexErrorCallback(NvFlexErrorSeverity type, const char* msg, const char* file, int line) {
    switch (type) {
    default:
        break;
    }
}

namespace Zongine {
    using DirectX::XMLoadFloat3;
    using DirectX::XMStoreFloat3;
    using DirectX::XMLoadFloat4x4;
    using DirectX::XMStoreFloat4x4A;
    using DirectX::XMVectorScale;
    using DirectX::XMVector3Normalize;

    void NvFlexSystem::Initialize() {
        NvFlexInitDesc initDesc{};
        NvFlexSolverDesc solverDesc{};
        
        initDesc.computeType = eNvFlexD3D11;
        initDesc.deviceIndex = -1;
        initDesc.enableExtensions = true;
        initDesc.renderDevice = DeviceManager::GetInstance().GetDevice().Get();
        initDesc.renderContext = DeviceManager::GetInstance().GetImmediateContext().Get();
        initDesc.computeContext = DeviceManager::GetInstance().GetImmediateContext().Get();
        initDesc.runOnRenderContext = false;

        m_FlexLib = NvFlexInit(NV_FLEX_VERSION, FlexErrorCallback, &initDesc);

        NvFlexSetSolverDescDefaults(&solverDesc);
        solverDesc.maxParticles = 100000;
        solverDesc.featureMode = eNvFlexFeatureModeSimpleSolids;

        m_Solver = NvFlexCreateSolver(m_FlexLib, &solverDesc);

        _InitializeParams();
    }

    void NvFlexSystem::Uninitialize() {
        if (m_Solver)
            NvFlexDestroySolver(m_Solver);

        if (m_FlexLib)
            NvFlexShutdown(m_FlexLib);
    }

    void NvFlexSystem::Tick(int nDeltaTime) {


        _UpdateWind(nDeltaTime);

        auto& entities = EntityManager::GetInstance().GetEntities<NvFlexComponent>();
        for (auto& [entityID, flexComponent] : entities) {
            auto& entity = EntityManager::GetInstance().GetEntity(entityID);

            if (!flexComponent.bInitialized) {
                NvFlexCopyDesc copyDesc{};

                flexComponent.Initialize(entity, m_FlexLib);

                copyDesc.elementCount = flexComponent.Content->Particles.size();

                NvFlexSetParticles(m_Solver, flexComponent.Content->Particles.buffer, &copyDesc);
                NvFlexSetRestParticles(m_Solver, flexComponent.Content->ResetParticles.buffer, &copyDesc);
                NvFlexSetNormals(m_Solver, flexComponent.Content->Normals.buffer, &copyDesc);
                NvFlexSetPhases(m_Solver, flexComponent.Content->Phases.buffer, &copyDesc);
                NvFlexSetVelocities(m_Solver, flexComponent.Content->Velocities.buffer, &copyDesc);

                NvFlexSetDynamicTriangles(m_Solver,
                    flexComponent.Content->Triangles.buffer,
                    flexComponent.Content->TriangleNormals.buffer,
                    flexComponent.Content->TriangleNormals.size());

                NvFlexSetSprings(m_Solver,
                    flexComponent.Content->springIndices.buffer,
                    flexComponent.Content->springLengths.buffer,
                    flexComponent.Content->springStiffness.buffer,
                    flexComponent.Content->springLengths.size());
            }

            NvFlexSetParticles(m_Solver, flexComponent.Content->Particles.buffer, nullptr);
            NvFlexSetPhases(m_Solver, flexComponent.Content->Phases.buffer, nullptr);
            NvFlexSetVelocities(m_Solver, flexComponent.Content->Velocities.buffer, nullptr);

            NvFlexSetActive(m_Solver, flexComponent.Content->Active.buffer, nullptr);
            NvFlexSetActiveCount(m_Solver, flexComponent.Content->Active.size());

            NvFlexSetParams(m_Solver, m_FlexParams.get());

            NvFlexUpdateSolver(m_Solver, nDeltaTime / 1000.f / 10, 2, false);

            NvFlexGetParticles(m_Solver, flexComponent.Content->Particles.buffer, nullptr);
            NvFlexGetPhases(m_Solver, flexComponent.Content->Phases.buffer, nullptr);
            NvFlexGetVelocities(m_Solver, flexComponent.Content->Velocities.buffer, nullptr);
        }

        for (auto& [entityID, flexComponent] : entities) {
            auto& particles = flexComponent.Content->Particles;
            auto flex = AssetManager::GetInstance().GetNvFlexAsset(flexComponent.Path);

            particles.map();

            for (int i = 0; i < flexComponent.Particles.size(); i++) {
                flexComponent.Particles[i] = particles[i];
            }

            for (int i = 0; i < flex->VertexParticleMap.size(); i++) {
                auto particle = flexComponent.Particles[flex->VertexParticleMap[i]];
                if (particle.w > 0)
                    flexComponent.FlexVertices[i].FlexPosition = particle;
            }

            particles.unmap();
        }
    }

    void NvFlexSystem::_InitializeParams() {
        m_FlexParams = std::make_unique<NvFlexParams>();

        m_FlexParams->numIterations = 4;
        m_FlexParams->gravity[0] = 0.0f;
        m_FlexParams->gravity[1] = -9.81f * 2;
        m_FlexParams->gravity[2] = 0.0f;
        m_FlexParams->radius = 0.05f;
        m_FlexParams->solidRestDistance = 0.05f;

        m_FlexParams->dynamicFriction = 0.025;
        m_FlexParams->staticFriction = 0.05;

        m_FlexParams->damping = 0.675f;
        m_FlexParams->drag = 0.375f;
        m_FlexParams->lift = 0.55f;
        m_FlexParams->cohesion = 0.025f;

        m_FlexParams->anisotropyScale = 1.0f;
        m_FlexParams->anisotropyMin = 0.1f;
        m_FlexParams->anisotropyMax = 2.0f;
        m_FlexParams->smoothing = 1.0f;
        m_FlexParams->solidPressure = 1.0f;
        m_FlexParams->buoyancy = 1.0f;

        m_FlexParams->diffuseThreshold = 100.f;
        m_FlexParams->diffuseBuoyancy = 1.f;
        m_FlexParams->diffuseDrag = 0.8f;
        m_FlexParams->diffuseBallistic = 16;
        m_FlexParams->diffuseLifetime = 2.f;

        m_FlexParams->collisionDistance = 0.025f;

        m_FlexParams->maxSpeed = FLT_MAX;
        m_FlexParams->maxAcceleration = FLT_MAX;
        m_FlexParams->relaxationMode = eNvFlexRelaxationLocal;
        m_FlexParams->relaxationFactor = 1.f;

        m_FlexParams->numPlanes = 1;
        (DirectX::XMFLOAT4&)m_FlexParams->planes[0] = { 0.0f, 1.0f, 0.0f, -0.01f };
    }

    void NvFlexSystem::_UpdateWind(int nDeltaTime) {
        m_WindTime += nDeltaTime / 1000.f;

        auto noise = Perlin1D(m_WindTime * 0.5f, 10, 0.25f);

        auto vNormalWind = XMVector3Normalize(XMLoadFloat3(&m_EnvironmentWind));
        auto wind = XMVectorScale(vNormalWind, 3.0f * 0.3f * std::fabs(noise));

        DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)m_FlexParams->wind, wind);
    }
}