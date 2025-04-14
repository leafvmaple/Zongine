#include "NVFlexSystem.h"

#include "../Managers/DeviceManager.h"
#include "../Entities/EntityManager.h"

#include "../Components/NVFlexComponent.h"

#pragma comment(lib, "NvFlexDebugD3D_x64.lib")
#pragma comment(lib, "NvFlexExtDebugD3D_x64.lib")

void FlexErorCallback(NvFlexErrorSeverity type, const char* msg, const char* file, int line) {
    switch (type) {
    default:
        break;
    }
}

namespace Zongine {
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
        solverDesc.maxParticles = 10000;

        m_Solver = NvFlexCreateSolver(m_FlexLib, &solverDesc);
        _InitializeParams();

        EntityManager::GetInstance().ForEach<NvFlexComponent>([this](auto entityID, auto& flexComponent) {
            flexComponent.Initialize();
            flexComponent.Content = std::make_unique<NvFlexContent>();
            flexComponent.Content->Particles = std::make_unique<NvFlexVector<DirectX::XMFLOAT4>>(m_FlexLib, flexComponent.Particles.size());
            flexComponent.Content->Phases = std::make_unique<NvFlexVector<int>>(m_FlexLib, flexComponent.Phases.size());

            auto particles = flexComponent.Content->Particles;
            auto phases = flexComponent.Content->Phases;

            particles->map();
            phases->map();

            for (auto& particle : flexComponent.Particles)
                particles->push_back(particle);

            for (auto& phase : flexComponent.Phases)
                phases->push_back(phase);

            particles->unmap();
            phases->unmap();
        });
    }

    void NvFlexSystem::Uninitialize() {
        if (m_Solver)
            NvFlexDestroySolver(m_Solver);

        if (m_FlexLib)
            NvFlexShutdown(m_FlexLib);
    }

    void NvFlexSystem::Tick(int nDeltaTime) {
        EntityManager::GetInstance().ForEach<NvFlexComponent>([this, nDeltaTime](auto entityID, auto& flexComponent) {
            NvFlexSetParticles(m_Solver, flexComponent.Content->Particles->buffer, nullptr);
            NvFlexSetPhases(m_Solver, flexComponent.Content->Phases->buffer, nullptr);
            NvFlexSetParams(m_Solver, m_FlexParams.get());

            NvFlexUpdateSolver(m_Solver, nDeltaTime / 1000.f, 1, false);

            NvFlexGetParticles(m_Solver, flexComponent.Content->Particles->buffer, nullptr);
        });
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