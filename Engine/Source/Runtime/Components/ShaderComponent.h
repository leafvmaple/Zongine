#pragma once

#include <d3d11.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <wrl/client.h>

namespace Zongine {
    struct ShaderComponent {
        std::vector<std::string> Paths;
    };
}