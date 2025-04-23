#include "NvFlexSystem.h"

#include "../Managers/DeviceManager.h"
#include "../Managers/AssetManager.h"
#include "../Entities/EntityManager.h"

#include "../Components/NVFlexComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SkeletonComponent.h"
#include "../Components/MeshComponent.h"

#include "Maths/perlin.h"

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
        NvFlexCopyDesc copyDesc{};

        _UpdateWind(nDeltaTime);

        auto& entities = EntityManager::GetInstance().GetEntities<NvFlexComponent>();
        for (auto& [entityID, flexComponent] : entities) {
            auto& entity = EntityManager::GetInstance().GetEntity(entityID);

            if (!flexComponent.bInitialized) {
                flexComponent.Initialize(entity, m_FlexLib);
            }

            copyDesc.elementCount = flexComponent.ParticleVertices.size();

            NvFlexSetParticles(m_Solver, flexComponent.Content->Particles.buffer, nullptr);
            NvFlexSetPhases(m_Solver, flexComponent.Content->Phases.buffer, nullptr);
            NvFlexSetVelocities(m_Solver, flexComponent.Content->Velocities.buffer, nullptr);

            NvFlexSetActiveCount(m_Solver, flexComponent.ParticleVertices.size());

            NvFlexSetParams(m_Solver, m_FlexParams.get());

            NvFlexUpdateSolver(m_Solver, nDeltaTime / 1000.f / 20, 2, false);

            NvFlexGetParticles(m_Solver, flexComponent.Content->Particles.buffer, nullptr);
            NvFlexGetPhases(m_Solver, flexComponent.Content->Phases.buffer, nullptr);
            NvFlexGetVelocities(m_Solver, flexComponent.Content->Velocities.buffer, nullptr);
        }

        for (auto& [entityID, flexComponent] : entities) {
            auto& particles = flexComponent.Content->Particles;

            particles.map();

            for (int particleID = 0; particleID < flexComponent.ParticleVertices.size(); particleID++) {
                auto vertexID = flexComponent.ParticleVertices[particleID];
                flexComponent.FlexVertices[vertexID].FlexPosition = particles[particleID];
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
        m_FlexParams->fluidRestDistance = 0.05f;

        m_FlexParams->dynamicFriction = 0.025;
        m_FlexParams->staticFriction = 0.05;
        m_FlexParams->drag = 0.375f;
        m_FlexParams->lift = 0.55f;
        m_FlexParams->cohesion = 0.025f;

        m_FlexParams->damping = 0.f;
        m_FlexParams->collisionDistance = 0.0125f;

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