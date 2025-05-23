#pragma once

#include <vector>
#include <DirectXMath.h>

namespace Zongine {
    struct BONE;

    class PhysicsSystem {
    public:
        void Tick(float fDeltaTime);
    };
}