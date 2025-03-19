#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <array>
#include <memory>

namespace Zongine {
    class DeviceManager;

    enum INPUT_LAYOUT {
        INPUT_LAYOUT_CI_MESH,
        INPUT_LAYOUT_CI_SKINMESH,
        INPUT_LAYOUT_CI_TERRAIN,

        INPUT_LAYOUT_COUNT,
    };

    enum PIXEL_SHADER {
        PIXEL_SHADER_NULL,
        PIXEL_SHADER_UV1_NO_LIGHT,

        PIXEL_SHADER_COUNT,
    };

    enum VERTEX_SHADER {
        VERTEX_SHADER_CI_MESH,
        VERTEX_SHADER_CI_SKINMESH,
        VERTEX_SHADER_CI_TERRAIN,

        VERTEX_SHADER_COUNT,
    };

    using Microsoft::WRL::ComPtr;

    struct ShaderInitInfo {
        std::shared_ptr<DeviceManager> pDevice;
    };

    class ShaderManager {
    public:
        bool Initialize(const ShaderInitInfo& info);

        ComPtr<ID3D11VertexShader> GetVertexShader(VERTEX_SHADER id) const { return m_VertexShaders[id]; }
        ComPtr<ID3D11PixelShader> GetPixelShader(PIXEL_SHADER id) const { return m_PixelShaders[id]; }
        ComPtr<ID3D11InputLayout> GetInputLayout(INPUT_LAYOUT id) const { return m_InputLayouts[id]; }

    private:
        std::array<ComPtr<ID3D11VertexShader>, VERTEX_SHADER_COUNT> m_VertexShaders{};
        std::array<ComPtr<ID3D11PixelShader>, PIXEL_SHADER_COUNT> m_PixelShaders{};
        std::array<ComPtr<ID3D11InputLayout>, INPUT_LAYOUT_COUNT> m_InputLayouts{};
    };
}