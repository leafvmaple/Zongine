#include "NVFlexComponent.h"

#include "../Entities/EntityManager.h"
#include "../Managers/AssetManager.h"
#include "../Managers/DeviceManager.h"

#include "../Components/TransformComponent.h"

#include "NVFlex/include/NvFlexExt.h"

namespace Zongine {
    using DirectX::XMVector3Transform;
    using DirectX::XMVECTOR;
    using DirectX::XMVectorSetW;
    using DirectX::XMLoadFloat4x4;
    using DirectX::XMVectorScale;
    using DirectX::XMVectorAdd;

    void NvFlexComponent::Initialize(const Entity& entity, NvFlexLibrary* library) {
        auto& transformComponent = entity.GetComponent<TransformComponent>();
        auto& meshComponent = entity.GetComponent<MeshComponent>();

        auto flex = AssetManager::GetInstance().GetNvFlexAsset(Path);
        auto mesh = AssetManager::GetInstance().GetMeshAsset(MeshPath);

        for (int i = 0; i < mesh->Vertices.size(); i++) {
            const auto& vertex = mesh->Vertices[i];
            auto invMass = flex->InvMass[i];

            FlexVertices.emplace_back(FLEX_VERTEX_EXT{ {
                vertex.Position.x, vertex.Position.y, vertex.Position.z, 1.f }, invMass
            });

            if (invMass != 0) {
                ParticleVertices.push_back(i);
            }
        }

        Content = std::make_unique<NvFlexContent>(library, ParticleVertices.size());

        auto& particles = Content->Particles;
        auto& phases = Content->Phases;
        auto& velocities = Content->Velocities;

        particles.map();
        phases.map();
        velocities.map();

        for (int particleID = 0; particleID < ParticleVertices.size(); particleID++) {
            auto vertexID = ParticleVertices[particleID];
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

            phases[particleID] = NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseSelfCollideFilter);
            velocities[particleID] = { 0.f, 0.f, 0.f };
        }

        particles.unmap();
        phases.unmap();
        velocities.unmap();

        bInitialized = true;
    }
}