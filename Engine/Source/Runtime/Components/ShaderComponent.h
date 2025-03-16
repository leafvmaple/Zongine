#pragma once

#include <d3d11.h>

#include "CameraComponent.h"


namespace Zongine {
    __declspec(align(16)) struct SHARED_SHADER_COMMON
    {
        __declspec(align(16)) struct SWITCH
        {
            int bEnableSunLight = 0;
            int bEnableConvertMap = 0;
            int bEnableIBL = 0;
            int bEnableFog = 0;
        };

        __declspec(align(16)) struct CAMERA
        {
            DirectX::XMMATRIX CameraView;
            DirectX::XMMATRIX CameraProject;
        };

        SWITCH Switch;
        CAMERA Camera;
    };

    class ShaderComponent {

    };
}