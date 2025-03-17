#pragma once

#include <d3d11.h>
#include <string>

#include "CameraComponent.h"


namespace Zongine {
    enum RUNTIME_MACRO {
        RUNTIME_MACRO_MESH,
        RUNTIME_MACRO_SKIN_MESH,
        RUNTIME_MACRO_TERRAIN,

        RUNTIME_MACRO_COUNT,
    };

    struct ShaderComponent {
        RUNTIME_MACRO Macro;
        std::string ShaderPath;
    };
}