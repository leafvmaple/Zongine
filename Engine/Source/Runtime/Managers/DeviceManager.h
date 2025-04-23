#pragma once

#include "Mananger.h"

#include <d3d11.h>
#include <wrl/client.h>

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    class DeviceManager : public SingleManager<DeviceManager> {
    public:
        void Initialize();
        void Resize();

        ComPtr<ID3D11Device> GetDevice() const { return m_piDevice; }
        ComPtr<ID3D11DeviceContext> GetImmediateContext() const { return m_piImmediateContext; }
        ComPtr<IDXGISwapChain> GetSwapChain() const { return m_piSwapChain; }
        ComPtr<ID3D11DepthStencilView> GetDepthStencilView() const { return m_piDepthStencilView; }

        ComPtr<ID3D11RenderTargetView> GetSwapChainRTV() const { return m_SwapChainRTV; }
        ComPtr<ID3D11RenderTargetView> GetMainRTV() const { return m_MainRTV; }
        ComPtr<ID3D11RenderTargetView> GetOITAccRTV() const { return m_OITAccRTV; }
        ComPtr<ID3D11RenderTargetView> GetOITWeightRTV() const { return m_OITWeightRTV; }

        ComPtr<ID3D11ShaderResourceView> GetMainSRV() const { return m_MainSRV; };
        ComPtr<ID3D11ShaderResourceView> GetOITAccSRV() const { return m_OITAccSRV; };
        ComPtr<ID3D11ShaderResourceView> GetOITWeightSRV() const { return m_OITWeightSRV; };

        const D3D11_VIEWPORT& GetViewport() const { return m_Viewport; }

    private:
        bool _CreateSwapChain(unsigned uWidth, unsigned uHeight, HWND hWnd);
		bool _CreateViewport(unsigned uWidth, unsigned uHeight);

		bool _CreateStencilView(unsigned uWidth, unsigned uHeight);
		bool _CreateSwapChainRTV();
        bool _CreateOITResource(unsigned uWidth, unsigned uHeight);

        ComPtr<ID3D11Device> m_piDevice{};
        ComPtr<ID3D11DeviceContext> m_piImmediateContext{};
        ComPtr<IDXGISwapChain> m_piSwapChain{};
		ComPtr<ID3D11DepthStencilView> m_piDepthStencilView{};

        ComPtr<ID3D11RenderTargetView> m_SwapChainRTV{};

        ComPtr<ID3D11RenderTargetView> m_MainRTV{};
        ComPtr<ID3D11RenderTargetView> m_OITAccRTV{};
        ComPtr<ID3D11RenderTargetView> m_OITWeightRTV{};

        ComPtr<ID3D11ShaderResourceView> m_MainSRV{};
        ComPtr<ID3D11ShaderResourceView> m_OITAccSRV{};
        ComPtr<ID3D11ShaderResourceView> m_OITWeightSRV{};

        D3D11_VIEWPORT m_Viewport{};
    };
}