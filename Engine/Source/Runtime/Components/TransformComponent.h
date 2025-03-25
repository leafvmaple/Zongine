#pragma once

#include "BaseComponent.h"

#include <string>
#include <DirectXMath.h>

namespace Zongine {
    enum class BIND_TYPE {
        Attach,
        Socket,
    };

    struct TransformComponent : public BaseComponent {
        DirectX::XMMATRIX World{};

        DirectX::XMFLOAT3 Position{};
        DirectX::XMFLOAT3 Rotation{};
        DirectX::XMFLOAT3 Scale{ 1.0f, 1.0f, 1.0f };

        BIND_TYPE BindType { BIND_TYPE::Attach };
        std::string TargetName{};
    };
}