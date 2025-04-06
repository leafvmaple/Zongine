#pragma once

#include <vector>
#include <DirectXMath.h>

namespace Zongine {
    struct BONE;

    class PhysicsSystem {
    public:
        void Tick(float fDeltaTime);

    private:
        void _InitializeDrivenBones(
            int parentIndex,
            std::vector<uint32_t>& drivenBones,
            const std::vector<BONE>& bones
        );
    };
}