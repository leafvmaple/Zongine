#pragma once

#include "Include/Enums.h"
#include "BaseComponent.h"

#include <d3d11.h>
#include <vector>
#include <memory>
#include <string>
#include <wrl/client.h>

#include "FX11/inc/d3dx11effect.h"

namespace Zongine {
    struct MaterialComponent {
        std::string Path{};
    };
}
