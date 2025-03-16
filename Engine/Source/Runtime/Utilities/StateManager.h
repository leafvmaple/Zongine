#pragma once

#include <memory>
#include <array>
#include <d3d11.h>
#include <wrl/client.h>

namespace Zongine {
    enum RASTERIZER_STATE_ID {
        RASTERIZER_STATE_CULL_NONE,
        RASTERIZER_STATE_CULL_CLOCKWISE,

        RASTERIZER_STATE_COUNT,
    };

    enum SAMPLER_STATE_ID {
        SAMPLER_STATE_DEFAULT,

        SAMPLER_STATE_POINT_CLAMP,

        SAMPLER_STATE_COUNT
    };

    enum DEPTHSTENCIL_STATE_ID {
        ZWRITE_ENABLE,

        DEPTH_WRITE_COUNT
    };

    using Microsoft::WRL::ComPtr;

    class DeviceManager;

    struct StateManagerDesc {
        std::shared_ptr<DeviceManager> pDevice{};
    };

    class StateManager {
    public:
        bool Initialize(const StateManagerDesc& desc);
    private:
		std::array<ComPtr<ID3D11RasterizerState>, RASTERIZER_STATE_COUNT> m_Rasterizers{};
		std::array<ComPtr<ID3D11SamplerState>, SAMPLER_STATE_COUNT> m_Samplers{};
		std::array<ComPtr<ID3D11DepthStencilState>, DEPTH_WRITE_COUNT> m_DepthStencils{};
    };
}