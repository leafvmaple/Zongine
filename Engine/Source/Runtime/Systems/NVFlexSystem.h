#pragma once

#include <memory>

struct NvFlexLibrary;
struct NvFlexSolver;
struct NvFlexParams;

namespace Zongine {
    class NvFlexSystem {
    public:
        NvFlexSystem();
        ~NvFlexSystem();

        void Initialize();
        void Uninitialize();

        void Tick(int nDeltaTime);
    private:
        struct float4 {
            float x, y, z, w;
        };

        void _InitializeParams();

        NvFlexLibrary* m_FlexLib{};
        NvFlexSolver* m_Solver{};

        std::unique_ptr<NvFlexParams> m_FlexParams{};
    };
}