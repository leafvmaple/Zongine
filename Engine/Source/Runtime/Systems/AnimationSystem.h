#pragma once

#include "Include/Types.h"

#include <string>
#include <memory>
#include <vector>
#include <DirectXMath.h>
#include <unordered_map>

using DirectX::XMFLOAT4X4;
using DirectX::XMMATRIX;

namespace Zongine {
    struct ManagerList;
    struct SkeletonAsset;
    struct MeshAsset;
    struct SkeletonComponent;
    struct AnimationComponent;
    struct AnimStateCollectionComponent;
    struct AnimTransitionCollectionComponent;
    struct AnimParameterCollectionComponent;
    struct AnimStateMachineRuntimeComponent;
    struct AnimStateMachineComponent;
    struct AnimCondition;
    struct AnimTransition;
    class Entity;

    /**
     * 动画系统
     * 
     * 负责更新所有实体的动画状态机和动画组件
     * 完全基于ECS架构:
     * 1. 状态机驱动的动画切换（纯组件数据）
     * 2. 骨骼动画播放和插值
     * 3. 蒙皮变换计算
     */
    class AnimationSystem {
    public:
        void Tick(int nDeltaTime);

    private:
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>>> m_SkeletonMeshMap{};

        // 状态机逻辑
        void _LoadStateMachineFromAsset(Entity& entity, AnimStateMachineComponent& stateMachineComp);
        void _UpdateStateMachine(Entity& entity, float deltaTime);
        void _InitializeStateMachine(Entity& entity);
        void _CheckTransitions(Entity& entity);
        void _StartTransition(Entity& entity, const AnimTransition& transition);
        void _UpdateTransition(Entity& entity, float deltaTime);
        bool _EvaluateCondition(const AnimCondition& condition, const AnimParameterCollectionComponent& params) const;
        bool _CanTransition(const AnimTransition& transition, const AnimParameterCollectionComponent& params, float normalizedTime) const;

        // 骨骼动画逻辑
        void _UpdateSkeletonSpaceRecursive(
            AnimationComponent& component,
            std::shared_ptr<SkeletonAsset> skeleton,
            std::vector<XMFLOAT4X4>& localTransform,
            int skeletonIndex
        );

        void _MapSkeletonTransformsToMesh(EntityID entityID, const SkeletonComponent& skeleton, const AnimationComponent& animation);
    };
}