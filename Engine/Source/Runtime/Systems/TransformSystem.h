#pragma once

#include "Entities/EntityManager.h"

#include <DirectXMath.h>

namespace Zongine {
    class TransformSystem {
    public:
        void Tick(float fDeltaTime);

    private:
        void _UpdateWorldTransformRecursive(Entity entity, const DirectX::XMFLOAT4X4& parentMatrix);
    };
}