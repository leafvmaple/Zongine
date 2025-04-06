#pragma once

#include "Include/Enums.h"
#include "Mananger.h"

#include <array>
#include <d3d11.h>
#include <wrl/client.h>

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    class StateManager : public SingleManager<StateManager> {
    public:
        bool Initialize();

        ComPtr<ID3D11RasterizerState> GetRasterizerState(RASTERIZER_STATE_ID id) const { return m_Rasterizers[id]; }
        ComPtr<ID3D11SamplerState> GetSamplerState(SAMPLER_STATE_ID id) const { return m_Samplers[id]; }
        ComPtr<ID3D11DepthStencilState> GetDepthStencilState(DEPTHSTENCIL_STATE_ID id) const { return m_DepthStencils[id]; }

    private:
        std::array<ComPtr<ID3D11RasterizerState>, RASTERIZER_STATE_COUNT> m_Rasterizers{};
        std::array<ComPtr<ID3D11SamplerState>, SAMPLER_STATE_COUNT> m_Samplers{};
        std::array<ComPtr<ID3D11DepthStencilState>, DEPTH_WRITE_COUNT> m_DepthStencils{};
    };
}