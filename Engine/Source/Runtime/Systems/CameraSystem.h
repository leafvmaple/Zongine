#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

namespace Zongine {
    class CameraSystem {
    public:
        bool Initialize();
        void Tick(float fDeltaTime);
    };
}