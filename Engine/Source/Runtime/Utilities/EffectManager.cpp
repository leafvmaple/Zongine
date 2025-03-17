#include "EffectManager.h"

#include "DeviceManager.h"

#include "StringUtils.h"

#include "FX11/inc/d3dx11effect.h"

#include <d3dcompiler.h>
#include <fstream>
#include <sstream>

#define MATERIAL_SHADER_ROOT "data/material/Shader/"

namespace Zongine {
    static const char* SShaderTemplate[] = {
        MATERIAL_SHADER_ROOT"MeshShader.fx5",
        MATERIAL_SHADER_ROOT"SkinMeshShader.fx5",
        MATERIAL_SHADER_ROOT"TerrainShader.fx5",
    };

    struct D3DInclude : public ID3DInclude
    {
    public:
        D3DInclude(const char* szFileName) : m_sName(szFileName) {}
        virtual ~D3DInclude() = default;

        virtual HRESULT STDMETHODCALLTYPE Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override {
            std::string fileName = !_stricmp(pFileName, "UserShader.fx5") ? m_sName : std::string(MATERIAL_SHADER_ROOT) + pFileName;

            std::ifstream file(fileName, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                return E_FAIL;
            }

            std::streamsize fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            auto buffer = std::make_unique<std::vector<char>>(fileSize);
            if (!file.read(buffer->data(), fileSize)) {
                return E_FAIL;
            }

            *ppData = m_pBuffer->data();
            *pBytes = static_cast<UINT>(fileSize);

            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE Close(LPCVOID pData) override {
            delete pData;

            return S_OK;
        }

    protected:
        std::string m_sName;
        std::unique_ptr<std::vector<char>> m_pBuffer;
    };

    void EffectManager::Initialize(const EffectManagerInfo& info) {
        m_DeviceManager = info.deviceManager;
    }

    ComPtr<ID3DX11Effect> EffectManager::LoadEffect(RUNTIME_MACRO macro, const std::string& filePath) {
        DWORD nShaderFlags = 0;
        ComPtr<ID3D10Blob> compiledShader;
        ComPtr<ID3D10Blob> compiledMsgs;

#if defined( DEBUG ) || defined( _DEBUG )
        nShaderFlags |= D3D10_SHADER_DEBUG;
        nShaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

        auto effect = m_Effects[macro][filePath];
        if (!effect)
        {
            auto device = m_DeviceManager->GetDevice();

            D3DInclude include(filePath.c_str());
            auto shaderPath = AnsiToWString(SShaderTemplate[macro]);

            D3DCompileFromFile(shaderPath.c_str(), 0, &include, 0, "fx_5_0", nShaderFlags, 0, compiledShader.GetAddressOf(), compiledMsgs.GetAddressOf());
            D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), 0, device.Get(), effect.GetAddressOf());
        }

        return effect;
    }

    ID3DX11EffectPass* EffectManager::GetEffectPass(ComPtr<ID3DX11Effect> effect, RENDER_PASS pass) {
        const RENDER_PASS_TABLE& passTable = m_RenderPassTable[pass];

        ID3DX11EffectTechnique* technique = effect->GetTechniqueByName(passTable.szTechniqueName);
        return technique->GetPassByIndex(passTable.uPassSlot);
    }
}