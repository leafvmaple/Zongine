#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

namespace Zongine {
    struct ShaderSystemInfo {

    };

    class ShaderSystem {
    public:
        bool Initialize(const ShaderSystemInfo& info);
        void Tick(float fDeltaTime);
    };
}