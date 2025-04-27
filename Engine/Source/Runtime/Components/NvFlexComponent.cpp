#include "NVFlexComponent.h"

#include "../Entities/EntityManager.h"
#include "../Managers/AssetManager.h"
#include "../Managers/DeviceManager.h"

#include "../Components/TransformComponent.h"

#include "NVFlex/include/NvFlexExt.h"
#include "NVFlex/core/cloth.h"

#include <unordered_set>
#include <queue>

namespace Zongine {
    using DirectX::XMVector3Transform;
    using DirectX::XMVECTOR;
    using DirectX::XMVectorSetW;
    using DirectX::XMLoadFloat4x4;
    using DirectX::XMVectorScale;
    using DirectX::XMVectorAdd;
    using DirectX::XMVectorSubtract;
    using DirectX::XMVector3Length;
    using DirectX::XMStoreFloat;
    using DirectX::XMMATRIX;
    using DirectX::XMFLOAT4X4;
    using DirectX::XMLoadFloat3;

    void NvFlexComponent::Initialize(const Entity& entity, NvFlexLibrary* library) {
        std::vector<std::unordered_set<int>> neighborVertices;

        auto& transformComponent = entity.GetComponent<TransformComponent>();
        auto& meshComponent = entity.GetComponent<MeshComponent>();

        auto flex = AssetManager::GetInstance().GetNvFlexAsset(Path);
        auto mesh = AssetManager::GetInstance().GetMeshAsset(meshComponent.Path);

        auto verticesCount = mesh->Vertices.size();

        ParticlesCount = flex->Particles.size();

        std::queue<int> queue;
        std::vector<int> visited(ParticlesCount, 0);
        std::vector<int> nearestAnchors(ParticlesCount, -1);

        Particles = flex->Particles;
        FlexPosition.resize(verticesCount);
        for (int i = 0; i < verticesCount; i++) {
            const auto& vertex = mesh->Vertices[i];
            auto particleID = flex->VertexParticleMap[i];

            DirectX::XMVECTOR position = DirectX::XMVectorZero();
            DirectX::XMVECTOR vertexPos = XMLoadFloat3(&vertex.Position);

            for (int j = 0; j < 4; j++) {
                auto boneIndex = vertex.BoneIndices[j];
                if (boneIndex == 0xFF)
                    continue;
                auto weight = vertex.BoneWeights[j];
                auto transform = XMVectorScale(XMVector3Transform(vertexPos, XMLoadFloat4x4(&meshComponent.SkinningTransforms[boneIndex])), weight / FLEX_NORMALIZE_SCLAE);
                position = XMVectorAdd(position, transform);
            }

            position = XMVector3Transform(position, XMLoadFloat4x4(&transformComponent.World));
            XMStoreFloat4(&FlexPosition[i], XMVectorSetW(position, flex->Particles[particleID].w));

            FlexVertices.emplace_back(FLEX_VERTEX_EXT{ FlexPosition[i], flex->Particles[particleID].w });
        }

        ClothMesh cloth((Vec4*)flex->Particles.data(), ParticlesCount, flex->ActiveParticleIndies.data(), flex->ActiveParticleIndies.size(), 1.f, 1.f, true);

        nearestAnchors.resize(ParticlesCount, -1);
        for (int particleID = 0; particleID < ParticlesCount; particleID++) {
            if (flex->Particles[particleID].w == 0.0f) {
                queue.push(particleID);
                nearestAnchors[particleID] = particleID;
                visited[particleID] = 1;
            }
        }

        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();

            for (auto& neighbor : flex->ActiveNeighborParticles[current]) {
                if (visited[neighbor] == 0) {
                    visited[neighbor] = 1;
                    nearestAnchors[neighbor] = nearestAnchors[current];
                    queue.push(neighbor);
                }
            }
        }

        for (int particleID = 0; particleID < ParticlesCount; particleID++) {
            if (nearestAnchors[particleID] > 0 && nearestAnchors[particleID] != particleID) {
                float distance{};
                cloth.mConstraintIndices.push_back(particleID);
                cloth.mConstraintIndices.push_back(nearestAnchors[particleID]);

                XMStoreFloat(&distance, XMVector3Length(
                    XMVectorSubtract(XMLoadFloat4(&flex->Particles[particleID]), XMLoadFloat4(&flex->Particles[nearestAnchors[particleID]]))));

                cloth.mConstraintRestLengths.push_back(distance * 1.1);
                cloth.mConstraintCoefficients.push_back(-0.9f);
            }
        }

        Content = std::make_unique<NvFlexContent>(library, ParticlesCount);

        Content->map();

        Content->Normals.resize(ParticlesCount, DirectX::XMFLOAT4(0, 0, 1, 0));
        for (int i = 0; i < ParticlesCount; i++) {
            auto particle = FlexPosition[flex->ParticleVertexMap[i]];
            Content->Particles[i] = particle;
            Content->ResetParticles[i] = particle;
        }

        for (int i = 0; i < ParticlesCount; i++) {
            Content->Phases[i] = NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseSelfCollideFilter);
            Content->Velocities[i] = { 0.f, 0.f, 0.f };
            Content->Active[i] = i;
        }

        for (auto particleID : flex->ActiveParticleIndies)
            Content->Triangles.push_back(particleID);

        for (int i = 0; i < flex->ActiveParticleIndies.size() / 3; i++)
            Content->TriangleNormals.push_back({ 0.0f, 0.0f, 1.0f });

        auto springCount = cloth.mConstraintIndices.size() / 2;
        for (int i = 0; i < springCount; i++) {
            auto particleID = cloth.mConstraintIndices[i * 2];
            auto particleID1 = cloth.mConstraintIndices[i * 2 + 1];

            if (flex->Particles[particleID].w > 0.f || flex->Particles[particleID1].w > 0.f) {
                Content->springIndices.push_back(cloth.mConstraintIndices[i * 2]);
                Content->springIndices.push_back(cloth.mConstraintIndices[i * 2 + 1]);
                Content->springLengths.push_back(cloth.mConstraintRestLengths[i]);
                Content->springStiffness.push_back(cloth.mConstraintCoefficients[i]);
            }
        }

        Content->unmap();

        bInitialized = true;
    }
}