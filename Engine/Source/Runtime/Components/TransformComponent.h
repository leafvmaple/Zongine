#pragma once

#include "BaseComponent.h"

#include <DirectXMath.h>

namespace Zongine {
    struct TransformComponent : public BaseComponent {
        DirectX::XMFLOAT3 Position{};
        DirectX::XMFLOAT3 Rotation{};
        DirectX::XMFLOAT3 Scale{ 1.0f, 1.0f, 1.0f };
    };
}