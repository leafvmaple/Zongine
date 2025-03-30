#include "DeviceManager.h"

#include "WindowManager.h"

#include "LAssert.h"

namespace Zongine {

    static const D3D_FEATURE_LEVEL FEATURE_LEVEL_ARRAY_0[] =
    {
        D3D_FEATURE_LEVEL_11_0,
    };

    static const D3D_FEATURE_LEVEL FEATURE_LEVEL_ARRAY_1[] =
    {
        D3D_FEATURE_LEVEL_11_1,
    };

    void DeviceManager::Initialize(const DeviceManagerDesc& desc) {
        UINT uCreateDeviceFlag{};

        m_WindowManager = desc.windowManager;

        auto width = m_WindowManager->GetWidth();
        auto height = m_WindowManager->GetHeight();

#if defined(DEBUG) || defined(_DEBUG)  
        uCreateDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, uCreateDeviceFlag,
            FEATURE_LEVEL_ARRAY_0, _countof(FEATURE_LEVEL_ARRAY_0),
            D3D11_SDK_VERSION,
            m_piDevice.GetAddressOf(), nullptr, m_piImmediateContext.GetAddressOf()
        );

        _CreateSwapChain(width, height, m_WindowManager->GetWindowHandle());
        _CreateViewport(width, height);
    }

    void DeviceManager::Resize() {
        if (m_piSwapChain) {
            ComPtr<ID3D11Texture2D> buffer{};

            auto width = m_WindowManager->GetWidth();
            auto height = m_WindowManager->GetHeight();

            m_piSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

            m_piSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buffer);
            m_piDevice->CreateRenderTargetView(buffer.Get(), nullptr, m_piRenderTargetView.GetAddressOf());

            _CreateStencilView(width, height);
            _CreateSwapChainRTV();

            _CreateViewport(width, height);
        }
    }

    bool DeviceManager::_CreateSwapChain(unsigned uWidth, unsigned uHeight, HWND hWnd) {
        DXGI_SWAP_CHAIN_DESC desc{};
        ComPtr<IDXGIDevice> device{};
        ComPtr<IDXGIAdapter> adapter{};
        ComPtr<IDXGIFactory> factory{};

        desc.BufferCount = 1;
        desc.BufferDesc.Width = uWidth;
        desc.BufferDesc.Height = uHeight;
        desc.BufferDesc.RefreshRate.Numerator = 60;
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        // DXGI_SAMPLE_DESC
        desc.SampleDesc.Count = 1;     // 1X MASS (MultiSample Anti-Aliasing) 

        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow = hWnd;
        desc.Windowed = true; // from SDK: should not create a full-screen swap chain
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        m_piDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)device.GetAddressOf());
        device->GetParent(__uuidof(IDXGIAdapter), (void**)adapter.GetAddressOf());
        adapter->GetParent(__uuidof(IDXGIFactory), (void**)factory.GetAddressOf());
        factory->CreateSwapChain(m_piDevice.Get(), &desc, m_piSwapChain.GetAddressOf());

        _CreateStencilView(uWidth, uHeight);
        _CreateSwapChainRTV();

        return true;
    }

    bool DeviceManager::_CreateViewport(unsigned uWidth, unsigned uHeight) {
        m_Viewport.Width = (FLOAT)uWidth;
        m_Viewport.Height = (FLOAT)uHeight;
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;

        m_piImmediateContext->RSSetViewports(1, &m_Viewport);

        return true;
    }

    bool DeviceManager::_CreateStencilView(unsigned uWidth, unsigned uHeight) {
        ComPtr<ID3D11Texture2D> buffer{};
        D3D11_TEXTURE2D_DESC desc{};

        desc.Width = uWidth;
        desc.Height = uHeight;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

        desc.SampleDesc.Count = 1;

        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        CHECK_HRESULT(m_piDevice->CreateTexture2D(&desc, 0, buffer.GetAddressOf()));
        CHECK_HRESULT(m_piDevice->CreateDepthStencilView(buffer.Get(), 0, m_piDepthStencilView.GetAddressOf()));

        return true;
    }

    bool DeviceManager::_CreateSwapChainRTV()
    {
        ComPtr<ID3D11Texture2D> buffer{};

        CHECK_HRESULT(m_piSwapChain->GetBuffer(0, __uuidof(buffer), reinterpret_cast<void**>(buffer.GetAddressOf())));
        CHECK_HRESULT(m_piDevice->CreateRenderTargetView(buffer.Get(), nullptr, m_piRenderTargetView.GetAddressOf()));

        m_piImmediateContext->OMSetRenderTargets(1, m_piRenderTargetView.GetAddressOf(), m_piDepthStencilView.Get());

        return true;
    }
}
