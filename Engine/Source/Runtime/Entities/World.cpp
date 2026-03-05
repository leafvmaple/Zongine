#include "World.h"

#include "../Components/NameComponent.h"
#include "../Components/HierarchyComponent.h"
#include "../Managers/EventManager.h"

#include <algorithm>

namespace Zongine {
    World::World() {
        m_AliveEntities.insert(0);
        Assign<NameComponent>(0, NameComponent{ "Root" });
        Assign<HierarchyComponent>(0, HierarchyComponent{});
    }

    EntityID World::CreateEntity(const std::string& name) {
        EntityID id;
        if (!m_FreeList.empty()) {
            id = m_FreeList.back();
            m_FreeList.pop_back();
        } else {
            id = ++m_NextEntityID;
        }
        m_AliveEntities.insert(id);
        Assign<NameComponent>(id, NameComponent{ name });
        return id;
    }

    void World::DestroyEntity(EntityID entity) {
        if (entity == 0) return;           // Cannot destroy root
        if (!IsAlive(entity)) return;

        // Recursively destroy children first (copy list -- it will be mutated)
        if (Has<HierarchyComponent>(entity)) {
            auto children = Get<HierarchyComponent>(entity).Children;
            for (EntityID child : children) {
                DestroyEntity(child);
            }
        }

        // Detach from parent
        if (Has<HierarchyComponent>(entity)) {
            EntityID parent = Get<HierarchyComponent>(entity).Parent;
            if (Has<HierarchyComponent>(parent)) {
                auto& siblings = Get<HierarchyComponent>(parent).Children;
                siblings.erase(
                    std::remove(siblings.begin(), siblings.end(), entity),
                    siblings.end()
                );
            }
        }

        // Remove all components from every storage pool
        for (auto& [typeIdx, storage] : m_Storages) {
            storage->RemoveEntity(entity);
        }

        m_AliveEntities.erase(entity);
        m_FreeList.push_back(entity);

        if (EventManager::IsRegistered()) {
            EventManager::GetInstance().Emit("ENTITY_UPDATE");
        }
    }

    bool World::IsAlive(EntityID entity) const {
        return m_AliveEntities.count(entity) > 0;
    }

    EntityID World::AddChild(EntityID parent, const std::string& name) {
        EntityID child = CreateEntity(name);

        if (!Has<HierarchyComponent>(parent))
            Assign<HierarchyComponent>(parent, HierarchyComponent{});

        Get<HierarchyComponent>(parent).Children.push_back(child);

        HierarchyComponent childHierarchy{};
        childHierarchy.Parent = parent;
        Assign<HierarchyComponent>(child, childHierarchy);

        if (EventManager::IsRegistered()) {
            EventManager::GetInstance().Emit("ENTITY_UPDATE");
        }

        return child;
    }

    const std::vector<EntityID>& World::GetChildren(EntityID entity) const {
        if (!Has<HierarchyComponent>(entity)) {
            static const std::vector<EntityID> empty{};
            return empty;
        }
        return Get<HierarchyComponent>(entity).Children;
    }

    EntityID World::GetParent(EntityID entity) const {
        if (!Has<HierarchyComponent>(entity)) return 0;
        return Get<HierarchyComponent>(entity).Parent;
    }

    std::string World::GetName(EntityID entity) const {
        if (!Has<NameComponent>(entity)) return "";
        return Get<NameComponent>(entity).Name;
    }
}
