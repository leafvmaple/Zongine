#pragma once

#include "../Include/IEditorBridge.h"
#include "CommandHistory.h"

#include <mutex>

namespace Zongine {
    class World;
    class EventManager;

    // =========================================================================
    // EditorBridgeImpl -- Concrete implementation of IEditorBridge
    //
    // Lives in Runtime, has full access to World and all Component types.
    // Translates ECS data into generic PropertyInfo/ComponentInfo structs
    // that the Editor can consume without knowing concrete types.
    // =========================================================================
    class EditorBridgeImpl : public IEditorBridge {
    public:
        explicit EditorBridgeImpl(std::recursive_mutex& tickMutex);
        ~EditorBridgeImpl() override = default;

        // === Scene hierarchy ===
        EntityInfo GetEntityInfo(EntityID id) const override;
        std::vector<EntityInfo> GetEntityTree() const override;
        EntityID GetRootEntity() const override;

        // === Component inspection ===
        std::vector<ComponentInfo> GetComponents(EntityID id) const override;

        // === Component editing ===
        bool SetProperty(EntityID id, const std::string& componentName,
                         const std::string& propertyName, const PropertyValue& value) override;

        // === Property read ===
        PropertyValue GetProperty(EntityID id, const std::string& componentName,
                                  const std::string& propertyName) const override;

        // === Undo / Redo ===
        bool SetPropertyWithUndo(EntityID id, const std::string& componentName,
                                 const std::string& propertyName, const PropertyValue& value) override;
        bool Undo() override;
        bool Redo() override;
        bool CanUndo() const override;
        bool CanRedo() const override;

        // === Events ===
        void Subscribe(const std::string& event, std::function<void()> callback) override;
        void Emit(const std::string& event) override;

    private:
        // Helper: recursively build entity tree
        void BuildEntityTree(EntityID id, std::vector<EntityInfo>& out) const;

        CommandHistory m_CommandHistory;
        std::recursive_mutex& m_TickMutex;
    };
}
