#pragma once

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>

#include "../Include/const.h"

#include "NVFlex/include/NvFlexExt.h"

namespace Zongine {
    constexpr int FLEX_NORMALIZE_SCLAE = 100;
    class Entity;

    struct NvFlexContent {
        NvFlexVector<int> Active;
        NvFlexVector<int> Phases;
        NvFlexVector<int> Triangles;

        NvFlexVector<DirectX::XMFLOAT4> Particles;
        NvFlexVector<DirectX::XMFLOAT4> Normals;
        NvFlexVector<DirectX::XMFLOAT4> ResetParticles;
        NvFlexVector<DirectX::XMFLOAT3> Velocities;
        NvFlexVector<DirectX::XMFLOAT3> TriangleNormals;

        NvFlexVector<int> springIndices;
        NvFlexVector<float> springLengths;
        NvFlexVector<float> springStiffness;

        NvFlexContent(NvFlexLibrary* l, int size) :
            Active(l, size),
            Particles(l, size),
            Normals(l),
            ResetParticles(l, size),
            Triangles(l),
            TriangleNormals(l),
            Phases(l, size),
            Velocities(l, size),
            springIndices(l),
            springLengths(l),
            springStiffness(l) {}

        void map() {
            Active.map();
            Particles.map();
            Normals.map();
            ResetParticles.map();
            Phases.map();
            Velocities.map();
            Triangles.map();
            TriangleNormals.map();
            springIndices.map();
            springLengths.map();
            springStiffness.map();
        }

        void unmap() {
            Active.unmap();
            Particles.unmap();
            Normals.unmap();
            ResetParticles.unmap();
            Phases.unmap();
            Velocities.unmap();
            Triangles.unmap();
            TriangleNormals.unmap();
            springIndices.unmap();
            springLengths.unmap();
            springStiffness.unmap();
        }
    };

    struct NvFlexComponent {
        bool bInitialized{ false };
        std::string Path{};
        std::string MeshPath{};

        std::shared_ptr<NvFlexContent> Content{};

        std::vector<DirectX::XMFLOAT4> Particles{};
        std::vector<DirectX::XMFLOAT4> FlexPosition{};
        std::vector<FLEX_VERTEX_EXT> FlexVertices{};

        int ParticlesCount{};
        int TrianglesCount{};

        void Initialize(const Entity& entity, NvFlexLibrary* library);
    };
}