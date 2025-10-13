#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace Zongine {
    // ============================================
    // Animation State Components (Pure Data)
    // ============================================

    /**
     * Animation State Definition
     * Each entity can have multiple AnimState components, distinguished by name
     */
    struct AnimState {
        std::string Name;           // State name
        std::string AnimationPath;  // Animation resource path
        bool Loop = true;           // Whether to loop
        float Speed = 1.0f;         // Playback speed multiplier
    };

    /**
     * Animation State Collection Component
     * Stores all animation states for an entity
     */
    struct AnimStateCollectionComponent {
        std::unordered_map<std::string, AnimState> States;
        std::string DefaultState;  // Default state name
    };

    // ============================================
    // Animation Transition Components (Pure Data)
    // ============================================

    /**
     * Transition Condition Type
     */
    enum class AnimConditionType {
        Greater,        // >
        Less,           // <
        Equal,          // ==
        NotEqual,       // !=
        GreaterOrEqual, // >=
        LessOrEqual     // <=
    };

    /**
     * Transition Condition
     */
    struct AnimCondition {
        std::string ParameterName;
        AnimConditionType Type;
        
        union {
            float FloatValue;
            int IntValue;
            bool BoolValue;
        };

        AnimCondition() : FloatValue(0.0f) {}
    };

    /**
     * State Transition Definition
     */
    struct AnimTransition {
        std::string FromState;
        std::string ToState;
        std::vector<AnimCondition> Conditions;  // All conditions must be met (AND)
        float TransitionDuration = 0.3f;        // Transition duration (seconds)
        bool HasExitTime = false;               // Whether needs to finish playing
        float ExitTime = 1.0f;                  // Exit time (0-1)
    };

    /**
     * Animation Transition Collection Component
     * Stores all state transitions for an entity
     */
    struct AnimTransitionCollectionComponent {
        std::vector<AnimTransition> Transitions;
    };

    // ============================================
    // Animation Parameter Components (Pure Data)
    // ============================================

    /**
     * Parameter Type
     */
    enum class AnimParameterType {
        Float,
        Int,
        Bool,
        Trigger
    };

    /**
     * Parameter Value
     */
    struct AnimParameter {
        std::string Name;
        AnimParameterType Type;
        
        union {
            float FloatValue;
            int IntValue;
            bool BoolValue;
        };

        AnimParameter() : FloatValue(0.0f) {}
    };

    /**
     * Animation Parameter Collection Component
     * Stores all animation parameters for an entity
     */
    struct AnimParameterCollectionComponent {
        std::unordered_map<std::string, AnimParameter> Parameters;
    };

    // ============================================
    // Animation State Machine Runtime Component (Pure Data)
    // ============================================

    /**
     * Animation State Machine Runtime State
     * Records current playback state, transition info, etc.
     */
    struct AnimStateMachineRuntimeComponent {
        std::string CurrentState;           // Current state name
        std::string NextState;              // Next state name (during transition)
        float CurrentStateTime = 0.0f;      // Current state playback time (seconds)
        float CurrentStateNormalizedTime = 0.0f;  // Normalized time (0-1)
        
        bool IsTransitioning = false;       // Whether transitioning
        float TransitionProgress = 0.0f;    // Transition progress (0-1)
        float TransitionDuration = 0.0f;    // Total transition duration (seconds)
        
        bool Initialized = false;           // Whether initialized
    };
}
