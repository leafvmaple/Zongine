#pragma once

#include "Types.h"
#include "Enums.h"
#include "Const.h"

#include <d3d11.h>
#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <unordered_map>
#include <wrl/client.h>

#include "FX11/inc/d3dx11effect.h"

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    struct SubsetMeshAsset {
        UINT uStartIndex{};
        UINT uIndexCount{};
    };

    struct BONE {
        UINT nParentIndex{};
        std::string Name{};
        DirectX::XMFLOAT4X4 InversePoseTransform{};
        DirectX::XMFLOAT4X4 PhysicsPoseTransform{};
        std::vector<UINT> Children{};
    };

    struct SOCKET {
        std::string Name{};
        UINT nParentBoneIndex{};
        DirectX::XMFLOAT4X4 Offset{};
    };

    struct MeshAsset {
        std::string Path{};

        struct VERTEX_BUFFER {
            ComPtr<ID3D11Buffer> Buffer{};
            UINT uStride{};
            UINT uOffset{};
        } Vertex{};

        struct INDEX_BUFFER {
            ComPtr<ID3D11Buffer> Buffer{};
            DXGI_FORMAT eFormat{ DXGI_FORMAT_UNKNOWN };
            UINT uOffset{};
        } Index{};

        INPUT_LAYOUT InputLayout{};

        std::vector<SubsetMeshAsset> Subsets{};
        std::vector<BONE> Bones{};
        std::vector<SOCKET> Sockets{};

        std::unordered_map<std::string, UINT> BoneMap{};
    };

    struct SubsetShader {
        std::string ShaderPath{};

        ComPtr<ID3DX11Effect> Effect{};
        ID3DX11EffectVariable* SubsetConst{};
        std::unordered_map<std::string, ID3DX11EffectShaderResourceVariable*> Variables{};
    };

    struct ShaderAsset {
        std::vector<SubsetShader> Subsets{};

        RENDER_PASS Pass{};

        ID3DX11EffectMatrixVariable* TransformMatrix{};
        ID3DX11EffectMatrixVariable* BonesMatrix{};

        std::vector<ComPtr<ID3D11Buffer>> SubsetBuffers{};
    };

    struct TextureAsset {
        std::string Name{};
        ComPtr<ID3D11ShaderResourceView> Texture{};
    };

    struct ReferenceMaterialAsset {
        std::string Path{};
        std::string ShaderName{};

        uint32_t nBlendMode;
        RASTERIZER_STATE_ID Rasterizer{};

        SKIN_SUBSET_CONST Const{};

        std::unordered_map<std::string, TextureAsset> Textures{};
    };

    struct MaterialAsset {
        std::string Path{};
        std::vector<ReferenceMaterialAsset> Subsets;
    };

    struct SkeletonBone {
        std::string Name{};
        std::vector<int> Children{};
    };

    struct SkeletonAsset {
        std::string Path;
        std::vector<SkeletonBone> Bones;

        int nRootBoneIndex{};
    };

    struct AnimationSRT {
        DirectX::XMFLOAT3 Translation{ 0.f, 0.f, 0.f };
        DirectX::XMFLOAT3 Scale{ 1.f, 1.f, 1.f };
        DirectX::XMFLOAT4 Rotation{ 0.f, 0.f, 0.f, 1.f };
        DirectX::XMFLOAT4 SRotation{ 0.f, 0.f, 0.f, 1.f };
    };

    struct AnimationAsset {
        std::string Path{};
        float FrameRate{};     // Frame per second
        float FrameLength{};   // ms

        int AnimationLength{};   // ms

        std::vector<int> BoneFlag{};
        std::vector<std::vector<AnimationSRT>> Clip{};
    };

    struct TerrainAsset {
        DirectX::XMINT2 Origin;
    };

    struct LandscapeRegionAsset {
        TerrainAsset Terrain{};
        std::vector<ReferenceMaterialAsset> Materials{};
        std::vector<std::vector<float>> HeightData;
        ComPtr<ID3D11ShaderResourceView> HeightTexture{};
    };

    struct LandscapeAsset {
        std::string Dir{};
        std::string Name{};
        std::vector<std::vector<LandscapeRegionAsset>> Regions{};
    };
}