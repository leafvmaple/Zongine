#pragma once

#include "Include/Enums.h"

#include <unordered_map>
#include <string>
#include <array>
#include <wrl/client.h>
#include <memory>

#include "Components/ShaderComponent.h"

#include "FX11/inc/d3dx11effect.h"

namespace Zongine {
    class DeviceManager;

    using Microsoft::WRL::ComPtr;

    struct EffectManagerInfo {
        std::shared_ptr<DeviceManager> deviceManager{};
    };

    class EffectManager {
    public:
        void Initialize(const EffectManagerInfo& info);

        ComPtr<ID3DX11Effect> LoadEffect(RUNTIME_MACRO macro, const std::string& path);

        void LoadVariables(ComPtr<ID3DX11Effect> effect, std::unordered_map<std::string, ID3DX11EffectShaderResourceVariable*>& Variables);

        ComPtr<ID3D11InputLayout> GetInputLayout(RUNTIME_MACRO macro) { return m_InputLayouts[macro]; }
        ID3DX11EffectPass* GetEffectPass(ComPtr<ID3DX11Effect> effect, RENDER_PASS pass);

    private:
        struct RENDER_PASS_TABLE {
            const char* szTechniqueName;
            unsigned    uPassSlot = 0;
        };

        std::shared_ptr<DeviceManager> m_DeviceManager{};
        std::array<std::unordered_map<std::string, ComPtr<ID3DX11Effect>>, RUNTIME_MACRO_COUNT> m_Effects{};
        std::array<ComPtr<ID3D11InputLayout>, RUNTIME_MACRO_COUNT> m_InputLayouts{};

        std::unordered_map<RENDER_PASS, RENDER_PASS_TABLE> m_RenderPassTable {
            { RENDER_PASS::COLOR, { "Color" } },
            { RENDER_PASS::COLORSOFTMASK, { "ColorSoftMask"} }
        };
    };
}