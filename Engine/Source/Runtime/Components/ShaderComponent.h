#pragma once

#include "Include/Enums.h"

#include <d3d11.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <wrl/client.h>

#include "FX11/inc/d3dx11effect.h"

namespace Zongine {
    struct ShaderComponent {
        std::string Path;
    };
}