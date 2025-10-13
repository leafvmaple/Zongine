#pragma once

#include "Components/AnimationStateComponents.h"
#include "Entities/Entity.h"

namespace Zongine {
    /**
     * Animation State Machine Builder
     * Provides a convenient API to set up animation state machine for entities
     * 
     * Usage example:
     * AnimStateMachineBuilder builder(entity);
     * builder.AddState("Idle", "idle.anim")
     *        .AddState("Walk", "walk.anim")
     *        .AddTransition("Idle", "Walk", 0.2f)
     *        .AddCondition("Idle", "Walk", "Speed", AnimConditionType::Greater, 0.1f)
     *        .SetDefaultState("Idle")
     *        .Build();
     */
    class AnimStateMachineBuilder {
    public:
        AnimStateMachineBuilder(Entity& entity);

        // Add a state
        AnimStateMachineBuilder& AddState(const std::string& name, const std::string& animPath, 
            bool loop = true, float speed = 1.0f);

        // Add a transition
        AnimStateMachineBuilder& AddTransition(const std::string& from, const std::string& to, 
            float duration = 0.3f);

        // Add condition to the last added transition
        AnimStateMachineBuilder& AddCondition(const std::string& from, const std::string& to,
            const std::string& paramName, AnimConditionType type, float value);
        AnimStateMachineBuilder& AddCondition(const std::string& from, const std::string& to,
            const std::string& paramName, AnimConditionType type, int value);
        AnimStateMachineBuilder& AddCondition(const std::string& from, const std::string& to,
            const std::string& paramName, AnimConditionType type, bool value);

        // Set exit time
        AnimStateMachineBuilder& SetExitTime(const std::string& from, const std::string& to,
            bool hasExitTime, float exitTime = 1.0f);

        // Set default state
        AnimStateMachineBuilder& SetDefaultState(const std::string& state);

        // Finish building, ensure components are added
        void Build();

        // Serialize to JSON file
        bool SaveToJson(const std::string& filePath) const;

        // Deserialize from JSON file and build state machine
        static bool LoadFromJson(Entity& entity, const std::string& filePath);

    private:
        Entity& m_Entity;
        AnimStateCollectionComponent m_States;
        AnimTransitionCollectionComponent m_Transitions;

        AnimTransition* FindTransition(const std::string& from, const std::string& to);
    };

    /**
     * Animation Parameter Helper
     * Provides a convenient API to set and get animation parameters
     */
    class AnimParameterHelper {
    public:
        static void SetFloat(Entity& entity, const std::string& name, float value);
        static void SetInt(Entity& entity, const std::string& name, int value);
        static void SetBool(Entity& entity, const std::string& name, bool value);
        static void SetTrigger(Entity& entity, const std::string& name);
        static void ResetTrigger(Entity& entity, const std::string& name);

        static float GetFloat(const Entity& entity, const std::string& name, float defaultValue = 0.0f);
        static int GetInt(const Entity& entity, const std::string& name, int defaultValue = 0);
        static bool GetBool(const Entity& entity, const std::string& name, bool defaultValue = false);

    private:
        static AnimParameter& GetOrCreateParameter(Entity& entity, const std::string& name, AnimParameterType type);
    };
}
