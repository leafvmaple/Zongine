#pragma once

#include <memory>
#include <DirectXMath.h>

#include "NVFlex/include/NvFlexExt.h"

namespace Zongine {
    class NvFlexSystem {
    public:
        void Initialize();
        void Uninitialize();

        void Tick(int nDeltaTime);
    private:
        void _InitializeParams();

        void _UpdateWind(int nDeltaTime);

        NvFlexLibrary* m_FlexLib{};
        NvFlexSolver* m_Solver{};

        bool Init{};

        std::unique_ptr<NvFlexParams> m_FlexParams{};

        float m_WindTime{};
        DirectX::XMFLOAT3 m_EnvironmentWind{ -1.f, 0.f, 0.f };
    };
}