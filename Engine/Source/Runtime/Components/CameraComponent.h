#pragma once

#include "Entities/Entity.h"

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

using DirectX::XMMATRIX;
using DirectX::XMVECTOR;

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    __declspec(align(16)) struct CAMERA
    {
        XMMATRIX CameraView;
        XMMATRIX CameraProject;
    };

    struct __declspec(align(16)) CameraComponent {
        CAMERA Camera;

        struct PERSPECTIVE_PARAM
        {
            float fFovAngleY;
            float fAspectRatio;
        } Perspective;

        ComPtr<ID3D11Buffer> Buffer{};
    };
}