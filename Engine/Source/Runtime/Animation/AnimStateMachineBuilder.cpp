#include "AnimStateMachineBuilder.h"
#include "Entities/EntityManager.h"

namespace Zongine {
    // ============================================
    // AnimStateMachineBuilder Implementation
    // ============================================

    AnimStateMachineBuilder::AnimStateMachineBuilder(Entity& entity)
        : m_Entity(entity) {
    }

    AnimStateMachineBuilder& AnimStateMachineBuilder::AddState(
        const std::string& name, const std::string& animPath, bool loop, float speed) {
        
        AnimState state;
        state.Name = name;
        state.AnimationPath = animPath;
        state.Loop = loop;
        state.Speed = speed;
        
        m_States.States[name] = state;
        
        // If first state, set as default
        if (m_States.DefaultState.empty()) {
            m_States.DefaultState = name;
        }
        
        return *this;
    }

    AnimStateMachineBuilder& AnimStateMachineBuilder::AddTransition(
        const std::string& from, const std::string& to, float duration) {
        
        AnimTransition transition;
        transition.FromState = from;
        transition.ToState = to;
        transition.TransitionDuration = duration;
        
        m_Transitions.Transitions.push_back(transition);
        
        return *this;
    }

    AnimStateMachineBuilder& AnimStateMachineBuilder::AddCondition(
        const std::string& from, const std::string& to,
        const std::string& paramName, AnimConditionType type, float value) {
        
        auto* transition = FindTransition(from, to);
        if (transition) {
            AnimCondition condition;
            condition.ParameterName = paramName;
            condition.Type = type;
            condition.FloatValue = value;
            transition->Conditions.push_back(condition);
        }
        
        return *this;
    }

    AnimStateMachineBuilder& AnimStateMachineBuilder::AddCondition(
        const std::string& from, const std::string& to,
        const std::string& paramName, AnimConditionType type, int value) {
        
        auto* transition = FindTransition(from, to);
        if (transition) {
            AnimCondition condition;
            condition.ParameterName = paramName;
            condition.Type = type;
            condition.IntValue = value;
            transition->Conditions.push_back(condition);
        }
        
        return *this;
    }

    AnimStateMachineBuilder& AnimStateMachineBuilder::AddCondition(
        const std::string& from, const std::string& to,
        const std::string& paramName, AnimConditionType type, bool value) {
        
        auto* transition = FindTransition(from, to);
        if (transition) {
            AnimCondition condition;
            condition.ParameterName = paramName;
            condition.Type = type;
            condition.BoolValue = value;
            transition->Conditions.push_back(condition);
        }
        
        return *this;
    }

    AnimStateMachineBuilder& AnimStateMachineBuilder::SetExitTime(
        const std::string& from, const std::string& to, bool hasExitTime, float exitTime) {
        
        auto* transition = FindTransition(from, to);
        if (transition) {
            transition->HasExitTime = hasExitTime;
            transition->ExitTime = exitTime;
        }
        
        return *this;
    }

    AnimStateMachineBuilder& AnimStateMachineBuilder::SetDefaultState(const std::string& state) {
        m_States.DefaultState = state;
        return *this;
    }

    void AnimStateMachineBuilder::Build() {
        // Add or update components
        if (m_Entity.HasComponent<AnimStateCollectionComponent>()) {
            m_Entity.GetComponent<AnimStateCollectionComponent>() = m_States;
        } else {
            m_Entity.AddComponent(m_States);
        }

        if (m_Entity.HasComponent<AnimTransitionCollectionComponent>()) {
            m_Entity.GetComponent<AnimTransitionCollectionComponent>() = m_Transitions;
        } else {
            m_Entity.AddComponent(m_Transitions);
        }

        // Ensure parameter component exists
        if (!m_Entity.HasComponent<AnimParameterCollectionComponent>()) {
            m_Entity.AddComponent(AnimParameterCollectionComponent{});
        }

        // Ensure runtime state component exists
        if (!m_Entity.HasComponent<AnimStateMachineRuntimeComponent>()) {
            AnimStateMachineRuntimeComponent runtime;
            runtime.CurrentState = m_States.DefaultState;
            m_Entity.AddComponent(runtime);
        }
    }

