#pragma once

#include <d3d11.h>
#include <memory>
#include <wrl/client.h>

namespace Zongine {
    class WindowManager;

    using Microsoft::WRL::ComPtr;

    struct DeviceManagerDesc {
        std::shared_ptr<WindowManager> windowManager;
    };

    class DeviceManager {
    public:
        void Initialize(const DeviceManagerDesc& desc);
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

        std::shared_ptr<WindowManager> m_WindowManager{};

        ComPtr<ID3D11Device> m_piDevice{};
        ComPtr<ID3D11DeviceContext> m_piImmediateContext{};
        ComPtr<IDXGISwapChain> m_piSwapChain{};
		ComPtr<ID3D11DepthStencilView> m_piDepthStencilView{};
        ComPtr<ID3D11RenderTargetView> m_piRenderTargetView{};

        D3D11_VIEWPORT m_Viewport{};
    };
}