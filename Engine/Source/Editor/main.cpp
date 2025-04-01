#include <QtWidgets/QApplication>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTreeWidget>

#include "Widgets/RenderWidget.h"
#include "Widgets/EntitiesWidget.h"
#include "Widgets/ComponentsWidget.h"

#include "Runtime/Engine.h"

using namespace Zongine;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Zongine Editor");

    auto engine = std::make_shared<Engine>();

    QSplitter splitter(Qt::Horizontal);
    splitter.setGeometry(QRect(0, 0, 1480, 720));

    RenderWidget* renderWidget = new RenderWidget(engine);
    EntitiesWidget* entityTree = new EntitiesWidget(engine);
    ComponentWidget* componentWidget = new ComponentWidget(engine);

    QObject::connect(entityTree, &QTreeWidget::itemClicked, [=](QTreeWidgetItem* item, int column) {
        uint64_t id = item->data(0, Qt::UserRole).value<uint64_t>();
        componentWidget->UpdateComponents(id);
    });

    splitter.addWidget(entityTree);
    splitter.addWidget(renderWidget);
    splitter.addWidget(componentWidget);
    splitter.setSizes({ 200, 800, 400 });

    splitter.show();

    return app.exec();
}