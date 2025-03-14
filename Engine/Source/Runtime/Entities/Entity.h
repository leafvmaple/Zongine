#pragma once

#include <cstdint>

namespace Zongine {
    using EntityID = uint32_t;

    class EntityManager;

    class Entity {
    private:
        EntityID m_ID;
        class EntityManager* m_Manager;

    public:
        Entity(EntityID id, EntityManager* manager) : m_ID(id), m_Manager(manager) {}

        template<typename ComponentType>
        const ComponentType& GetComponent() const;

        template<typename ComponentType>
        ComponentType& GetComponent();

        EntityID GetID() const { return m_ID; }
    };
}
