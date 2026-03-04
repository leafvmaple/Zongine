#pragma once

#include "Include/Types.h"

#include <DirectXMath.h>

namespace Zongine {
    class TransformSystem {
    public:
        void Tick(float fDeltaTime);

    private:
        void _UpdateWorldTransformRecursive(EntityID entityID, const DirectX::XMFLOAT4X4& parentMatrix);
    };
}