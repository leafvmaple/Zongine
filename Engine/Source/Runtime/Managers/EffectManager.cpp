#include "EffectManager.h"

#include "DeviceManager.h"

#include "Include/Enums.h"

#include "Utilities/StringUtils.h"

#include <d3d11.h>
#include "FX11/inc/d3dx11effect.h"

#include <d3dcompiler.h>
#include <fstream>
#include <sstream>

#define MATERIAL_SHADER_ROOT "data/material/Shader/"

namespace Zongine {
    static struct _SHADER_TEMPLATE {
        RUNTIME_MACRO Macro;
        const char* szPath;
        int nDescCount;
        D3D11_INPUT_ELEMENT_DESC Desc[30];
    } SShaderTemplate[] = {
        {
            RUNTIME_MACRO_MESH,
            MATERIAL_SHADER_ROOT"MeshShader.fx5",
            5,
            {
                {"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR",       0, DXGI_FORMAT_B8G8R8A8_UNORM,      0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,        0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
            }
        },
        {
            RUNTIME_MACRO_SKIN_MESH,
            MATERIAL_SHADER_ROOT"SkinMeshShader.fx5",
            7,
            {
                {"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR",       0, DXGI_FORMAT_B8G8R8A8_UNORM,      0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,        0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT,       0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0},
            }
        },
        {
            RUNTIME_MACRO_TERRAIN,
            MATERIAL_SHADER_ROOT"TerrainShader.fx5",
            3,
            {
                {"POSITION",    0, DXGI_FORMAT_R32G32_FLOAT,        0,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA,    0},
                {"CINSTANCE",   0, DXGI_FORMAT_R32G32B32A32_SINT,   1,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_INSTANCE_DATA,  1},
                {"CINSTANCE",   1, DXGI_FORMAT_R32G32B32A32_FLOAT,  1,  D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_INSTANCE_DATA,  1},
            }
        },
    };

    struct D3DInclude : public ID3DInclude
    {
    public:
        D3DInclude(const std::string& path) : m_Name(path) {}
        virtual ~D3DInclude() = default;

        virtual HRESULT STDMETHODCALLTYPE Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override {
            std::string fileName = !_stricmp(pFileName, "UserShader.fx5") ? m_Name : std::string(MATERIAL_SHADER_ROOT) + pFileName;

            std::ifstream file(fileName, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                return E_FAIL;
            }

            std::streamsize fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            m_pBuffer = std::make_unique<std::vector<char>>(fileSize);
            if (!file.read(m_pBuffer->data(), fileSize)) {
                return E_FAIL;
            }

            *ppData = m_pBuffer->data();
            *pBytes = static_cast<UINT>(fileSize);

            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE Close(LPCVOID pData) override {
            return S_OK;
        }

    protected:
        std::string m_Name;
        std::unique_ptr<std::vector<char>> m_pBuffer;
    };

    void EffectManager::Initialize(const EffectManagerInfo& info) {
        m_DeviceManager = info.deviceManager;
    }

    ComPtr<ID3DX11Effect> EffectManager::LoadEffect(RUNTIME_MACRO macro, const std::string& path) {
        DWORD nShaderFlags = 0;
        ComPtr<ID3D10Blob> compiledShader;
        ComPtr<ID3D10Blob> compiledMsgs;

#if defined( DEBUG ) || defined( _DEBUG )
        nShaderFlags |= D3D10_SHADER_DEBUG;
        nShaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
        auto device = m_DeviceManager->GetDevice();

        auto& effect = m_Effects[macro][path];
        if (!effect) {
            std::filesystem::path filePath = SShaderTemplate[macro].szPath;

            char error[MAX_PATH];

            D3DInclude include(path);

            auto hr = D3DCompileFromFile(filePath.wstring().c_str(), 0, &include, 0, "fx_5_0", nShaderFlags, 0, compiledShader.GetAddressOf(), compiledMsgs.GetAddressOf());
            if (FAILED(hr)) {
                strcpy(error, (const char*)compiledMsgs->GetBufferPointer());
            }
            D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), 0, device.Get(), effect.GetAddressOf());
        }

        auto& inputLayout = m_InputLayouts[macro];
        if (!inputLayout) {
            D3DX11_PASS_DESC passDesc;
            auto& desc = SShaderTemplate[macro];

            auto pass = effect->GetTechniqueByIndex(0)->GetPassByIndex(0);
            pass->GetDesc(&passDesc);

            device->CreateInputLayout(
                desc.Desc, desc.nDescCount,
                passDesc.pIAInputSignature,
                passDesc.IAInputSignatureSize,
                inputLayout.GetAddressOf()
            );
        }

        return effect;
    }

    void EffectManager::LoadVariables(ComPtr<ID3DX11Effect> effect, std::unordered_map<std::string, ID3DX11EffectShaderResourceVariable*>& Variables) {
        D3DX11_EFFECT_DESC desc;
        D3DX11_EFFECT_VARIABLE_DESC variableDesc;
        D3DX11_EFFECT_TYPE_DESC typeDesc;

        effect->GetDesc(&desc);

        for (int i = 0; i < desc.GlobalVariables; i++)
        {
            ID3DX11EffectVariable* pVariable = effect->GetVariableByIndex(i);
            pVariable->GetDesc(&variableDesc);

            ID3DX11EffectType* pType = pVariable->GetType();
            pType->GetDesc(&typeDesc);

            if (typeDesc.Type == D3D_SVT_TEXTURE2D)
            {
                ID3DX11EffectShaderResourceVariable* pSRVariable = pVariable->AsShaderResource();
                Variables.try_emplace(variableDesc.Name, pSRVariable);
            }
        }
    }

    ID3DX11EffectPass* EffectManager::GetEffectPass(ComPtr<ID3DX11Effect> effect, RENDER_PASS pass) {
        const RENDER_PASS_TABLE& passTable = m_RenderPassTable[pass];

        auto technique = effect->GetTechniqueByName(passTable.szTechniqueName);
        return technique->GetPassByIndex(passTable.uPassSlot);
    }
}