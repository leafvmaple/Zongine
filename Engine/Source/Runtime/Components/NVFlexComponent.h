#pragma once

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>

#include "../Include/const.h"

#include "NVFlex/include/NvFlexExt.h"

namespace Zongine {
    constexpr int FLEX_NORMALIZE_SCLAE = 100;

    struct NvFlexContent {
        std::shared_ptr<NvFlexVector<DirectX::XMFLOAT4>> Particles;
        std::shared_ptr<NvFlexVector<int>> Phases;
    };

    struct NvFlexComponent {
        std::string Path{};
        std::string MeshPath{};

        std::vector<int> ParticleVertices{};
        std::vector<int> Phases{};

        std::shared_ptr<NvFlexContent> Content{};

        std::vector<FLEX_VERTEX_EXT> FlexVertices{};

        void Initialize();
    };
}