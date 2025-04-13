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
            NVFlexVector<int> phases(m_FlexLib, flexComponent.Phases.size());

            particles.map();
            phases.map();

            for (auto& particle : flexComponent.Particles)
                particles.push_back(particle);

            for (auto& phase : flexComponent.Phases)
                phases.push_back(phase);

            particles.unmap();
            phases.unmap();

            NvFlexSetParticles(m_Solver, particles.buffer, nullptr);
            NvFlexSetPhases(m_Solver, phases.buffer, nullptr);
        });
    }
}