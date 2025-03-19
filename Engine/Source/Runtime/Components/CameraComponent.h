#pragma once

#include "Entities/Entity.h"

#include <DirectXMath.h>
using DirectX::XMMATRIX;
using DirectX::XMVECTOR;

namespace Zongine {
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
    };
}