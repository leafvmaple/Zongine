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
        ComPtr<ID3D11RenderTargetView> GetRenderTargetView() const { return m_piRenderTargetView; }

    private:
        bool _CreateSwapChain(unsigned uWidth, unsigned uHeight, HWND hWnd);
		bool _CreateViewport(unsigned uWidth, unsigned uHeight);

		bool _CreateStencilView(unsigned uWidth, unsigned uHeight);
		bool _CreateSwapChainRTV();

        ComPtr<ID3D11Device> m_piDevice{};
        ComPtr<ID3D11DeviceContext> m_piImmediateContext{};
        ComPtr<IDXGISwapChain> m_piSwapChain{};
		ComPtr<ID3D11DepthStencilView> m_piDepthStencilView{};
        ComPtr<ID3D11RenderTargetView> m_piRenderTargetView{};

        D3D11_VIEWPORT m_Viewport{};
    };
}