#include "AnimationSystem.h"

#include "Include/Maths.h"

#include "Entities/World.h"
#include "Managers/AssetManager.h"

#include "Components/AnimationComponent.h"
#include "Components/AnimationStateComponents.h"
#include "Components/SkeletonComponent.h"
#include "Components/MeshComponent.h"

#include "Animation/AnimStateMachineBuilder.h"

#include <algorithm>

namespace Zongine {
    void AnimationSystem::Tick(float deltaTime) {
        float deltaSeconds = deltaTime;

        // First, check for unloaded state machines and load them
        World::GetInstance().ForEach<AnimStateMachineComponent>(
            [this](auto entityID, auto& stateMachineComp) {
            if (!stateMachineComp.IsLoaded && !stateMachineComp.StateMachineAssetPath.empty()) {
                _LoadStateMachineFromAsset(entityID, stateMachineComp);
            }
        });

        // Then update all state machines (based on ECS components)
        World::GetInstance().ForEach<AnimStateMachineRuntimeComponent>(
            [this, deltaSeconds](auto entityID, auto& runtime) {
            _UpdateStateMachine(entityID, deltaSeconds);
        });

        // Finally update all animations
        World::GetInstance().ForEach<AnimationComponent>(
            [this, deltaTime](auto entityID, auto& animationComponent) {

            auto& world = World::GetInstance();
            auto& skeletonComponent = world.Get<SkeletonComponent>(entityID);

            if (animationComponent.Path.empty())
                return;

            auto animation = AssetManager::GetInstance().GetAnimationAsset(animationComponent.Path);
            auto skeleton = AssetManager::GetInstance().GetSkeletonAsset(skeletonComponent.Path);

            if (!animation || !skeleton)
                return;

            // Apply speed multiplier (convert seconds to milliseconds for animation timeline)
            int nDeltaTimeMs = static_cast<int>(deltaTime * 1000.0f);
            int scaledDeltaTime = static_cast<int>(nDeltaTimeMs * animationComponent.Speed);
            animationComponent.nPlayTime += scaledDeltaTime;

            // Handle looping
            uint64_t nAnimationTime;
            if (animationComponent.Loop) {
                nAnimationTime = animationComponent.nPlayTime % animation->AnimationLength;
            } else {
                nAnimationTime = std::min<uint64_t>(animationComponent.nPlayTime, animation->AnimationLength - 1);
            }

            int nFrame = int(nAnimationTime / animation->FrameLength);
            float fInterpolation = (nAnimationTime - nFrame * animation->FrameLength) / (float)animation->FrameLength;

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
        auto& world = World::GetInstance();
        if (world.Has<MeshComponent>(entityID)) {
            auto& meshComponent = world.Get<MeshComponent>(entityID);
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

        for (auto childID : world.GetChildren(entityID)) {
            _MapSkeletonTransformsToMesh(childID, skeleton, animation);
        }
    }

    // ============================================
    // State Machine Logic Implementation (Pure ECS)
    // ============================================

    void AnimationSystem::_UpdateStateMachine(EntityID entityID, float deltaTime) {
        auto& world = World::GetInstance();
        if (!world.Has<AnimStateCollectionComponent>(entityID) ||
            !world.Has<AnimStateMachineRuntimeComponent>(entityID) ||
            !world.Has<AnimationComponent>(entityID)) {
            return;
        }

        auto& states = world.Get<AnimStateCollectionComponent>(entityID);
        auto& runtime = world.Get<AnimStateMachineRuntimeComponent>(entityID);
        auto& animComp = world.Get<AnimationComponent>(entityID);

        // Initialize state machine
        if (!runtime.Initialized) {
            _InitializeStateMachine(entityID);
            return;
        }

        if (runtime.IsTransitioning) {
            _UpdateTransition(entityID, deltaTime);
        } else {
            // Get current state
            auto stateIt = states.States.find(runtime.CurrentState);
            if (stateIt == states.States.end()) return;
            
            const auto& currentState = stateIt->second;

            // Update state time
            runtime.CurrentStateTime += deltaTime * currentState.Speed;

            // Update animation path
            if (animComp.Path != currentState.AnimationPath) {
                animComp.Path = currentState.AnimationPath;
                animComp.nPlayTime = 0;
            }

            // Calculate normalized time
            auto animation = AssetManager::GetInstance().GetAnimationAsset(currentState.AnimationPath);
            if (animation) {
                float animationDuration = animation->AnimationLength / 1000.0f;
                runtime.CurrentStateNormalizedTime = runtime.CurrentStateTime / animationDuration;

                // Handle looping
                if (currentState.Loop && runtime.CurrentStateNormalizedTime >= 1.0f) {
                    runtime.CurrentStateTime = 0.0f;
                    runtime.CurrentStateNormalizedTime = 0.0f;
                }
            }

            // Check state transitions
            _CheckTransitions(entityID);

            // Reset all triggers
            if (world.Has<AnimParameterCollectionComponent>(entityID)) {
                auto& params = world.Get<AnimParameterCollectionComponent>(entityID);
                for (auto& [name, param] : params.Parameters) {
                    if (param.Type == AnimParameterType::Trigger && param.BoolValue) {
                        param.BoolValue = false;
                    }
                }
            }
        }
    }

    void AnimationSystem::_InitializeStateMachine(EntityID entityID) {
        auto& world = World::GetInstance();
        auto& states = world.Get<AnimStateCollectionComponent>(entityID);
        auto& runtime = world.Get<AnimStateMachineRuntimeComponent>(entityID);
        auto& animComp = world.Get<AnimationComponent>(entityID);

        runtime.CurrentState = states.DefaultState;
        runtime.CurrentStateTime = 0.0f;
        runtime.CurrentStateNormalizedTime = 0.0f;
        runtime.Initialized = true;

        // Set initial animation
        auto stateIt = states.States.find(runtime.CurrentState);
        if (stateIt != states.States.end()) {
            animComp.Path = stateIt->second.AnimationPath;
            animComp.Speed = stateIt->second.Speed;
            animComp.Loop = stateIt->second.Loop;
            animComp.nPlayTime = 0;
        }
    }

    void AnimationSystem::_CheckTransitions(EntityID entityID) {
        auto& world = World::GetInstance();
        if (!world.Has<AnimTransitionCollectionComponent>(entityID) ||
            !world.Has<AnimParameterCollectionComponent>(entityID)) {
            return;
        }

        auto& transitions = world.Get<AnimTransitionCollectionComponent>(entityID);
        auto& params = world.Get<AnimParameterCollectionComponent>(entityID);
        auto& runtime = world.Get<AnimStateMachineRuntimeComponent>(entityID);

        for (const auto& transition : transitions.Transitions) {
            if (transition.FromState == runtime.CurrentState &&
                _CanTransition(transition, params, runtime.CurrentStateNormalizedTime)) {
                _StartTransition(entityID, transition);
                break;
            }
        }
    }

    void AnimationSystem::_StartTransition(EntityID entityID, const AnimTransition& transition) {
        auto& runtime = World::GetInstance().Get<AnimStateMachineRuntimeComponent>(entityID);

        runtime.IsTransitioning = true;
        runtime.NextState = transition.ToState;
        runtime.TransitionProgress = 0.0f;
        runtime.TransitionDuration = transition.TransitionDuration;
    }

    void AnimationSystem::_UpdateTransition(EntityID entityID, float deltaTime) {
        auto& world = World::GetInstance();
        auto& runtime = world.Get<AnimStateMachineRuntimeComponent>(entityID);
        auto& states = world.Get<AnimStateCollectionComponent>(entityID);
        auto& animComp = world.Get<AnimationComponent>(entityID);

        runtime.TransitionProgress += deltaTime / runtime.TransitionDuration;

        if (runtime.TransitionProgress >= 1.0f) {
            // Transition complete
            runtime.IsTransitioning = false;
            runtime.CurrentState = runtime.NextState;
            runtime.CurrentStateTime = 0.0f;
            runtime.CurrentStateNormalizedTime = 0.0f;
            runtime.NextState.clear();

            // Update animation
            auto stateIt = states.States.find(runtime.CurrentState);
            if (stateIt != states.States.end()) {
                animComp.Path = stateIt->second.AnimationPath;
                animComp.Speed = stateIt->second.Speed;
                animComp.Loop = stateIt->second.Loop;
                animComp.nPlayTime = 0;
            }
        }
        // TODO: Animation blending can be implemented here
    }

    bool AnimationSystem::_EvaluateCondition(
        const AnimCondition& condition, 
        const AnimParameterCollectionComponent& params) const {
        
        auto it = params.Parameters.find(condition.ParameterName);
        if (it == params.Parameters.end()) return false;

        const auto& param = it->second;

        if (param.Type == AnimParameterType::Trigger) {
            return param.BoolValue;
        }

        switch (condition.Type) {
        case AnimConditionType::Greater:
            if (param.Type == AnimParameterType::Float)
                return param.FloatValue > condition.FloatValue;
            else if (param.Type == AnimParameterType::Int)
                return param.IntValue > condition.IntValue;
            break;

        case AnimConditionType::Less:
            if (param.Type == AnimParameterType::Float)
                return param.FloatValue < condition.FloatValue;
            else if (param.Type == AnimParameterType::Int)
                return param.IntValue < condition.IntValue;
            break;

        case AnimConditionType::Equal:
            if (param.Type == AnimParameterType::Float)
                return std::abs(param.FloatValue - condition.FloatValue) < 0.001f;
            else if (param.Type == AnimParameterType::Int)
                return param.IntValue == condition.IntValue;
            else if (param.Type == AnimParameterType::Bool)
                return param.BoolValue == condition.BoolValue;
            break;

        case AnimConditionType::NotEqual:
            if (param.Type == AnimParameterType::Float)
                return std::abs(param.FloatValue - condition.FloatValue) >= 0.001f;
            else if (param.Type == AnimParameterType::Int)
                return param.IntValue != condition.IntValue;
            else if (param.Type == AnimParameterType::Bool)
                return param.BoolValue != condition.BoolValue;
            break;

        case AnimConditionType::GreaterOrEqual:
            if (param.Type == AnimParameterType::Float)
                return param.FloatValue >= condition.FloatValue;
            else if (param.Type == AnimParameterType::Int)
                return param.IntValue >= condition.IntValue;
            break;

        case AnimConditionType::LessOrEqual:
            if (param.Type == AnimParameterType::Float)
                return param.FloatValue <= condition.FloatValue;
            else if (param.Type == AnimParameterType::Int)
                return param.IntValue <= condition.IntValue;
            break;
        }

        return false;
    }

    bool AnimationSystem::_CanTransition(
        const AnimTransition& transition,
        const AnimParameterCollectionComponent& params,
        float normalizedTime) const {

        // Check exit time
        if (transition.HasExitTime && normalizedTime < transition.ExitTime) {
            return false;
        }

        // Check all conditions
        for (const auto& condition : transition.Conditions) {
            if (!_EvaluateCondition(condition, params)) {
                return false;
            }
        }

        return true;
    }

    void AnimationSystem::_LoadStateMachineFromAsset(EntityID entityID, AnimStateMachineComponent& stateMachineComp) {
        // Use AnimStateMachineBuilder to load from JSON
        if (AnimStateMachineBuilder::LoadFromJson(entityID, stateMachineComp.StateMachineAssetPath)) {
            stateMachineComp.IsLoaded = true;
        }
    }
}