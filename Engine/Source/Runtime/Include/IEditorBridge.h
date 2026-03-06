#pragma once

#include "Types.h"

#include <string>
#include <vector>
#include <variant>
#include <array>
#include <functional>

namespace Zongine {
    // =========================================================================
    // PropertyValue -- type-erased component property for Editor display/edit
    //
    // Editor UI never needs to know about TransformComponent, MeshComponent, etc.
    // It just deals with float/int/string/vector values.
    // =========================================================================
    using PropertyValue = std::variant<
        float,
        int,
        uint32_t,
        bool,
        std::string,
        std::array<float, 3>,
        std::array<float, 4>
    >;

    struct PropertyInfo {
        std::string Name;
        PropertyValue Value;
        bool ReadOnly = false;
    };

    struct ComponentInfo {
        std::string Name;                       // e.g. "Transform", "Mesh"
        std::vector<PropertyInfo> Properties;
    };

    struct EntityInfo {
        EntityID ID;
        std::string Name;
        std::vector<EntityID> Children;
    };

    // =========================================================================
    // IEditorBridge -- The ONLY interface the Editor should depend on
    //
    // This decouples the Qt Editor from concrete ECS types. The Editor never
    // needs to #include any Component header or access World directly.
    // =========================================================================
    class IEditorBridge {
    public:
        virtual ~IEditorBridge() = default;

        // === Scene hierarchy ===
        virtual EntityInfo GetEntityInfo(EntityID id) const = 0;
        virtual std::vector<EntityInfo> GetEntityTree() const = 0;
        virtual EntityID GetRootEntity() const = 0;

        // === Component inspection ===
        virtual std::vector<ComponentInfo> GetComponents(EntityID id) const = 0;

        // === Component editing (write-back to ECS) ===
        virtual bool SetProperty(EntityID id, const std::string& componentName,
                                 const std::string& propertyName, const PropertyValue& value) = 0;

        // === Property read (for undo -- get current value before overwrite) ===
        virtual PropertyValue GetProperty(EntityID id, const std::string& componentName,
                                          const std::string& propertyName) const = 0;

        // === Undo / Redo ===
        virtual bool SetPropertyWithUndo(EntityID id, const std::string& componentName,
                                         const std::string& propertyName, const PropertyValue& value) = 0;
        virtual bool Undo() = 0;
        virtual bool Redo() = 0;
        virtual bool CanUndo() const = 0;
        virtual bool CanRedo() const = 0;

        // === Event subscription ===
        virtual void Subscribe(const std::string& event, std::function<void()> callback) = 0;
        virtual void Emit(const std::string& event) = 0;
    };
}
