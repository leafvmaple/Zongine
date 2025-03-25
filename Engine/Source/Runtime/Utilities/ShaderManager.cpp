#include "ShaderManager.h"

#include "DeviceManager.h"

#include "IShader.h"

namespace Zongine {
    static const char* VERTEX_SHADER_ID_NAME[] {
        "VERTEX_SHADER_CI_MESH",
        "VERTEX_SHADER_CI_SKINMESH",
        "VERTEX_SHADER_CI_TERRAIN",
    };

    static const char* PIXEL_SHADER_ID_NAME[] {
        "PIXEL_SHADER_NULL",
        "PIXEL_SHADER_UV1_NO_LIGHT",
    };

    static const struct _LAYOUT_INFO {
        INPUT_LAYOUT eInputLayout;
        VERTEX_SHADER eVertexShader;
        unsigned int uDescCount;
        D3D11_INPUT_ELEMENT_DESC DescArr[30];
    } INPUT_LAYOUT_LIST[] {
        {
            INPUT_LAYOUT_CI_MESH,
            VERTEX_SHADER_CI_MESH,
            5,
            {
                {"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR",       0, DXGI_FORMAT_B8G8R8A8_UNORM,      0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,        0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0}
            }
        },
        {
            INPUT_LAYOUT_CI_SKINMESH,
            VERTEX_SHADER_CI_SKINMESH,
            7,
            {
                {"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR",       0, DXGI_FORMAT_B8G8R8A8_UNORM,      0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,        0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT,       0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0}
            }
        },
        {
            INPUT_LAYOUT_CI_TERRAIN,
            VERTEX_SHADER_CI_TERRAIN,
            3,
            {
                {"POSITION",    0, DXGI_FORMAT_R32G32_FLOAT,        0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA,    0},
                {"CINSTANCE",   0, DXGI_FORMAT_R32G32B32A32_SINT,   1,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_INSTANCE_DATA,  1},
                {"CINSTANCE",   1, DXGI_FORMAT_R32G32B32A32_FLOAT,  1,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_INSTANCE_DATA,  1},
            }
        },
    };

    bool ShaderManager::Initialize(const ShaderInitInfo& info) {

        std::array<SHADER_SOURCE, VERTEX_SHADER_COUNT> VertexShaderInfos{};
        auto pDevice = info.pDevice->GetDevice();

        // Vertex Shader
        for (int i = 0; i < VERTEX_SHADER_COUNT; ++i)
        {
            SHADER_DESC desc{ VERTEX_SHADER_ID_NAME[i] };
            auto& ShaderInfo = VertexShaderInfos[i];

            CHECK_BOOL(LoadVSShader(&desc, &ShaderInfo));
            CHECK_HRESULT(pDevice->CreateVertexShader(ShaderInfo.pByte, ShaderInfo.nSize, nullptr, m_VertexShaders[i].GetAddressOf()));
        }

        // Pixel Shader
        for (int i = PIXEL_SHADER_NULL + 1; i < PIXEL_SHADER_COUNT; ++i)
        {
            SHADER_DESC desc{ PIXEL_SHADER_ID_NAME[i] };
            SHADER_SOURCE source{};

            CHECK_BOOL(LoadPSShader(&desc, &source));
            CHECK_HRESULT(pDevice->CreatePixelShader(source.pByte, source.nSize, nullptr, m_PixelShaders[i].GetAddressOf()));
        }

        // Input Layer
        for (int i = 0; i < INPUT_LAYOUT_COUNT; ++i)
        {
            const auto& LayoutInfo = INPUT_LAYOUT_LIST[i];
            const auto& ShaderInfo = VertexShaderInfos[LayoutInfo.eVertexShader];

            CHECK_HRESULT(pDevice->CreateInputLayout(LayoutInfo.DescArr, LayoutInfo.uDescCount, ShaderInfo.pByte, ShaderInfo.nSize, m_InputLayouts[i].GetAddressOf()));
        }

        return true;
    }
}