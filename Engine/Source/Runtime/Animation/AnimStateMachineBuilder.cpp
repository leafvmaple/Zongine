#include "AnimStateMachineBuilder.h"
#include "Entities/EntityManager.h"

#include "../LLoader/3rd/rapidjson/include/rapidjson/document.h"
#include "../LLoader/3rd/rapidjson/include/rapidjson/writer.h"
#include "../LLoader/3rd/rapidjson/include/rapidjson/stringbuffer.h"
#include "../LLoader/3rd/rapidjson/include/rapidjson/prettywriter.h"

#include <fstream>
#include <sstream>

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

    // ============================================
    // Serialization/Deserialization Implementation
    // ============================================

    bool AnimStateMachineBuilder::SaveToJson(const std::string& filePath) const {
        using namespace rapidjson;

        Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        // Default state
        doc.AddMember("defaultState", Value(m_States.DefaultState.c_str(), allocator), allocator);

        // States array
        Value statesArray(kArrayType);
        for (const auto& [name, state] : m_States.States) {
            Value stateObj(kObjectType);
            stateObj.AddMember("name", Value(state.Name.c_str(), allocator), allocator);
            stateObj.AddMember("animationPath", Value(state.AnimationPath.c_str(), allocator), allocator);
            stateObj.AddMember("loop", state.Loop, allocator);
            stateObj.AddMember("speed", state.Speed, allocator);
            statesArray.PushBack(stateObj, allocator);
        }
        doc.AddMember("states", statesArray, allocator);

        // Transitions array
        Value transitionsArray(kArrayType);
        for (const auto& transition : m_Transitions.Transitions) {
            Value transitionObj(kObjectType);
            transitionObj.AddMember("from", Value(transition.FromState.c_str(), allocator), allocator);
            transitionObj.AddMember("to", Value(transition.ToState.c_str(), allocator), allocator);
            transitionObj.AddMember("duration", transition.TransitionDuration, allocator);
            transitionObj.AddMember("hasExitTime", transition.HasExitTime, allocator);
            transitionObj.AddMember("exitTime", transition.ExitTime, allocator);

            // Conditions array
            Value conditionsArray(kArrayType);
            for (const auto& condition : transition.Conditions) {
                Value conditionObj(kObjectType);
                conditionObj.AddMember("parameter", Value(condition.ParameterName.c_str(), allocator), allocator);

                // Condition type
                const char* typeStr = "Greater";
                switch (condition.Type) {
                    case AnimConditionType::Greater: typeStr = "Greater"; break;
                    case AnimConditionType::Less: typeStr = "Less"; break;
                    case AnimConditionType::Equal: typeStr = "Equal"; break;
                    case AnimConditionType::NotEqual: typeStr = "NotEqual"; break;
                    case AnimConditionType::GreaterOrEqual: typeStr = "GreaterOrEqual"; break;
                    case AnimConditionType::LessOrEqual: typeStr = "LessOrEqual"; break;
                }
                conditionObj.AddMember("type", Value(typeStr, allocator), allocator);
                conditionObj.AddMember("value", condition.FloatValue, allocator);

                conditionsArray.PushBack(conditionObj, allocator);
            }
            transitionObj.AddMember("conditions", conditionsArray, allocator);

            transitionsArray.PushBack(transitionObj, allocator);
        }
        doc.AddMember("transitions", transitionsArray, allocator);

        // Write to file
        std::ofstream ofs(filePath);
        if (!ofs.is_open()) return false;

        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer);
        doc.Accept(writer);

        ofs << buffer.GetString();
        ofs.close();

        return true;
    }

    bool AnimStateMachineBuilder::LoadFromJson(Entity& entity, const std::string& filePath) {
        using namespace rapidjson;

        // Read file
        std::ifstream ifs(filePath);
        if (!ifs.is_open()) return false;

        std::stringstream buffer;
        buffer << ifs.rdbuf();
        std::string content = buffer.str();
        ifs.close();

        // Parse JSON
        Document doc;
        doc.Parse(content.c_str());

        if (doc.HasParseError()) return false;

        // Create builder
        AnimStateMachineBuilder builder(entity);

        // Load states
        if (doc.HasMember("states") && doc["states"].IsArray()) {
            const auto& statesArray = doc["states"].GetArray();
            for (const auto& stateObj : statesArray) {
                std::string name = stateObj["name"].GetString();
                std::string animPath = stateObj["animationPath"].GetString();
                bool loop = stateObj.HasMember("loop") ? stateObj["loop"].GetBool() : true;
                float speed = stateObj.HasMember("speed") ? stateObj["speed"].GetFloat() : 1.0f;

                builder.AddState(name, animPath, loop, speed);
            }
        }

        // Load default state
        if (doc.HasMember("defaultState")) {
            builder.SetDefaultState(doc["defaultState"].GetString());
        }

        // Load transitions
        if (doc.HasMember("transitions") && doc["transitions"].IsArray()) {
            const auto& transitionsArray = doc["transitions"].GetArray();
            for (const auto& transitionObj : transitionsArray) {
                std::string from = transitionObj["from"].GetString();
                std::string to = transitionObj["to"].GetString();
                float duration = transitionObj.HasMember("duration") ? 
                    transitionObj["duration"].GetFloat() : 0.3f;

                builder.AddTransition(from, to, duration);

                // Load exit time settings
                if (transitionObj.HasMember("hasExitTime")) {
                    bool hasExitTime = transitionObj["hasExitTime"].GetBool();
                    float exitTime = transitionObj.HasMember("exitTime") ? 
                        transitionObj["exitTime"].GetFloat() : 1.0f;
                    builder.SetExitTime(from, to, hasExitTime, exitTime);
                }

                // Load conditions
                if (transitionObj.HasMember("conditions") && transitionObj["conditions"].IsArray()) {
                    const auto& conditionsArray = transitionObj["conditions"].GetArray();
                    for (const auto& conditionObj : conditionsArray) {
                        std::string parameter = conditionObj["parameter"].GetString();
                        std::string typeStr = conditionObj["type"].GetString();
                        float value = conditionObj["value"].GetFloat();

                        // Parse condition type
                        AnimConditionType type = AnimConditionType::Greater;
                        if (typeStr == "Greater") type = AnimConditionType::Greater;
                        else if (typeStr == "Less") type = AnimConditionType::Less;
                        else if (typeStr == "Equal") type = AnimConditionType::Equal;
                        else if (typeStr == "NotEqual") type = AnimConditionType::NotEqual;
                        else if (typeStr == "GreaterOrEqual") type = AnimConditionType::GreaterOrEqual;
                        else if (typeStr == "LessOrEqual") type = AnimConditionType::LessOrEqual;

                        builder.AddCondition(from, to, parameter, type, value);
                    }
                }
            }
        }

        // Build the state machine
        builder.Build();

        return true;
    }
}
