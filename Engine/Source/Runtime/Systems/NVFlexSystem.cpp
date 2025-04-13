#include "NVFlexSystem.h"

#include "../Managers/DeviceManager.h"
#include "../Entities/EntityManager.h"

#include "../Components/NVFlexComponent.h"

#include "NVFlex/include/NvFlexExt.h"

#pragma comment(lib, "NvFlexDebugD3D_x64.lib")
#pragma comment(lib, "NvFlexExtDebugD3D_x64.lib")

void FlexErorCallback(NvFlexErrorSeverity type, const char* msg, const char* file, int line) {
    switch (type) {
    default:
        break;
    }
}

namespace Zongine {
    NvFlexSystem::NvFlexSystem() {}
    NvFlexSystem::~NvFlexSystem() {}

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
    }

    void NvFlexSystem::Uninitialize() {
        if (m_Solver)
            NvFlexDestroySolver(m_Solver);

        if (m_FlexLib)
            NvFlexShutdown(m_FlexLib);
    }

    void NvFlexSystem::Tick(int nDeltaTime) {
        EntityManager::GetInstance().ForEach<NvFlexComponent>([this](auto entityID, auto& flexComponent) {
            NvFlexVector<DirectX::XMFLOAT4> particles(m_FlexLib, flexComponent.Particles.size());
            NvFlexVector<int> phases(m_FlexLib, flexComponent.Phases.size());

            particles.map();
            phases.map();

            for (auto& particle : flexComponent.Particles)
                particles.push_back(particle);

            for (auto& phase : flexComponent.Phases)
                phases.push_back(phase);

            particles.unmap();
            phases.unmap();

            NvFlexSetParams(m_Solver, m_FlexParams.get());
            NvFlexSetParticles(m_Solver, particles.buffer, nullptr);
            NvFlexSetPhases(m_Solver, phases.buffer, nullptr);
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