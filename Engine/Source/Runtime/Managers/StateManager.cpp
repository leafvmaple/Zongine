#include "StateManager.h"

#include "DeviceManager.h"

#include "LAssert.h"

namespace Zongine {
    static const D3D11_RASTERIZER_DESC RASTERIZER_STATE_LIST[] = {
        { // RASTERIZER_STATE_CULL_NONE,
            D3D11_FILL_SOLID,   //D3D11_FILL_MODE FillMode;
            D3D11_CULL_NONE,    //D3D11_CULL_MODE CullMode;
            FALSE,              //BOOL FrontCounterClockwise;
            0,      //INT DepthBias;
            0.0f,   //FLOAT DepthBiasClamp;
            0.0f,   //FLOAT SlopeScaledDepthBias;
            TRUE,   //BOOL DepthClipEnable;
            FALSE,  //BOOL ScissorEnable;
            FALSE,  //BOOL MultisampleEnable;
            FALSE,  //BOOL AntialiasedLineEnable;
        },
        { // RASTERIZER_STATE_CULL_CLOCKWISE
            D3D11_FILL_SOLID,   //D3D11_FILL_MODE FillMode;
            D3D11_CULL_BACK,    //D3D11_CULL_MODE CullMode;
            TRUE,               //BOOL FrontCounterClockwise;
            0,      //INT DepthBias;
            0.0f,   //FLOAT DepthBiasClamp;
            0.0f,   //FLOAT SlopeScaledDepthBias;
            TRUE,   //BOOL DepthClipEnable;
            FALSE,  //BOOL ScissorEnable;
            FALSE,  //BOOL MultisampleEnable;
            FALSE,  //BOOL AntialiasedLineEnable;
        },
    };

    static const D3D11_BLEND_DESC BLEND_STATE_LIST[] = {
        {
            false,  // BOOL AlphaToCoverageEnable
            false,  // BOOL IndependentBlendEnable;
            {
                {
                    false, // BOOL BlendEnable;
                    D3D11_BLEND_SRC_ALPHA, // D3D11_BLEND SrcBlend
                    D3D11_BLEND_INV_SRC_ALPHA, // D3D11_BLEND DestBlend
                    D3D11_BLEND_OP_ADD, // D3D11_BLEND_OP BlendOp
                    D3D11_BLEND_SRC_ALPHA, // D3D11_BLEND SrcBlendAlpha
                    D3D11_BLEND_INV_SRC_ALPHA, // D3D11_BLEND DestBlendAlpha
                    D3D11_BLEND_OP_ADD, // D3D11_BLEND_OP BlendOpAlpha
                    D3D11_COLOR_WRITE_ENABLE_ALL // UINT8 RenderTargetWriteMask;
                }
            }
        },
        {
            false,  // BOOL AlphaToCoverageEnable
            false,  // BOOL IndependentBlendEnable;
            {
                {
                    false, // BOOL BlendEnable;
                    D3D11_BLEND_SRC_ALPHA, // D3D11_BLEND SrcBlend
                    D3D11_BLEND_INV_SRC_ALPHA, // D3D11_BLEND DestBlend
                    D3D11_BLEND_OP_ADD, // D3D11_BLEND_OP BlendOp
                    D3D11_BLEND_SRC_ALPHA, // D3D11_BLEND SrcBlendAlpha
                    D3D11_BLEND_INV_SRC_ALPHA, // D3D11_BLEND DestBlendAlpha
                    D3D11_BLEND_OP_ADD, // D3D11_BLEND_OP BlendOpAlpha
                    D3D11_COLOR_WRITE_ENABLE_ALL // UINT8 RenderTargetWriteMask;
                }
            }
        },
        {
            false,  // BOOL AlphaToCoverageEnable
            false,  // BOOL IndependentBlendEnable;
            {
                {
                    true, // BOOL BlendEnable;
                    D3D11_BLEND_SRC_ALPHA, // D3D11_BLEND SrcBlend
                    D3D11_BLEND_INV_SRC_ALPHA, // D3D11_BLEND DestBlend
                    D3D11_BLEND_OP_ADD, // D3D11_BLEND_OP BlendOp
                    D3D11_BLEND_SRC_ALPHA, // D3D11_BLEND SrcBlendAlpha
                    D3D11_BLEND_INV_SRC_ALPHA, // D3D11_BLEND DestBlendAlpha
                    D3D11_BLEND_OP_MAX, // D3D11_BLEND_OP BlendOpAlpha
                    D3D11_COLOR_WRITE_ENABLE_ALL // UINT8 RenderTargetWriteMask;
                }
            }
        },
        {
            false,  // BOOL AlphaToCoverageEnable
            true,  // BOOL IndependentBlendEnable;
            {
                {
                    true, // BOOL BlendEnable;
                    D3D11_BLEND_ONE, // D3D11_BLEND SrcBlend
                    D3D11_BLEND_ONE, // D3D11_BLEND DestBlend
                    D3D11_BLEND_OP_ADD, // D3D11_BLEND_OP BlendOp
                    D3D11_BLEND_ONE, // D3D11_BLEND SrcBlendAlpha
                    D3D11_BLEND_ONE, // D3D11_BLEND DestBlendAlpha
                    D3D11_BLEND_OP_ADD, // D3D11_BLEND_OP BlendOpAlpha
                    D3D11_COLOR_WRITE_ENABLE_ALL // UINT8 RenderTargetWriteMask;
                },
                {
                    true, // BOOL BlendEnable;
                    D3D11_BLEND_ZERO, // D3D11_BLEND SrcBlend
                    D3D11_BLEND_INV_SRC_COLOR, // D3D11_BLEND DestBlend
                    D3D11_BLEND_OP_ADD, // D3D11_BLEND_OP BlendOp
                    D3D11_BLEND_ONE, // D3D11_BLEND SrcBlendAlpha
                    D3D11_BLEND_ONE, // D3D11_BLEND DestBlendAlpha
                    D3D11_BLEND_OP_ADD, // D3D11_BLEND_OP BlendOpAlpha
                    D3D11_COLOR_WRITE_ENABLE_ALL // UINT8 RenderTargetWriteMask;
                }
            }
        },
    };

