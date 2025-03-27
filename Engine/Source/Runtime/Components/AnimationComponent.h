#pragma once

#include <DirectXMath.h>

namespace Zongine {
    struct AnimationComponent {
        std::string Path{};
        int nPlayTime{};

        std::vector<DirectX::XMFLOAT4X4> Matrix{};
    };
}