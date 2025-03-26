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

            std::vector<DirectX::XMMATRIX> matrices;
            for (int i = 0; i < skeleton->Bones.size(); i++) {
                auto flag = animation->BoneFlag[i];
                auto& curClip = animation->Clip[i][nFrame];
                auto& nextClip = animation->Clip[i][(nFrame + 1) % animation->Clip[i].size()];

                AnimationSRT rts{};

                XMFloat4Slerp(&rts.SRotation, &curClip.SRotation, &nextClip.SRotation, fInterpolation);
                XMFloat4Slerp(&rts.Rotation, &curClip.Rotation, &nextClip.Rotation, fInterpolation);
                XMFloat3Slerp(&rts.Scale, &curClip.Scale, &nextClip.Scale, fInterpolation);
                XMFloat3Slerp(&rts.Translation, &curClip.Translation, &nextClip.Translation, fInterpolation);

                auto& matrix = matrices.emplace_back(DirectX::XMMatrixIdentity());

                if (flag & BONE_FLAG_AFFLINE)
                    matrix = XMMatrixAffineTransformation(
                        XMLoadFloat3(&rts.Scale), g_XMZero, XMLoadFloat4(&rts.Rotation), XMLoadFloat3(&rts.Translation));
                else
                    matrix = XMMatrixTransformation(g_XMZero, XMLoadFloat4(&rts.SRotation),
                        XMLoadFloat3(&rts.Scale), g_XMZero, XMLoadFloat4(&rts.Rotation), XMLoadFloat3(&rts.Translation));
            }

            for (auto childID : entity.GetChildren()) {
                auto& child = m_EntityManager->GetEntity(childID);
                auto& meshComponent = child.GetComponent<MeshComponent>();
                auto mesh = m_ResourceManager->GetMeshAsset(meshComponent.Path);

                auto& skeletonMeshMap = m_SkeletonMeshMap[skeletonComponent.Path][meshComponent.Path];
                if (skeletonMeshMap.empty()) {
                    _InitialSkeletonMeshMap(skeletonMeshMap, skeleton.get(), mesh.get());
                }
            };
        }
    }

    void AnimationSystem::_InitialSkeletonMeshMap(std::vector<int>& map, const SkeletonAsset* skeleton, const MeshAsset* mesh) {
        map.resize(mesh->BoneMap.size(), -1);
        for (auto [boneName, index] : mesh->BoneMap) {
            auto it = std::find_if(skeleton->Bones.begin(), skeleton->Bones.end(), [boneName](const SkeletonBone& bone) {
                return bone.Name == boneName;
                });
            if (it != skeleton->Bones.end()) {
                map[index] = it - skeleton->Bones.begin();
            }
        }
    }
}