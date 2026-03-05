#pragma once

#include <QTreeWidget>

#include <string>
#include <array>
#include <variant>
#include <cstdint>

namespace Zongine {
    class IEditorBridge;

    class ComponentWidget : public QTreeWidget {
        Q_OBJECT

    public:
        explicit ComponentWidget(IEditorBridge& bridge, QWidget* parent = nullptr);
        ~ComponentWidget() = default;

        void UpdateComponents(uint64_t id);

    private:
        using PropertyValue = std::variant<
            float, int, uint32_t, bool, std::string,
            std::array<float, 3>, std::array<float, 4>
        >;

        void AddVectorProperty(const std::array<float, 3>& vec, QTreeWidgetItem* parent,
                               uint64_t entityId, const std::string& componentName,
                               const std::string& propertyName, bool readOnly);
        void AddNumberProperty(float value, QTreeWidgetItem* parent,
                               uint64_t entityId, const std::string& componentName,
                               const std::string& propertyName, bool readOnly);
        void AddNumberProperty(int value, QTreeWidgetItem* parent, bool readOnly);
        void AddNumberProperty(uint32_t value, QTreeWidgetItem* parent, bool readOnly);
        void AddBoolProperty(bool value, QTreeWidgetItem* parent,
                             uint64_t entityId, const std::string& componentName,
                             const std::string& propertyName, bool readOnly);
        void AddStringProperty(const std::string& value, QTreeWidgetItem* parent, bool readOnly);

        IEditorBridge& m_Bridge;
    };
}