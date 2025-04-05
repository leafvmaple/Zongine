#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

namespace Zongine {
    struct MeshComponent {
        std::string Path{};

        std::vector<DirectX::XMFLOAT4X4> BoneModelTransforms{};
        std::vector<DirectX::XMFLOAT4X4> SkinningTransforms{};
    };
}