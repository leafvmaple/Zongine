#pragma once

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>

#include "../Include/const.h"

#include "NVFlex/include/NvFlexExt.h"

namespace Zongine {
    struct NvFlexContent {
        std::shared_ptr<NvFlexVector<DirectX::XMFLOAT4>> Particles;
        std::shared_ptr<NvFlexVector<int>> Phases;
    };

    struct NvFlexComponent {
        std::string Path{};
        std::string MeshPath{};

        std::vector<DirectX::XMFLOAT4> Particles{};
        std::vector<int> Phases{};

        std::shared_ptr<NvFlexContent> Content{};

        std::vector<FLEX_VERTEX_EXT> Vertices{};

        void Initialize();
    };
}