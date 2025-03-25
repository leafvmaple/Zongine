#pragma once

#include <DirectXMath.h>

namespace Zongine {
    struct AnimationComponent {
        std::vector<DirectX::XMMATRIX> Matrix;
    };
}