#include "AnimationSystem.h"

#include "Include/Maths.h"
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

    void AnimationSystem::Tick(int nDeltaTime) {
        auto& entities = m_EntityManager->GetEntities<AnimationComponent>();
        for (auto& [entityID, animationComponent] : entities) {
            auto& entity = m_EntityManager->GetEntity(entityID);
            auto& skeletonComponent = entity.GetComponent<SkeletonComponent>();

            auto animation = m_ResourceManager->GetAnimationAsset(animationComponent.Path);
            auto skeleton = m_ResourceManager->GetSkeletonAsset(skeletonComponent.Path);

            animationComponent.nPlayTime += nDeltaTime;
            uint64_t nAnimationTime = animationComponent.nPlayTime % animation->AnimationLength;
            int nFrame = int(nAnimationTime / animation->FrameLength);
            float fInterpolation = (nAnimationTime - nFrame * animation->FrameLength) / animation->FrameLength;

            std::vector<XMFLOAT4X4> matrices;
            auto skeletonBoneCount = skeleton->Bones.size();

            for (int i = 0; i < skeletonBoneCount; i++) {
                auto flag = animation->BoneFlag[i];
                auto& curClip = animation->Clip[i][nFrame];
                auto& nextClip = animation->Clip[i][(nFrame + 1) % animation->Clip[i].size()];

                AnimationSRT rts{};

                XMFloat4Slerp(&rts.SRotation, &curClip.SRotation, &nextClip.SRotation, fInterpolation);
                XMFloat4Slerp(&rts.Rotation, &curClip.Rotation, &nextClip.Rotation, fInterpolation);
                XMFloat3Slerp(&rts.Scale, &curClip.Scale, &nextClip.Scale, fInterpolation);
                XMFloat3Slerp(&rts.Translation, &curClip.Translation, &nextClip.Translation, fInterpolation);

                auto& matrix = matrices.emplace_back();

                XMStoreFloat4x4(&matrix, XMMatrixTransformation(g_XMZero, XMLoadFloat4(&rts.SRotation),
                        XMLoadFloat3(&rts.Scale), g_XMZero, XMLoadFloat4(&rts.Rotation), XMLoadFloat3(&rts.Translation)));
            }

            if (animationComponent.Matrix.empty())
                animationComponent.Matrix.resize(skeletonBoneCount);

            animationComponent.Matrix[skeleton->nRootBoneIndex] = matrices[skeleton->nRootBoneIndex];
            _UpdateSkeletonTransform(animationComponent, skeleton, matrices, skeleton->nRootBoneIndex);
            _UpdateMeshBonesTransform(entityID, skeletonComponent, animationComponent);
        }
    }

    void AnimationSystem::_UpdateSkeletonTransform(
        AnimationComponent& component,
        std::shared_ptr<SkeletonAsset> skeleton,
        std::vector<XMFLOAT4X4>& matrices,
        int skeletonIndex) {

        auto& skeletonBone = skeleton->Bones[skeletonIndex];
        for (auto& childIndex : skeletonBone.Children) {
            XMStoreFloat4x4(&component.Matrix[childIndex], XMLoadFloat4x4(&matrices[childIndex]) * XMLoadFloat4x4(&component.Matrix[skeletonIndex]));
            _UpdateSkeletonTransform(component, skeleton, matrices, childIndex);
        }
    }

    void AnimationSystem::_UpdateMeshBonesTransform(EntityID entityID, const SkeletonComponent& skeleton, const AnimationComponent& animation) {
        auto& entity = m_EntityManager->GetEntity(entityID);
        if (m_EntityManager->HasComponent<MeshComponent>(entityID)) {
            auto& meshComponent = entity.GetComponent<MeshComponent>();
            auto mesh = m_ResourceManager->GetMeshAsset(meshComponent.Path);

            if (meshComponent.Matrix.empty()) {
                meshComponent.Matrix.resize(mesh->Bones.size());
                meshComponent.Offset.resize(mesh->Bones.size());
            }

            auto& meshSkeletonMap = m_ResourceManager->GetMeshSkeletonMap(skeleton.Path, meshComponent.Path);

            for (int i = 0; i < meshSkeletonMap.size(); i++) {
                auto skeletonIndex = meshSkeletonMap[i];
                if (skeletonIndex != -1)
                    meshComponent.Matrix[i] = animation.Matrix[skeletonIndex];
                else
                    XMStoreFloat4x4(&meshComponent.Matrix[i], XMMatrixInverse(nullptr, XMLoadFloat4x4(&mesh->Bones[i].Offset)));
                XMStoreFloat4x4(&meshComponent.Offset[i], XMLoadFloat4x4(&mesh->Bones[i].Offset) * XMLoadFloat4x4(&meshComponent.Matrix[i]));
            }
        }

        for (auto childID : entity.GetChildren()) {
            _UpdateMeshBonesTransform(childID, skeleton, animation);
        }
    }
}