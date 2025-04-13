#pragma once

#include <memory>

struct NvFlexLibrary;
struct NvFlexSolver;

namespace Zongine {
    class NvFlexSystem {
    public:
        void Initialize();
        void Uninitialize();

        void Tick(int nDeltaTime);
    private:
        struct float4 {
            float x, y, z, w;
        };

        NvFlexLibrary* m_FlexLib{};
        NvFlexSolver* m_Solver{};
    };
}