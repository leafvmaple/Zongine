#include "PhysicsSystem.h"

#include "Components/FlexibleComponent.h"
#include "Components/TransformComponent.h"
#include "Components/MeshComponent.h"

namespace Zongine {
    bool PhysicsSystem::Initialize(const ManagerList& info) {
        m_EntityManager = info.entityManager;
        m_AssetManager = info.assetManager;
        return true;
    }
    void PhysicsSystem::Tick(float fDeltaTime) {
        // Physics simulation logic goes here

        auto& entities = m_EntityManager->GetEntities<FlexibleComponent>();
        for (auto& [entityID, flexibleComponent] : entities) {
            auto& entity = m_EntityManager->GetEntity(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();
            auto& meshComponent = entity.GetComponent<MeshComponent>();

            auto mesh = m_AssetManager->GetMeshAsset(meshComponent.Path);

            for (auto& driver : flexibleComponent.Drivers) {
                if (driver.driven.empty())
                    _InitializeDrivenBones(driver.index, driver.driven, mesh->Bones);

                auto driverInverseTransform = XMLoadFloat4x4(&mesh->Bones[driver.index].InversePoseTransform);
                auto driverModelTransform = XMLoadFloat4x4(&meshComponent.BoneModelTransforms[driver.index]);

                for (auto drivenIndex : driver.driven) {
                    auto& driven = mesh->Bones[drivenIndex];

                    XMStoreFloat4x4(&meshComponent.BoneModelTransforms[drivenIndex],
                        XMLoadFloat4x4(&driven.PhysicsPoseTransform) * driverInverseTransform * driverModelTransform);

                    XMStoreFloat4x4(&meshComponent.SkinningTransforms[drivenIndex],
                        XMLoadFloat4x4(&driven.InversePoseTransform) * XMLoadFloat4x4(&meshComponent.BoneModelTransforms[drivenIndex]));
                }
            }
        }
    }

    void PhysicsSystem::_InitializeDrivenBones(
        int parentIndex,
        std::vector<uint32_t>& drivenBones,
        const std::vector<BONE>& bones
    ) {
        auto& bone = bones[parentIndex];
        for (auto child : bone.Children) {
            drivenBones.push_back(child);
            _InitializeDrivenBones(child, drivenBones, bones);
        }
    }
}