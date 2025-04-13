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

        ComPtr<ID3D11Buffer> Buffer{};
    };
}