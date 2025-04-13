#pragma once

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>

struct NvFlexExtAsset;

namespace Zongine {

    struct NvFlexComponent {
        std::string Path{};

        std::vector<float> InvMass{};

        std::shared_ptr<NvFlexExtAsset> asset{ nullptr };
        std::vector<DirectX::XMFLOAT4> Particles{};
        std::vector<int> Phases{};
    };

    void Initialize(NvFlexComponent& flexComponent, const std::string& path);
}