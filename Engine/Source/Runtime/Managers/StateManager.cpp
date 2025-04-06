#include "StateManager.h"

#include "DeviceManager.h"

#include "LAssert.h"

namespace Zongine {
    struct _RASTERIZER_STATE_INFO {
        RASTERIZER_STATE_ID eID;
        D3D11_RASTERIZER_DESC Desc;
    };

    struct _SAMPLER_STATE_INFO {
        SAMPLER_STATE_ID eID;
        D3D11_SAMPLER_DESC Desc;
    };

    struct _DEPTH_STENCIL_STATE_INFO {
        DEPTHSTENCIL_STATE_ID eID;
        D3D11_DEPTH_STENCIL_DESC Desc;
    };

    static const _RASTERIZER_STATE_INFO RASTERIZER_STATE_LIST[] = {
        {
            RASTERIZER_STATE_CULL_NONE,
            {
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
            }
        },
        {
            RASTERIZER_STATE_CULL_CLOCKWISE,
            {
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
            }
        },
    };

    static const _SAMPLER_STATE_INFO SAMPLER_STATE_LIST[] = {
        {
            SAMPLER_STATE_DEFAULT,
            {
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
            }
        },
        {
            SAMPLER_STATE_POINT_CLAMP,
            {
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
        },
    };

    static const _DEPTH_STENCIL_STATE_INFO DEPTH_STENCIL_STATE_LIST[] = {
        {
            ZWRITE_ENABLE,
            {
                TRUE,
                D3D11_DEPTH_WRITE_MASK_ALL,
                D3D11_COMPARISON_LESS_EQUAL,
                FALSE
            }
        },
    };

    bool StateManager::Initialize() {
        auto desc = DeviceManager::GetInstance().GetDevice();

        for (int i = 0; i < RASTERIZER_STATE_COUNT; i++)
            CHECK_HRESULT(desc->CreateRasterizerState(&RASTERIZER_STATE_LIST[i].Desc, m_Rasterizers[i].GetAddressOf()));

        for (int i = 0; i < SAMPLER_STATE_COUNT; i++)
            CHECK_HRESULT(desc->CreateSamplerState(&SAMPLER_STATE_LIST[i].Desc, m_Samplers[i].GetAddressOf()));

        for (int i = 0; i < DEPTH_WRITE_COUNT; i++)
            CHECK_HRESULT(desc->CreateDepthStencilState(&DEPTH_STENCIL_STATE_LIST[i].Desc, m_DepthStencils[i].GetAddressOf()));

        return true;
    }
}