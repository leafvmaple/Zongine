#include "EditorBridgeImpl.h"

#include <windows.h>

#include "../Entities/World.h"
#include "../Include/ComponentReflection.h"
#include "EventManager.h"

// Component headers only needed for dynamic-array special cases
#include "../Components/ShaderComponent.h"
#include "../Components/FlexibleComponent.h"

namespace Zongine {

    // =========================================================================
    // SetPropertyCommand -- used internally by SetPropertyWithUndo
    // =========================================================================
    class SetPropertyCommand : public ICommand {
    public:
        SetPropertyCommand(EditorBridgeImpl& bridge, EntityID entity,
                           std::string comp, std::string prop,
                           PropertyValue oldVal, PropertyValue newVal)
            : m_Bridge(bridge), m_Entity(entity)
            , m_Component(std::move(comp)), m_Property(std::move(prop))
            , m_OldValue(std::move(oldVal)), m_NewValue(std::move(newVal))
        {}

        void Execute() override {
            m_Bridge.SetProperty(m_Entity, m_Component, m_Property, m_NewValue);
        }

        void Undo() override {
            m_Bridge.SetProperty(m_Entity, m_Component, m_Property, m_OldValue);
        }

        std::string GetDescription() const override {
            return "Set " + m_Component + "." + m_Property;
        }

    private:
        EditorBridgeImpl& m_Bridge;
        EntityID m_Entity;
        std::string m_Component;
        std::string m_Property;
        PropertyValue m_OldValue;
        PropertyValue m_NewValue;
    };

    // =========================================================================
    // Constructor
    // =========================================================================

    EditorBridgeImpl::EditorBridgeImpl(std::recursive_mutex& tickMutex)
        : m_TickMutex(tickMutex)
    {
    }

    // =========================================================================
    // Scene hierarchy
    // =========================================================================

    EntityInfo EditorBridgeImpl::GetEntityInfo(EntityID id) const {
        std::lock_guard<std::recursive_mutex> lock(m_TickMutex);
        auto& world = World::GetInstance();
        EntityInfo info;
        info.ID = id;
        info.Name = world.GetName(id);
        info.Children = world.GetChildren(id);
        return info;
    }

    std::vector<EntityInfo> EditorBridgeImpl::GetEntityTree() const {
        std::lock_guard<std::recursive_mutex> lock(m_TickMutex);
        auto& world = World::GetInstance();
        EntityID root = world.GetRootEntity();
        auto& rootChildren = world.GetChildren(root);

        std::vector<EntityInfo> tree;
        for (auto childId : rootChildren) {
            BuildEntityTree(childId, tree);
        }
        return tree;
    }

    EntityID EditorBridgeImpl::GetRootEntity() const {
        std::lock_guard<std::recursive_mutex> lock(m_TickMutex);
        return World::GetInstance().GetRootEntity();
    }

    void EditorBridgeImpl::BuildEntityTree(EntityID id, std::vector<EntityInfo>& out) const {
        out.push_back(GetEntityInfo(id));
        auto& world = World::GetInstance();
        for (auto childId : world.GetChildren(id)) {
            BuildEntityTree(childId, out);
        }
    }

    // =========================================================================
    // Component inspection -- reflection-driven
    //
    // Iterates the ComponentReflectionRegistry to build ComponentInfo lists.
    // Components with dynamic array properties (Shader, Flexible) are
    // special-cased here because their property count varies per entity.
    // =========================================================================

    std::vector<ComponentInfo> EditorBridgeImpl::GetComponents(EntityID id) const {
        std::lock_guard<std::recursive_mutex> lock(m_TickMutex);
        auto& world = World::GetInstance();
        auto& registry = ComponentReflectionRegistry::Instance();
        std::vector<ComponentInfo> components;

        for (auto& desc : registry.GetAll()) {
            if (!desc.HasComponent(world, id)) continue;

            const void* comp = desc.GetComponentConst(world, id);
            ComponentInfo ci;
            ci.Name = desc.Name;

            if (!desc.Properties.empty()) {
                // Standard reflection path -- static properties
                for (auto& prop : desc.Properties) {
                    ci.Properties.push_back({ prop.Name, prop.Getter(comp), prop.ReadOnly });
                }
            } else if (desc.Name == "Shader") {
                // Dynamic array: Shader paths
                auto& s = *static_cast<const ShaderComponent*>(comp);
                for (size_t i = 0; i < s.Paths.size(); i++) {
                    ci.Properties.push_back({ "Path[" + std::to_string(i) + "]", s.Paths[i], true });
                }
            } else if (desc.Name == "Flexible") {
                // Dynamic array: Flexible drivers
                auto& f = *static_cast<const FlexibleComponent*>(comp);
                for (size_t i = 0; i < f.Drivers.size(); i++) {
                    ci.Properties.push_back({
                        "Driver[" + std::to_string(i) + "].index",
                        f.Drivers[i].index,
                        true
                    });
                }
            }

            components.push_back(std::move(ci));
        }

        return components;
    }

