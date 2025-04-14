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
        NvFlexVector<DirectX::XMFLOAT4> Particles;
        NvFlexVector<int> Phases;
        NvFlexVector<DirectX::XMFLOAT3> Velocities;

        NvFlexContent(NvFlexLibrary* l, int size) : Particles(l, size) , Phases(l, size), Velocities(l, size){ }
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