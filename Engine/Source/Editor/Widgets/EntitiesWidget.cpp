#include "EntitiesWidget.h"

#include "Runtime/Entities/EntityManager.h"

namespace Zongine {
    EntitiesWidget::EntitiesWidget(std::shared_ptr<Engine> engine, QWidget* parent /*= nullptr*/) {
        setColumnCount(2);
        setHeaderLabels({ "ID", "Name" });

        setTreePosition(1);
        setColumnWidth(0, 30);
        setAlternatingRowColors(true);

        engine->SubscribeEvent("ENTITY_UPDATE", [this] {
            this->UpdateEntities();
        });

        m_Engine = engine;

        UpdateEntities();
    }

    void EntitiesWidget::UpdateEntities() {
        clear();
        auto& rootEntity = m_Engine->GetRootEntity();
        auto& children = rootEntity.GetChildren();
        for (auto& id : children) {
            auto item = new QTreeWidgetItem(this);
            UpdateEntity(id, item);
        }
    }

    void EntitiesWidget::UpdateEntity(EntityID id, QTreeWidgetItem* item) {
        auto& entity = m_Engine->GetEntity(id);
        auto name = entity.GetName();
        item->setText(0, QString::number(id));
        item->setText(1, QString::fromStdString(name));
        item->setData(0, Qt::UserRole, QVariant::fromValue(id));

        auto& children = entity.GetChildren();
        for (auto& child : children) {
            auto sub = new QTreeWidgetItem(item);
            UpdateEntity(child, sub);
        }
    }
}