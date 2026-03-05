#pragma once

#include "../Include/Types.h"
#include "../Managers/Manager.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <typeindex>
#include <cassert>
#include <tuple>
#include <string>

namespace Zongine {
    // Forward declarations
    struct NameComponent;
    struct HierarchyComponent;
    template<typename... Components> class View;

    // =========================================================================
    // ComponentStorage -- contiguous packed arrays with O(1) entity lookup
    //
    // Data layout:
    //   m_Data:          [ {entityA, compA}, {entityB, compB}, ... ]  (packed)
    //   m_EntityToIndex: { entityA->0, entityB->1, ... }               (sparse)
    //
    // Iteration is cache-friendly (sequential vector traversal).
    // Lookup by EntityID is O(1) amortized via hash map.
    // =========================================================================
    class IComponentStorage {
    public:
        virtual ~IComponentStorage() = default;
        virtual void RemoveEntity(EntityID entity) = 0;
    };

    template<typename T>
    class ComponentStorage : public IComponentStorage {
    private:
        std::vector<std::pair<EntityID, T>> m_Data;
        std::unordered_map<EntityID, uint32_t> m_EntityToIndex;

    public:
        T& AddComponent(EntityID entity, const T& component) {
            auto it = m_EntityToIndex.find(entity);
            if (it != m_EntityToIndex.end()) {
                m_Data[it->second].second = component;
                return m_Data[it->second].second;
            }
            uint32_t index = static_cast<uint32_t>(m_Data.size());
            m_Data.emplace_back(entity, component);
            m_EntityToIndex[entity] = index;
            return m_Data.back().second;
        }

        T& GetComponent(EntityID entity) {
            auto it = m_EntityToIndex.find(entity);
            assert(it != m_EntityToIndex.end() && "Entity does not have this component.");
            return m_Data[it->second].second;
        }

        const T& GetComponent(EntityID entity) const {
            auto it = m_EntityToIndex.find(entity);
            assert(it != m_EntityToIndex.end() && "Entity does not have this component.");
            return m_Data[it->second].second;
        }

        bool HasComponent(EntityID entity) const {
            return m_EntityToIndex.find(entity) != m_EntityToIndex.end();
        }

        std::vector<std::pair<EntityID, T>>& GetComponents() { return m_Data; }
        const std::vector<std::pair<EntityID, T>>& GetComponents() const { return m_Data; }

        void RemoveComponent(EntityID entity) {
            auto it = m_EntityToIndex.find(entity);
            if (it == m_EntityToIndex.end()) return;

            uint32_t index = it->second;
            uint32_t lastIndex = static_cast<uint32_t>(m_Data.size()) - 1;

            if (index != lastIndex) {
                EntityID lastEntity = m_Data[lastIndex].first;
                m_Data[index] = std::move(m_Data[lastIndex]);
                m_EntityToIndex[lastEntity] = index;
            }

            m_Data.pop_back();
            m_EntityToIndex.erase(it);
        }

        void RemoveEntity(EntityID entity) override {
            RemoveComponent(entity);
        }

        size_t Size() const { return m_Data.size(); }
        bool Empty() const { return m_Data.empty(); }
    };

    // =========================================================================
    // World -- Central ECS Registry
    //
    // Entity is just an ID (EntityID = uint32_t). All data lives in
    // ComponentStorage pools owned by World. Systems query data through
    // World's API: Assign/Get/Has for single components, View<> or ForEach
    // for batch iteration.
    //
    // Hierarchy (parent/child) is a plain HierarchyComponent, not baked
    // into the entity type.
    // =========================================================================
    class World : public SingleManager<World> {
    public:
        World();

        // === Entity lifecycle ===
        EntityID CreateEntity(const std::string& name = "");
        void DestroyEntity(EntityID entity);
        bool IsAlive(EntityID entity) const;
        EntityID GetRootEntity() const { return 0; }

        // === Component operations ===
        template<typename T>
        T& Assign(EntityID entity, const T& component = T{}) {
            return GetOrCreatePool<T>().AddComponent(entity, component);
        }

        template<typename T>
        T& Get(EntityID entity) {
            return GetPool<T>().GetComponent(entity);
        }

        template<typename T>
        const T& Get(EntityID entity) const {
            return GetPoolConst<T>().GetComponent(entity);
        }

