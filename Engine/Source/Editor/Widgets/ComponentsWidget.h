#pragma once

#include <QTreeWidget> 

namespace Zongine {
    class Engine;
    class Entity;

    class ComponentWidget : public QTreeWidget {
        Q_OBJECT

    public:
        explicit ComponentWidget(std::shared_ptr<Engine> engine, QWidget* parent = nullptr);
        ~ComponentWidget() = default;

        void UpdateComponents(uint64_t id);

    private:
        template<typename T>
        void AddVectorProperty(const T& vec, QTreeWidgetItem* parent);

        void AddPathProperty(std::string path, QTreeWidgetItem* parent);

        std::shared_ptr<Engine> m_Engine{};
    };
}