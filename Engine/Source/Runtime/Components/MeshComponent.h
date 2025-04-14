#pragma once

#include <memory>
#include <vector>
#include <string>
#include <d3d11.h>
#include <DirectXMath.h>

namespace Zongine {
    struct MeshComponent {
        std::string Path{};

        std::vector<DirectX::XMFLOAT4X4> BoneModelTransforms{};
        std::vector<DirectX::XMFLOAT4X4> SkinningTransforms{};
    };
}
