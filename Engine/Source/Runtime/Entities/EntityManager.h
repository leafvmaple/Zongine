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
    };

    class EntityManager {
    private:
        EntityID nextEntityID = 1; // 自动生成的实体 ID
        std::vector<Entity> m_Entities;
        std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> componentStorages;

    public:
        Entity& CreateEntity() {
            EntityID id = nextEntityID++;
            return m_Entities.emplace_back(Entity(id, this));
        }

        const std::vector<Entity>& GetEntities() {
            return m_Entities;
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

        // 获取组件存储
        template<typename ComponentType>
        ComponentStorage<ComponentType>& GetStorage() const {
            std::type_index typeIndex(typeid(ComponentType));
            auto it = componentStorages.find(typeIndex);
            assert(it != componentStorages.end() && "Component type not found.");
            return static_cast<ComponentStorage<ComponentType>&>(*it->second);
        }
    };

    extern EntityManager GEntityManager;
}