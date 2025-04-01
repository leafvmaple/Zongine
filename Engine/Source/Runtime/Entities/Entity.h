#pragma once

#include <vector>
#include <string>

#include "../Include/Types.h"

namespace Zongine {
    const EntityID INVALID_ENTITY{};

    class EntityManager;

    class Entity {
    private:
        EntityID m_ID;
        class EntityManager* m_Manager{};

        std::vector<EntityID> m_Children{};

    public:
        Entity(EntityID id, EntityManager* manager) : m_ID(id), m_Manager(manager) {}

        template<typename ComponentType>
        const ComponentType& GetComponent() const;

        template<typename ComponentType>
        ComponentType& GetComponent() {
            return const_cast<ComponentType&>(static_cast<const Entity&>(*this).GetComponent<ComponentType>());
        }

        template<typename ComponentType>
        ComponentType& AddComponent(const ComponentType& component);

        template<typename ComponentType>
        bool HasComponent() const;

        EntityID GetID() const { return m_ID; }
        std::string GetName() const;

        Entity& AddChild();
        Entity& AddChild(std::string name);

        const std::vector<EntityID>& GetChildren() const { return m_Children; }
    };
}
