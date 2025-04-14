#pragma once

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <wtypes.h>

namespace Zongine {
    using DirectX::PackedVector::XMCOLOR;

    __declspec(align(16)) struct SKIN_SUBSET_CONST {
        DirectX::XMFLOAT4A ModelColor;
        BOOL EnableAlphaTest;
        float AlphaReference;
        float AlphaReference2;
    };

    __declspec(align(16)) struct CAMERA_CONST {
        DirectX::XMFLOAT4X4A CameraView;
        DirectX::XMFLOAT4X4A CameraProject;
    };

    __declspec(align(16)) struct FLEX_VERTEX_EXT {
        DirectX::XMFLOAT4A FlexPosition;
        float MixFactor;
    };
}