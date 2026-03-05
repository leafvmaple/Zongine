#include "EntitiesWidget.h"
#include "QtEventBridge.h"

#include "Runtime/Include/IEditorBridge.h"

namespace Zongine {
    EntitiesWidget::EntitiesWidget(IEditorBridge& bridge, QtEventBridge& eventBridge, QWidget* parent /*= nullptr*/)
        : QTreeWidget(parent), m_Bridge(bridge)
    {
        setColumnCount(2);
        setHeaderLabels({ "ID", "Name" });

        setTreePosition(1);
        setColumnWidth(0, 30);
        setAlternatingRowColors(true);

        // React to engine entity changes via Qt signal (safe for UI thread)
        connect(&eventBridge, &QtEventBridge::entityTreeChanged, this, &EntitiesWidget::UpdateEntities);

        UpdateEntities();
    }

    void EntitiesWidget::UpdateEntities() {
        clear();
        auto rootID = m_Bridge.GetRootEntity();
        auto rootInfo = m_Bridge.GetEntityInfo(rootID);
        for (auto& childId : rootInfo.Children) {
            auto item = new QTreeWidgetItem(this);
            UpdateEntity(childId, item);
        }
    }

    void EntitiesWidget::UpdateEntity(uint32_t id, QTreeWidgetItem* item) {
        auto info = m_Bridge.GetEntityInfo(id);
        item->setText(0, QString::number(info.ID));
        item->setText(1, QString::fromStdString(info.Name));
        item->setData(0, Qt::UserRole, QVariant::fromValue(static_cast<uint64_t>(info.ID)));

        for (auto& childId : info.Children) {
            auto sub = new QTreeWidgetItem(item);
            UpdateEntity(childId, sub);
        }
    }
}