        template<typename T>
        bool Has(EntityID entity) const {
            std::type_index idx(typeid(T));
            auto it = m_Storages.find(idx);
            if (it == m_Storages.end()) return false;
            return static_cast<const ComponentStorage<T>&>(*it->second).HasComponent(entity);
        }

        template<typename T>
        void Remove(EntityID entity) {
            std::type_index idx(typeid(T));
            auto it = m_Storages.find(idx);
            if (it != m_Storages.end()) {
                static_cast<ComponentStorage<T>&>(*it->second).RemoveComponent(entity);
            }
        }

        // === Multi-component View ===
        template<typename... Components>
        View<Components...> CreateView() {
            return View<Components...>(*this);
        }

        // === Single-component iteration (backwards compat) ===
        template<typename T, typename Func>
        void ForEach(Func&& func) {
            auto& pool = GetOrCreatePool<T>();
            for (auto& [entityID, component] : pool.GetComponents()) {
                func(entityID, component);
            }
        }

        template<typename T>
        std::vector<std::pair<EntityID, T>>& GetComponents() {
            return GetOrCreatePool<T>().GetComponents();
        }

        // === Hierarchy ===
        EntityID AddChild(EntityID parent, const std::string& name = "");
        const std::vector<EntityID>& GetChildren(EntityID entity) const;
        EntityID GetParent(EntityID entity) const;

        // === Utility ===
        std::string GetName(EntityID entity) const;

    private:
        EntityID m_NextEntityID{0};
        std::vector<EntityID> m_FreeList;
        std::unordered_set<EntityID> m_AliveEntities;
        std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> m_Storages;

        template<typename T>
        ComponentStorage<T>& GetOrCreatePool() {
            std::type_index idx(typeid(T));
            if (m_Storages.find(idx) == m_Storages.end()) {
                m_Storages[idx] = std::make_unique<ComponentStorage<T>>();
            }
            return static_cast<ComponentStorage<T>&>(*m_Storages[idx]);
        }

        template<typename T>
        ComponentStorage<T>& GetPool() {
            std::type_index idx(typeid(T));
            auto it = m_Storages.find(idx);
            assert(it != m_Storages.end() && "Component pool not found. Did you Assign this type?");
            return static_cast<ComponentStorage<T>&>(*it->second);
        }

        template<typename T>
        const ComponentStorage<T>& GetPoolConst() const {
            std::type_index idx(typeid(T));
            auto it = m_Storages.find(idx);
            assert(it != m_Storages.end() && "Component pool not found.");
            return static_cast<const ComponentStorage<T>&>(*it->second);
        }
    };

    // =========================================================================
    // Exclude -- tag type for View exclusion filters
    //
    // Usage:
    //   view.Each(Exclude<NvFlexComponent>{}, [](EntityID id, A& a, B& b) { ... });
    // =========================================================================
    template<typename... Ts>
    struct Exclude {};

    // =========================================================================
    // View -- Multi-component query
    //
    // Usage:
    //   world.CreateView<TransformComponent, MeshComponent>()
    //       .Each([](EntityID id, TransformComponent& t, MeshComponent& m) {
    //           // iterate all entities that have BOTH components
    //       });
    //
    //   // With exclusion:
    //   world.CreateView<TransformComponent, MeshComponent>()
    //       .Each(Exclude<NvFlexComponent>{}, [](EntityID id, TransformComponent& t, MeshComponent& m) {
    //           // iterate entities with Transform+Mesh but WITHOUT NvFlex
    //       });
    //
    // Iterates the smallest component pool and filters by the rest.
    // =========================================================================
    template<typename... Components>
    class View {
        using First = std::tuple_element_t<0, std::tuple<Components...>>;

    public:
        explicit View(World& world) : m_World(world) {}

        template<typename Func>
        void Each(Func&& func) {
            auto& data = m_World.GetComponents<First>();
            for (auto& [entityID, _] : data) {
                if ((m_World.Has<Components>(entityID) && ...)) {
                    func(entityID, m_World.Get<Components>(entityID)...);
                }
            }
        }

        template<typename... Excluded, typename Func>
        void Each(Exclude<Excluded...>, Func&& func) {
            auto& data = m_World.GetComponents<First>();
            for (auto& [entityID, _] : data) {
                if ((m_World.Has<Components>(entityID) && ...) &&
                    (!m_World.Has<Excluded>(entityID) && ...)) {
                    func(entityID, m_World.Get<Components>(entityID)...);
                }
            }
        }

    private:
        World& m_World;
    };
}
