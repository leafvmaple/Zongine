#pragma once

#include <vector>
#include <functional>

#include <DirectXMath.h>

namespace Zongine {
    using DirectX::XMFLOAT4;
    using DirectX::XMFLOAT3;

    void MergeVertex(std::vector<int>& ParticleVertexMap,
        std::vector<int>& VertiesParticlesMap,
        const std::vector<XMFLOAT4>& Verties,
        float cellSize, XMFLOAT3 offset,
        std::function<bool(int vertex1, int vertex2)> func);
}
