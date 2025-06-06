#pragma once

#include "Entity.h"

#include <unordered_map>
#include <memory>
#include <typeindex>
#include <cassert>

#include "../Managers/Mananger.h"
#include "../Components/NameComponent.h"

namespace Zongine {
    // TODO Refactor
    class IComponentStorage {
    public:
        virtual ~IComponentStorage() = default;
    };

    template<typename ComponentType>
    class ComponentStorage : public IComponentStorage {
    private:
        std::unordered_map<EntityID, ComponentType> components;

    public:
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

        bool HasComponent(EntityID entity) const {
            return components.find(entity) != components.end();
        }

        std::unordered_map<EntityID, ComponentType>& GetComponents() {
            return components;
        }
    };

    class EntityManager : public SingleManager<EntityManager> {
    private:
        EntityID m_NextEntityID{};
        std::unordered_map<EntityID, Entity> m_Entities{};
        std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> componentStorages{};

    public:
        EntityManager() {
            m_Entities.emplace(0, Entity(0, this)); // Root entity
        }

        Entity& CreateEntity(std::string name) {
            EntityID id = ++m_NextEntityID;
            auto& entity = m_Entities.emplace(id, Entity(id, this)).first->second;
            entity.AddComponent<NameComponent>(NameComponent({ name }));

            return entity;
        }

        Entity& CreateEntity() {
            return CreateEntity("");
        }

        std::unordered_map<EntityID, Entity>& GetEntities() {
            return m_Entities;
        }

        const std::unordered_map<EntityID, Entity>& GetEntities() const {
            return m_Entities;
        }

        template<typename ComponentType>
        std::unordered_map<EntityID, ComponentType>& GetEntities() {
            auto& storage = GetOrCreateStorage<ComponentType>();
            return storage.GetComponents();
        }

        template<typename ComponentType, typename Func>
        void ForEach(Func&& func) {
            auto& storage = GetOrCreateStorage<ComponentType>();
            for (auto& [entityID, component] : storage.GetComponents()) {
                func(entityID, component);
            }
        }

        Entity& GetEntity(EntityID entity) {
            assert(m_Entities.find(entity) != m_Entities.end() && "Entity not found.");
            return m_Entities.at(entity);
        }

        Entity& GetRootEntity() {
            return GetEntity(0);
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

            // return const_cast<ComponentType&>(static_cast<const EntityManager&>(*this).GetComponent(entity));
        }

        template<typename ComponentType>
        const ComponentType& GetComponent(EntityID entity) const {
            const auto& storage = GetOrCreateStorage<ComponentType>();
            return storage.GetComponent(entity);
        }

        template<typename ComponentType>
        bool HasComponent(EntityID entity) {
            const auto& storage = GetOrCreateStorage<ComponentType>();
            return storage.HasComponent(entity);
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