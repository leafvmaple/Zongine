#pragma once

#include <QTreeWidget>

namespace Zongine {
    class IEditorBridge;
    class QtEventBridge;

    class EntitiesWidget : public QTreeWidget {
        Q_OBJECT

    public:
        explicit EntitiesWidget(IEditorBridge& bridge, QtEventBridge& eventBridge, QWidget* parent = nullptr);
        ~EntitiesWidget() = default;

    public slots:
        void UpdateEntities();

    private:
        void UpdateEntity(uint32_t id, QTreeWidgetItem* item);

        IEditorBridge& m_Bridge;
    };
}