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
    using DirectX::XMVector4Length;
    using DirectX::XMStoreFloat;

    void NvFlexComponent::Initialize(const Entity& entity, NvFlexLibrary* library) {
        std::vector<std::unordered_set<int>> neighborVertices;

        auto& transformComponent = entity.GetComponent<TransformComponent>();
        auto& meshComponent = entity.GetComponent<MeshComponent>();

        auto flex = AssetManager::GetInstance().GetNvFlexAsset(Path);
        auto mesh = AssetManager::GetInstance().GetMeshAsset(meshComponent.Path);

        auto verticesCount = mesh->Vertices.size();

        ParticlesCount = flex->ParticleVertexMap.size();

        std::queue<int> queue;
        std::vector<int> visited(ParticlesCount, 0);
        std::vector<int> nearestAnchors(ParticlesCount, -1);
        std::vector<DirectX::XMFLOAT4> particles(ParticlesCount);

        for (int i = 0; i < verticesCount; i++) {
            const auto& vertex = mesh->Vertices[i];

            FlexVertices.emplace_back(FLEX_VERTEX_EXT{ {
                vertex.Position.x, vertex.Position.y, vertex.Position.z, 1.f }, 0.f
            });
        }

        for (int i = 0; i < ParticlesCount; i++) {
            auto vertexID = flex->ParticleVertexMap[i];
            auto& vertex = mesh->Vertices[vertexID];
            FlexVertices[vertexID].MixFactor = flex->InvMass[i];
        }

        for (int particleID = 0; particleID < flex->ParticleVertexMap.size(); particleID++) {
            auto vertexID = flex->ParticleVertexMap[particleID];
            const auto& vertex = mesh->Vertices[vertexID];

            DirectX::XMVECTOR position = DirectX::XMVectorZero();
            DirectX::XMVECTOR vertexPos = XMLoadFloat3(&vertex.Position);

            for (int i = 0; i < 4; i++) {
                auto boneIndex = vertex.BoneIndices[i];
                if (boneIndex != 0xFF) {
                    auto weight = vertex.BoneWeights[i];
                    const auto& skinTransform = meshComponent.SkinningTransforms[boneIndex];
                    auto transform = XMVectorScale(XMVector3Transform(vertexPos, XMLoadFloat4x4(&skinTransform)), weight / FLEX_NORMALIZE_SCLAE);
                    position = XMVectorAdd(position, transform);
                }
            }

            position = XMVector3Transform(position, XMLoadFloat4x4(&transformComponent.World));
            XMStoreFloat4(&particles[particleID], XMVectorSetW(position, FlexVertices[vertexID].MixFactor));
        }

        ClothMesh cloth((Vec4*)particles.data(), ParticlesCount, flex->ActiveParticleIndies.data(), flex->ActiveParticleIndies.size(), 1.f, 1.f, true);

        nearestAnchors.resize(ParticlesCount, -1);
        for (int particleID = 0; particleID < ParticlesCount; particleID++) {
            if (flex->InvMass[particleID] == 0.0f) {
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

        /*for (int particleID = 0; particleID < ParticlesCount; particleID++) {
            if (nearestAnchors[particleID] > 0 && nearestAnchors[particleID] != particleID) {
                float distance{};
                cloth.mConstraintIndices.push_back(particleID);
                cloth.mConstraintIndices.push_back(nearestAnchors[particleID]);

                XMVECTOR vPoint1 = XMLoadFloat4(&particles[particleID]);
                XMVECTOR vPoint2 = XMLoadFloat4(&particles[nearestAnchors[particleID]]);
                XMVECTOR vDiff = XMVectorSubtract(vPoint2, vPoint1);
                XMVECTOR vLength = XMVector4Length(vDiff);

                XMStoreFloat(&distance, vLength);

                cloth.mConstraintRestLengths.push_back(distance * 1.1);
                cloth.mConstraintCoefficients.push_back(-0.9f);
            }
        }*/

        Content = std::make_unique<NvFlexContent>(library, ParticlesCount);

        Content->map();

        for (int particleID = 0; particleID < flex->ParticleVertexMap.size(); particleID++) {
            Content->Phases[particleID] = NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseSelfCollideFilter);
            Content->Velocities[particleID] = { 0.f, 0.f, 0.f };
        }

        for (int i = 0; i < ParticlesCount; i++)
            Content->Particles[i] = particles[i];

        for (auto particleID : flex->ActiveParticleIndies) {
            Content->Triangles.push_back(particleID);
            Content->triangleNormals.push_back({ 0.0f, 0.0f, 1.0f });
        };

        auto springCount = cloth.mConstraintIndices.size() / 2;
        for (int i = 0; i < springCount; i++) {
            Content->springIndices.push_back(cloth.mConstraintIndices[i * 2]);
            Content->springIndices.push_back(cloth.mConstraintIndices[i * 2 + 1]);
            Content->springLengths.push_back(cloth.mConstraintRestLengths[i]);
            Content->springStiffness.push_back(cloth.mConstraintCoefficients[i]);
        }

        Content->unmap();

        bInitialized = true;
    }
}