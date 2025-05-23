#include "FlexibleComponent.h"

#include "MeshComponent.h"

#include "../Entities/EntityManager.h"
#include "../Managers/AssetManager.h"

namespace Zongine {
    static void _InitializeDrivenBones(int parentIndex, std::vector<uint32_t>& drivenBones, const std::vector<BONE>& bones) {
        auto& bone = bones[parentIndex];
        for (auto child : bone.Children) {
            auto& childBone = bones[child];
            if (childBone.Name.starts_with("fb"))
                drivenBones.push_back(child);
            _InitializeDrivenBones(child, drivenBones, bones);
        }
    }

    void FlexibleComponent::Initialize(const Entity& entity) {
        auto& meshComponent = entity.GetComponent<MeshComponent>();
        auto mesh = AssetManager::GetInstance().GetMeshAsset(meshComponent.Path);

        for (auto& driver : Drivers) {
            _InitializeDrivenBones(driver.index, driver.driven, mesh->Bones);
        }
    }
}