    static const D3D11_SAMPLER_DESC SAMPLER_STATE_LIST[] = {
        { // SAMPLER_STATE_DEFAULT
            D3D11_FILTER_MIN_MAG_MIP_LINEAR,   //D3D11_FILTER Filter;
            D3D11_TEXTURE_ADDRESS_WRAP, //D3D11_TEXTURE_ADDRESS_MODE AddressU;
            D3D11_TEXTURE_ADDRESS_WRAP, //D3D11_TEXTURE_ADDRESS_MODE AddressV;
            D3D11_TEXTURE_ADDRESS_WRAP, //D3D11_TEXTURE_ADDRESS_MODE AddressW;
            0,                          //FLOAT MipLODBias;
            8,                          //UINT MaxAnisotropy;
            D3D11_COMPARISON_NEVER,     //D3D11_COMPARISON_FUNC ComparisonFunc;
            {0, 0, 0, 0},               //FLOAT BorderColor[ 4 ];
            0.0f,                       //FLOAT MinLOD;
            D3D11_FLOAT32_MAX           //FLOAT MaxLOD;
        },
        { // SAMPLER_STATE_POINT_CLAMP,
            D3D11_FILTER_MIN_MAG_MIP_POINT,  //D3D11_FILTER Filter;
            D3D11_TEXTURE_ADDRESS_CLAMP, //D3D11_TEXTURE_ADDRESS_MODE AddressU;
            D3D11_TEXTURE_ADDRESS_CLAMP, //D3D11_TEXTURE_ADDRESS_MODE AddressV;
            D3D11_TEXTURE_ADDRESS_CLAMP, //D3D11_TEXTURE_ADDRESS_MODE AddressW;
            0,                          //FLOAT MipLODBias;
            8,                          //UINT MaxAnisotropy;
            D3D11_COMPARISON_NEVER,     //D3D11_COMPARISON_FUNC ComparisonFunc;
            {0, 0, 0, 0},               //FLOAT BorderColor[ 4 ];
            0.0f,                       //FLOAT MinLOD;
            D3D11_FLOAT32_MAX           //FLOAT MaxLOD;
        }
    };

    static const D3D11_DEPTH_STENCIL_DESC DEPTH_STENCIL_STATE_LIST[] = {
        { // ZWRITE_ENABLE
            true,   // Enable Depth
            D3D11_DEPTH_WRITE_MASK_ALL,
            D3D11_COMPARISON_LESS_EQUAL,
            false   // Disable Stencil
        },
        { // ZWRITE_DISABLE
            false,  // Disable Depth
            D3D11_DEPTH_WRITE_MASK_ZERO,
            D3D11_COMPARISON_LESS_EQUAL,
            false   // Disable Stencil
        },
    };

    bool StateManager::Initialize() {
        auto desc = DeviceManager::GetInstance().GetDevice();

        assert(_countof(RASTERIZER_STATE_LIST) == RASTERIZER_STATE_COUNT);
        for (int i = 0; i < RASTERIZER_STATE_COUNT; i++)
            CHECK_HRESULT(desc->CreateRasterizerState(&RASTERIZER_STATE_LIST[i], m_Rasterizers[i].GetAddressOf()));

        assert(_countof(BLEND_STATE_LIST) == BLEND_STATE_COUNT);
        for (int i = 0; i < BLEND_STATE_COUNT; i++)
            CHECK_HRESULT(desc->CreateBlendState(&BLEND_STATE_LIST[i], m_Blends[i].GetAddressOf()));

        assert(_countof(SAMPLER_STATE_LIST) == SAMPLER_STATE_COUNT);
        for (int i = 0; i < SAMPLER_STATE_COUNT; i++)
            CHECK_HRESULT(desc->CreateSamplerState(&SAMPLER_STATE_LIST[i], m_Samplers[i].GetAddressOf()));

        assert(_countof(DEPTH_STENCIL_STATE_LIST) == DEPTH_STENCIL_STATE_COUNT);
        for (int i = 0; i < DEPTH_STENCIL_STATE_COUNT; i++)
            CHECK_HRESULT(desc->CreateDepthStencilState(&DEPTH_STENCIL_STATE_LIST[i], m_DepthStencils[i].GetAddressOf()));

        return true;
    }
}