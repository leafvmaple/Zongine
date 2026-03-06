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

    /**
     * Animation System
     * 
     * Responsible for updating animation state machines and animation components for all entities
     * Fully based on ECS architecture:
     * 1. State machine driven animation switching (pure component data)
     * 2. Skeletal animation playback and interpolation
     * 3. Skinning transform computation
     */
    class AnimationSystem {
    public:
        void Tick(float deltaTime);

    private:
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>>> m_SkeletonMeshMap{};

        // State machine logic
        void _LoadStateMachineFromAsset(EntityID entityID, AnimStateMachineComponent& stateMachineComp);
        void _UpdateStateMachine(EntityID entityID, float deltaTime);
        void _InitializeStateMachine(EntityID entityID);
        void _CheckTransitions(EntityID entityID);
        void _StartTransition(EntityID entityID, const AnimTransition& transition);
        void _UpdateTransition(EntityID entityID, float deltaTime);
        bool _EvaluateCondition(const AnimCondition& condition, const AnimParameterCollectionComponent& params) const;
        bool _CanTransition(const AnimTransition& transition, const AnimParameterCollectionComponent& params, float normalizedTime) const;

        // Skeletal animation logic
        void _UpdateSkeletonSpaceRecursive(
            AnimationComponent& component,
            std::shared_ptr<SkeletonAsset> skeleton,
            std::vector<XMFLOAT4X4>& localTransform,
            int skeletonIndex
        );

        void _MapSkeletonTransformsToMesh(EntityID entityID, const SkeletonComponent& skeleton, const AnimationComponent& animation);
    };
}