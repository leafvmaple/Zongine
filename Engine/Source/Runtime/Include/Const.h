#pragma once

#include <DirectXMath.h>
#include <wtypes.h>

namespace Zongine {
    __declspec(align(16)) struct SKIN_SUBSET_CONST {
        DirectX::XMFLOAT4A ModelColor;
        BOOL        EnableAlphaTest;
        float       AlphaReference;
        float       AlphaReference2;
    };

    __declspec(align(16)) struct CAMERA_CONST {
        DirectX::XMFLOAT4X4 CameraView;
        DirectX::XMFLOAT4X4 CameraProject;
    };
}