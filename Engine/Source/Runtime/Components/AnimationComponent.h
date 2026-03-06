#pragma once

#include <DirectXMath.h>
#include <string>
#include <vector>

namespace Zongine {
    /**
     * Animation Component
     * 
     * Supports two usage modes:
     * 1. Direct single animation playback (set Path and nPlayTime)
     * 2. State machine control via AnimationStateMachineComponent
     */
    struct AnimationComponent {
        std::string Path{};          // Current animation path being played
        int nPlayTime{};             // Current playback time (milliseconds)
        float Speed = 1.0f;          // Playback speed multiplier
        bool Loop = true;            // Whether to loop playback

        std::vector<DirectX::XMFLOAT4X4> ModelTransforms{};  // Bone model-space transform matrices
    };

    /**
     * Animation State Machine Component
     * 
     * Similar to Unity's Animator component, stores state machine asset path
     * AssetManager loads state machine configuration to entity during initialization
     */
    struct AnimStateMachineComponent {
        std::string StateMachineAssetPath{};  // State machine JSON file path
        bool IsLoaded = false;                // Whether the state machine has been loaded
    };
}