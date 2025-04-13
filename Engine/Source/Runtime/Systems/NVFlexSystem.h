#pragma once

#include <memory>

#include "NVFlex/include/NvFlexExt.h"

namespace Zongine {
    class NvFlexSystem {
    public:
        void Initialize();
        void Uninitialize();

        void Tick(int nDeltaTime);
    private:
        void _InitializeParams();

        NvFlexLibrary* m_FlexLib{};
        NvFlexSolver* m_Solver{};

        std::unique_ptr<NvFlexParams> m_FlexParams{};
    };
}