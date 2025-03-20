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
    using Microsoft::WRL::ComPtr;

    struct _Texture {
        std::string Name;
        ComPtr<ID3D11ShaderResourceView> Texture;
    };

    struct ReferMaterial {
        std::string Path{};
        std::string ShaderName{};

        RASTERIZER_STATE_ID Rasterizer{};

        std::unordered_map<std::string, _Texture> Textures{};
    };

    struct MaterialComponent : public BaseComponent {
        std::vector<std::shared_ptr<ReferMaterial>> Subsets;
    };
}
