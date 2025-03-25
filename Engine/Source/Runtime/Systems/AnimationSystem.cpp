#include "AnimationSystem.h"

#include "Managers/Mananger.h"

#include "Components/AnimationComponent.h"
#include "Components/SkeletonComponent.h"
#include "Components/MeshComponent.h"

namespace Zongine {
    bool AnimationSystem::Initialize(const ManagerList& info) {
        m_EntityManager = info.entityManager;
        m_ResourceManager = info.resourceManager;
        return true;
    }

    void AnimationSystem::Tick(float fDeltaTime) {
        auto& entities = m_EntityManager->GetEntities<AnimationComponent>();
        for (auto& [entityID, animationComponent] : entities) {
            auto& entity = m_EntityManager->GetEntity(entityID);
            auto& skeletonComponent = entity.GetComponent<SkeletonComponent>();
            auto& meshComponent = entity.GetComponent<MeshComponent>();

            auto mesh = m_ResourceManager->GetMeshAsset(meshComponent.Path);
            auto skeleton = m_ResourceManager->GetSkeletonAsset(skeletonComponent.Path);

            auto& skeletonMeshMap = m_SkeletonMeshMap[skeletonComponent.Path][meshComponent.Path];
            if (skeletonMeshMap.empty()) {
                for (const auto& bone : skeleton->Bones) {
                    int index = mesh->BoneMap.find(bone.Name) != mesh->BoneMap.end() ? mesh->BoneMap[bone.Name] : -1;
                    skeletonMeshMap.push_back(index);
                }
            }

            /*for (auto& [name, animation] : animationComponent.Animations) {
                auto& skeleton = skeletonComponent.Skeleton;
                auto& skeletonMesh = m_SkeletonMeshMap[name];
                for (int i = 0; i < skeleton.Bones.size(); i++) {
                    auto& bone = skeleton.Bones[i];
                    auto& mesh = skeletonMesh[bone.Name];
                    auto& transform = skeleton.Bones[i].Transform;
                    auto& meshTransform = mesh.Transform;
                    transform = meshTransform;
                }
            }*/
        }
    }
}