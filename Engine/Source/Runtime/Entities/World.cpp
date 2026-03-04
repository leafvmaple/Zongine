#include "World.h"

#include "../Components/NameComponent.h"
#include "../Components/HierarchyComponent.h"

namespace Zongine {
    World::World() {
        m_AliveEntities.insert(0);
        Assign<NameComponent>(0, NameComponent{ "Root" });
        Assign<HierarchyComponent>(0, HierarchyComponent{});
    }

    EntityID World::CreateEntity(const std::string& name) {
        EntityID id = ++m_NextEntityID;
        m_AliveEntities.insert(id);
        Assign<NameComponent>(id, NameComponent{ name });
        return id;
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
