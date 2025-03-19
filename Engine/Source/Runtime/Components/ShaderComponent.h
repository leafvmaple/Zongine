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

    using Microsoft::WRL::ComPtr;

    struct SubsetShader {
        std::string ShaderPath{};

        ComPtr<ID3DX11Effect> Effect{};
        std::unordered_map<std::string, ID3DX11EffectShaderResourceVariable*> Variables{};
    };

    struct ShaderComponent {
        std::vector<SubsetShader> Subsets{};

        RUNTIME_MACRO Macro{};
        RENDER_PASS Pass{};

        ComPtr<ID3D11Buffer> ModelBuffer{};

        std::vector<ComPtr<ID3D11Buffer>> SubsetBuffers{};
    };
}