    // =========================================================================
    // Component editing -- reflection-driven write-back
    // =========================================================================

    bool EditorBridgeImpl::SetProperty(EntityID id, const std::string& componentName,
                                       const std::string& propertyName, const PropertyValue& value) {
        std::lock_guard<std::recursive_mutex> lock(m_TickMutex);
        auto& world = World::GetInstance();
        auto& registry = ComponentReflectionRegistry::Instance();

        for (auto& desc : registry.GetAll()) {
            if (desc.Name != componentName) continue;
            if (!desc.HasComponent(world, id)) return false;

            void* comp = desc.GetComponent(world, id);
            for (auto& prop : desc.Properties) {
                if (prop.Name == propertyName && !prop.ReadOnly && prop.Setter) {
                    return prop.Setter(comp, value);
                }
            }
        }

        return false;
    }

    // =========================================================================
    // Property read -- reflection-driven (for undo snapshots)
    // =========================================================================

    PropertyValue EditorBridgeImpl::GetProperty(EntityID id, const std::string& componentName,
                                                const std::string& propertyName) const {
        std::lock_guard<std::recursive_mutex> lock(m_TickMutex);
        auto& world = World::GetInstance();
        auto& registry = ComponentReflectionRegistry::Instance();

        for (auto& desc : registry.GetAll()) {
            if (desc.Name != componentName) continue;
            if (!desc.HasComponent(world, id)) break;

            const void* comp = desc.GetComponentConst(world, id);
            for (auto& prop : desc.Properties) {
                if (prop.Name == propertyName) {
                    return prop.Getter(comp);
                }
            }
        }

        // Fallback for unknown properties
        return std::string{};
    }

    // =========================================================================
    // Undo / Redo
    // =========================================================================

    bool EditorBridgeImpl::SetPropertyWithUndo(EntityID id, const std::string& componentName,
                                               const std::string& propertyName, const PropertyValue& value) {
        std::lock_guard<std::recursive_mutex> lock(m_TickMutex);

        PropertyValue oldVal = GetProperty(id, componentName, propertyName);
        auto cmd = std::make_unique<SetPropertyCommand>(
            *this, id, componentName, propertyName, std::move(oldVal), value);
        m_CommandHistory.Execute(std::move(cmd));

        Emit("COMPONENT_UPDATE");
        return true;
    }

    bool EditorBridgeImpl::Undo() {
        std::lock_guard<std::recursive_mutex> lock(m_TickMutex);
        bool ok = m_CommandHistory.Undo();
        if (ok) Emit("COMPONENT_UPDATE");
        return ok;
    }

    bool EditorBridgeImpl::Redo() {
        std::lock_guard<std::recursive_mutex> lock(m_TickMutex);
        bool ok = m_CommandHistory.Redo();
        if (ok) Emit("COMPONENT_UPDATE");
        return ok;
    }

    bool EditorBridgeImpl::CanUndo() const { return m_CommandHistory.CanUndo(); }
    bool EditorBridgeImpl::CanRedo() const { return m_CommandHistory.CanRedo(); }

    // =========================================================================
    // Events -- delegate to EventManager
    // =========================================================================

    void EditorBridgeImpl::Subscribe(const std::string& event, std::function<void()> callback) {
        if (EventManager::IsRegistered()) {
            EventManager::GetInstance().Subscribe(event, std::move(callback));
        }
    }

    void EditorBridgeImpl::Emit(const std::string& event) {
        if (EventManager::IsRegistered()) {
            EventManager::GetInstance().Emit(event);
        }
    }
}
