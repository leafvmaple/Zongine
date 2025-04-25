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
        NvFlexVector<int> Phases;
        NvFlexVector<int> Triangles;

        NvFlexVector<DirectX::XMFLOAT4> Particles;
        NvFlexVector<DirectX::XMFLOAT3> Velocities;
        NvFlexVector<DirectX::XMFLOAT3> triangleNormals;

        NvFlexVector<int> springIndices;
        NvFlexVector<float> springLengths;
        NvFlexVector<float> springStiffness;

        NvFlexContent(NvFlexLibrary* l, int size) :
            Particles(l, size), Triangles(l), triangleNormals(l),
            Phases(l, size), Velocities(l, size),
            springIndices(l), springLengths(l), springStiffness(l) {}

        void map() {
            Particles.map();
            Phases.map();
            Velocities.map();
            Triangles.map();
            triangleNormals.map();
            springIndices.map();
            springLengths.map();
            springStiffness.map();
        }

        void unmap() {
            Particles.unmap();
            Phases.unmap();
            Velocities.unmap();
            Triangles.unmap();
            triangleNormals.unmap();
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

        std::vector<FLEX_VERTEX_EXT> FlexVertices{};

        int ParticlesCount{};
        int TrianglesCount{};

        void Initialize(const Entity& entity, NvFlexLibrary* library);
    };
}