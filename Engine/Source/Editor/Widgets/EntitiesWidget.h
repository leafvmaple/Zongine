#pragma once

#include <QTreeWidget>
#include <memory>

#include "Runtime/Engine.h"

namespace Zongine {
    class EntitiesWidget : public QTreeWidget {
        Q_OBJECT

    public:
        explicit EntitiesWidget(std::shared_ptr<Engine> engine, QWidget* parent = nullptr);
        ~EntitiesWidget() = default;

    private:
        void UpdateEntity();

        std::shared_ptr<Engine> m_Engine{};
    };
}