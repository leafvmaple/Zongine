#include "AnimationSystem.h"

#include "Include/Maths.h"

#include "Entities/EntityManager.h"
#include "Managers/AssetManager.h"

#include "Components/AnimationComponent.h"
#include "Components/SkeletonComponent.h"
#include "Components/MeshComponent.h"

namespace Zongine {
    void AnimationSystem::Tick(int nDeltaTime) {
        EntityManager::GetInstance().ForEach<AnimationComponent>(
            [this, nDeltaTime](auto entityID, auto& animationComponent) {

            auto& entity = EntityManager::GetInstance().GetEntity(entityID);
            auto& skeletonComponent = entity.GetComponent<SkeletonComponent>();

            auto animation = AssetManager::GetInstance().GetAnimationAsset(animationComponent.Path);
            auto skeleton = AssetManager::GetInstance().GetSkeletonAsset(skeletonComponent.Path);

            animationComponent.nPlayTime += nDeltaTime;
            uint64_t nAnimationTime = animationComponent.nPlayTime % animation->AnimationLength;
            int nFrame = int(nAnimationTime / animation->FrameLength);
            float fInterpolation = (nAnimationTime - nFrame * animation->FrameLength) / animation->FrameLength;

            std::vector<XMFLOAT4X4> localTransforms;
            auto skeletonBoneCount = skeleton->Bones.size();

            for (int i = 0; i < skeletonBoneCount; i++) {
                // auto flag = animation->BoneFlag[i];
                auto& curClip = animation->Clip[i][nFrame];
                auto& nextClip = animation->Clip[i][(nFrame + 1) % animation->Clip[i].size()];

                AnimationSRT rts{};

                XMFloat4Slerp(&rts.SRotation, &curClip.SRotation, &nextClip.SRotation, fInterpolation);
                XMFloat4Slerp(&rts.Rotation, &curClip.Rotation, &nextClip.Rotation, fInterpolation);
                XMFloat3Slerp(&rts.Scale, &curClip.Scale, &nextClip.Scale, fInterpolation);
                XMFloat3Slerp(&rts.Translation, &curClip.Translation, &nextClip.Translation, fInterpolation);

                auto& matrix = localTransforms.emplace_back();

                XMStoreFloat4x4(&matrix, XMMatrixTransformation(g_XMZero, XMLoadFloat4(&rts.SRotation),
                    XMLoadFloat3(&rts.Scale), g_XMZero, XMLoadFloat4(&rts.Rotation), XMLoadFloat3(&rts.Translation)));
            }

            if (animationComponent.ModelTransforms.empty())
                animationComponent.ModelTransforms.resize(skeletonBoneCount);

            animationComponent.ModelTransforms[skeleton->nRootBoneIndex] = localTransforms[skeleton->nRootBoneIndex];
            _UpdateSkeletonSpaceRecursive(animationComponent, skeleton, localTransforms, skeleton->nRootBoneIndex);
            _MapSkeletonTransformsToMesh(entityID, skeletonComponent, animationComponent);
        });
    }

    void AnimationSystem::_UpdateSkeletonSpaceRecursive(
        AnimationComponent& component,
        std::shared_ptr<SkeletonAsset> skeleton,
        std::vector<XMFLOAT4X4>& localTransform,
        int skeletonIndex) {

        auto& skeletonBone = skeleton->Bones[skeletonIndex];
        for (auto& childIndex : skeletonBone.Children) {
            XMStoreFloat4x4(&component.ModelTransforms[childIndex], XMLoadFloat4x4(&localTransform[childIndex]) * XMLoadFloat4x4(&component.ModelTransforms[skeletonIndex]));
            _UpdateSkeletonSpaceRecursive(component, skeleton, localTransform, childIndex);
        }
    }

    void AnimationSystem::_MapSkeletonTransformsToMesh(EntityID entityID, const SkeletonComponent& skeleton, const AnimationComponent& animation) {
        auto& entity = EntityManager::GetInstance().GetEntity(entityID);
        if (entity.HasComponent<MeshComponent>()) {
            auto& meshComponent = entity.GetComponent<MeshComponent>();
            auto mesh = AssetManager::GetInstance().GetMeshAsset(meshComponent.Path);

            auto& meshSkeletonMap = AssetManager::GetInstance().GetMeshSkeletonMap(skeleton.Path, meshComponent.Path);

            assert(mesh->Bones.size() == meshSkeletonMap.size());

            for (int i = 0; i < meshSkeletonMap.size(); i++) {
                auto skeletonIndex = meshSkeletonMap[i];
                if (skeletonIndex == -1)
                    continue;

                meshComponent.BoneModelTransforms[i] = animation.ModelTransforms[skeletonIndex];

                XMStoreFloat4x4(&meshComponent.SkinningTransforms[i],
                    XMLoadFloat4x4(&mesh->Bones[i].InversePoseTransform) * XMLoadFloat4x4(&meshComponent.BoneModelTransforms[i]));
            }
        }

        for (auto childID : entity.GetChildren()) {
            _MapSkeletonTransformsToMesh(childID, skeleton, animation);
        }
    }
}