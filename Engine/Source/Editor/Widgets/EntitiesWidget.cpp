#include "EntitiesWidget.h"

#include "Runtime/Entities/EntityManager.h"

namespace Zongine {
    EntitiesWidget::EntitiesWidget(std::shared_ptr<Engine> engine, QWidget* parent /*= nullptr*/) {
        setColumnCount(1);
        setHeaderLabels({ "Entities" });

        engine->SubscribeEvent("ENTITY_UPDATE", [this] {
            this->UpdateEntity();
        });

        m_Engine = engine;

        UpdateEntity();
    }

    void EntitiesWidget::UpdateEntity() {
        clear();
        auto& rootEntity = m_Engine->GetRootEntity();
        auto& children = rootEntity.GetChildren();
        for (auto& id : children) {
            // auto& entity = m_Engine->GetEntity(id);
            auto item = new QTreeWidgetItem(this);
            item->setText(0, QString::number(id));
            addTopLevelItem(item);
        }
    }
}