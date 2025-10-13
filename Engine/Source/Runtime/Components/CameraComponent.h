#pragma once

#include "../Include/Const.h"

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

using DirectX::XMMATRIX;

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    struct CameraComponent {
        CAMERA_CONST Matrix{};

        struct PERSPECTIVE_PARAM
        {
            float fFovAngleY;
            float fAspectRatio;
        } Perspective;

        // Third-person camera parameters
        float Distance = 200.0f;          // Distance from target
        float MinDistance = 50.0f;        // Minimum zoom distance
        float MaxDistance = 500.0f;       // Maximum zoom distance
    };
}