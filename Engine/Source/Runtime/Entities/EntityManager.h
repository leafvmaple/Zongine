#pragma once

#include "Entities/Entity.h"

#include <unordered_map>
#include <memory>
#include <typeindex>
#include <cassert>

namespace Zongine {
    class IComponentStorage {
    public:
        virtual ~IComponentStorage() = default;
    };

    template<typename ComponentType>
    class ComponentStorage : public IComponentStorage {
    private:
        std::unordered_map<EntityID, ComponentType> components; // 存储实体 ID 和组件

    public:
        // TODO std::move
        ComponentType& AddComponent(EntityID entity, const ComponentType& component) {
            components[entity] = component;
            return components[entity];
        }

        ComponentType& GetComponent(EntityID entity) {
            assert(components.find(entity) != components.end() && "Entity does not have this component.");
            return components[entity];
        }

        const ComponentType& GetComponent(EntityID entity) const {
            assert(components.find(entity) != components.end() && "Entity does not have this component.");
            return components.at(entity);
        }

        // 检查实体是否有该组件
        bool HasComponent(EntityID entity) const {
            return components.find(entity) != components.end();
        }

        const std::unordered_map<EntityID, ComponentType>& GetComponents() const {
            return components;
        }
    };

    class EntityManager {
    private:
        EntityID m_NextEntityID{};
        std::unordered_map<EntityID, Entity> m_Entities{};
        std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> componentStorages{};

    public:
        Entity& CreateEntity() {
            EntityID id = ++m_NextEntityID;
			return m_Entities.emplace(id, Entity(id, this)).first->second;
        }

        const std::unordered_map<EntityID, Entity>& GetEntities() {
            return m_Entities;
        }

        template<typename ComponentType>
        const std::unordered_map<EntityID, ComponentType>& GetEntities() {
            const auto& storage = GetOrCreateStorage<ComponentType>();
            return storage.GetComponents();
        }

        Entity& GetEntity(EntityID entity) {
            assert(m_Entities.find(entity) != m_Entities.end() && "Entity not found.");
            return m_Entities.at(entity);
        }

        template<typename ComponentType>
        ComponentType& AddComponent(EntityID entity, const ComponentType& component) {
            auto& storage = GetOrCreateStorage<ComponentType>();
            return storage.AddComponent(entity, component);
        }

        template<typename ComponentType>
        ComponentType& GetComponent(EntityID entity) {
            auto& storage = GetOrCreateStorage<ComponentType>();
            return storage.GetComponent(entity);
        }

        template<typename ComponentType>
        const ComponentType& GetComponent(EntityID entity) const {
            const auto& storage = GetOrCreateStorage<ComponentType>();
            return storage.GetComponent(entity);
        }

        template<typename ComponentType>
        bool HasComponent(EntityID entity) const {
            const auto* storage = GetOrCreateStorage<ComponentType>();
            return storage && storage->HasComponent(entity);
        }

    private:
        template<typename ComponentType>
        ComponentStorage<ComponentType>& GetOrCreateStorage() {
            std::type_index typeIndex(typeid(ComponentType));
            if (componentStorages.find(typeIndex) == componentStorages.end()) {
                componentStorages[typeIndex] = std::make_unique<ComponentStorage<ComponentType>>();
            }
            return static_cast<ComponentStorage<ComponentType>&>(*componentStorages[typeIndex]);
        }

        template<typename ComponentType>
        ComponentStorage<ComponentType>& GetStorage() const {
            std::type_index typeIndex(typeid(ComponentType));
            auto it = componentStorages.find(typeIndex);
            assert(it != componentStorages.end() && "Component type not found.");
            return static_cast<ComponentStorage<ComponentType>&>(*it->second);
        }
    };
}