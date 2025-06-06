#pragma once

#include "Types.h"
#include "Enums.h"
#include "Const.h"

#include <d3d11.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <unordered_map>
#include <wrl/client.h>

#include "FX11/inc/d3dx11effect.h"

namespace Zongine {
    using Microsoft::WRL::ComPtr;
    using DirectX::PackedVector::XMCOLOR;

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

    struct VERTEX {
        DirectX::XMFLOAT3 Position{ 0.f, 0.f, 0.f };
        DirectX::XMFLOAT3 Normal{ 0.f, 0.f, 0.f };
        DirectX::XMFLOAT4 Tangent{ 0.0f, 1.0f, 0.0f, 1.0f };
        XMCOLOR Color{ 0xFF, 0xFF, 0xFF, 0xFF };
        DirectX::XMFLOAT2 TexCoords{ 0.f, 0.f };
        float BoneWeights[4]{ 0.f };
        BYTE BoneIndices[4]{ 0xFF, 0xFF, 0xFF, 0xFF };
        XMCOLOR Emissive{ 0xFF, 0xFF, 0xFF, 0xFF };
    };

    struct VERTEX_BUFFER {
        ComPtr<ID3D11Buffer> Buffer{};
        UINT uStride{};
        UINT uOffset{};
    };

    struct INDEX_BUFFER {
        ComPtr<ID3D11Buffer> Buffer{};
        DXGI_FORMAT eFormat{ DXGI_FORMAT_UNKNOWN };
        UINT uOffset{};
    };

    struct MeshAsset {
        std::string Path{};

        VERTEX_BUFFER Vertex{};
        INDEX_BUFFER Index{};

        RUNTIME_MACRO Macro{};

        std::vector<SubsetMeshAsset> Subsets{};
        std::vector<BONE> Bones{};
        std::vector<SOCKET> Sockets{};
        std::vector<VERTEX> Vertices{};

        std::vector<std::vector<int>> NeighborVertices;

        std::unordered_map<std::string, UINT> BoneMap{};
    };

    struct SubsetShaderAsset {
        std::string ShaderPath{};

        ComPtr<ID3DX11Effect> Effect{};
        ID3DX11EffectConstantBuffer* ModelConst{};
        ID3DX11EffectConstantBuffer* SubsetConst{};
        ID3DX11EffectConstantBuffer* CameraConst{};
        std::unordered_map<std::string, ID3DX11EffectShaderResourceVariable*> Variables{};
    };

    struct ShaderAsset {
        std::vector<SubsetShaderAsset> Subsets{};
        std::vector<ComPtr<ID3D11Buffer>> SubsetBuffers{};
    };

    struct TextureAsset {
        std::string Name{};
        ComPtr<ID3D11ShaderResourceView> Texture{};
    };

    struct ReferenceMaterialAsset {
        std::string Path{};
        std::string ShaderName{};

        BLEND_STATE Blend{};
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

    struct NvFlexAsset {
        std::string Path;

        std::vector<int> ParticleVertexMap{};
        std::vector<int> VertexParticleMap;
        std::vector<bool> IsBorder;
        std::vector<bool> IsGap;

        std::vector<DirectX::XMFLOAT4> Particles{};
        std::vector<int> ParticleIndies{};
        std::vector<int> ActiveParticleIndies{};

        std::vector<std::unordered_set<int>> ActiveNeighborParticles;

        ComPtr<ID3D11Buffer> Buffers{};
        UINT uStride{};
        UINT uOffset{};
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
        DirectX::XMUINT2 Origin;
    };

    struct LandscapeRegionAsset {
        TerrainAsset Terrain{};
        // TODO
        // std::vector<ReferenceMaterialAsset> Materials{};
        ReferenceMaterialAsset Material{};
        std::vector<std::vector<float>> HeightData;
        ComPtr<ID3D11ShaderResourceView> HeightTexture{};
        ID3DX11EffectVariable* TerrainParam{};
    };

    struct LandscapeAsset {
        std::string Dir{};
        std::string Name{};
        std::vector<std::vector<LandscapeRegionAsset>> Regions{};
    };
}