    AnimTransition* AnimStateMachineBuilder::FindTransition(const std::string& from, const std::string& to) {
        for (auto& transition : m_Transitions.Transitions) {
            if (transition.FromState == from && transition.ToState == to) {
                return &transition;
            }
        }
        return nullptr;
    }

    // ============================================
    // AnimParameterHelper Implementation
    // ============================================

    void AnimParameterHelper::SetFloat(Entity& entity, const std::string& name, float value) {
        auto& param = GetOrCreateParameter(entity, name, AnimParameterType::Float);
        param.FloatValue = value;
    }

    void AnimParameterHelper::SetInt(Entity& entity, const std::string& name, int value) {
        auto& param = GetOrCreateParameter(entity, name, AnimParameterType::Int);
        param.IntValue = value;
    }

    void AnimParameterHelper::SetBool(Entity& entity, const std::string& name, bool value) {
        auto& param = GetOrCreateParameter(entity, name, AnimParameterType::Bool);
        param.BoolValue = value;
    }

    void AnimParameterHelper::SetTrigger(Entity& entity, const std::string& name) {
        auto& param = GetOrCreateParameter(entity, name, AnimParameterType::Trigger);
        param.BoolValue = true;
    }

    void AnimParameterHelper::ResetTrigger(Entity& entity, const std::string& name) {
        if (!entity.HasComponent<AnimParameterCollectionComponent>()) return;

        auto& params = entity.GetComponent<AnimParameterCollectionComponent>();
        auto it = params.Parameters.find(name);
        if (it != params.Parameters.end() && it->second.Type == AnimParameterType::Trigger) {
            it->second.BoolValue = false;
        }
    }

    float AnimParameterHelper::GetFloat(const Entity& entity, const std::string& name, float defaultValue) {
        if (!entity.HasComponent<AnimParameterCollectionComponent>()) return defaultValue;

        const auto& params = entity.GetComponent<AnimParameterCollectionComponent>();
        auto it = params.Parameters.find(name);
        if (it != params.Parameters.end() && it->second.Type == AnimParameterType::Float) {
            return it->second.FloatValue;
        }
        return defaultValue;
    }

    int AnimParameterHelper::GetInt(const Entity& entity, const std::string& name, int defaultValue) {
        if (!entity.HasComponent<AnimParameterCollectionComponent>()) return defaultValue;

        const auto& params = entity.GetComponent<AnimParameterCollectionComponent>();
        auto it = params.Parameters.find(name);
        if (it != params.Parameters.end() && it->second.Type == AnimParameterType::Int) {
            return it->second.IntValue;
        }
        return defaultValue;
    }

    bool AnimParameterHelper::GetBool(const Entity& entity, const std::string& name, bool defaultValue) {
        if (!entity.HasComponent<AnimParameterCollectionComponent>()) return defaultValue;

        const auto& params = entity.GetComponent<AnimParameterCollectionComponent>();
        auto it = params.Parameters.find(name);
        if (it != params.Parameters.end() && 
            (it->second.Type == AnimParameterType::Bool || it->second.Type == AnimParameterType::Trigger)) {
            return it->second.BoolValue;
        }
        return defaultValue;
    }

    AnimParameter& AnimParameterHelper::GetOrCreateParameter(
        Entity& entity, const std::string& name, AnimParameterType type) {
        
        if (!entity.HasComponent<AnimParameterCollectionComponent>()) {
            entity.AddComponent(AnimParameterCollectionComponent{});
        }

        auto& params = entity.GetComponent<AnimParameterCollectionComponent>();
        auto it = params.Parameters.find(name);
        
        if (it == params.Parameters.end()) {
            AnimParameter param;
            param.Name = name;
            param.Type = type;
            param.FloatValue = 0.0f;
            params.Parameters[name] = param;
            return params.Parameters[name];
        }
        
        return it->second;
    }
}
