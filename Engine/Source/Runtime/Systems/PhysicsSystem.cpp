#include "PhysicsSystem.h"

#include "Entities/EntityManager.h"
#include "Managers/AssetManager.h"

#include "Components/FlexibleComponent.h"
#include "Components/TransformComponent.h"
#include "Components/MeshComponent.h"

namespace Zongine {
    void PhysicsSystem::Tick(float fDeltaTime) {
        auto& entities = EntityManager::GetInstance().GetEntities<FlexibleComponent>();
        for (auto& [entityID, flexibleComponent] : entities) {
            auto& entity = EntityManager::GetInstance().GetEntity(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();
            auto& meshComponent = entity.GetComponent<MeshComponent>();

            auto mesh = AssetManager::GetInstance().GetMeshAsset(meshComponent.Path);

            for (auto& driver : flexibleComponent.Drivers) {
                auto driverInverseTransform = XMLoadFloat4x4(&mesh->Bones[driver.index].InversePoseTransform);
                auto driverModelTransform = XMLoadFloat4x4(&meshComponent.BoneModelTransforms[driver.index]);

                // TODO: Use World Transform
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
}