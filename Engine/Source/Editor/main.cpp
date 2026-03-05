#include <QtWidgets/QApplication>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTreeWidget>

#include "Widgets/RenderWidget.h"
#include "Widgets/EntitiesWidget.h"
#include "Widgets/ComponentsWidget.h"
#include "Widgets/QtEventBridge.h"

#include "Runtime/Engine.h"
#include "Runtime/Include/IEditorBridge.h"

using namespace Zongine;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Zongine Editor");

    auto engine = std::make_shared<Engine>();
    engine->SetEditorMode(true);

    QSplitter splitter(Qt::Horizontal);
    splitter.setGeometry(QRect(0, 0, 1480, 720));

    // RenderWidget still needs Engine for Initialize/Tick
    RenderWidget* renderWidget = new RenderWidget(engine);

    // After Initialize, get the decoupled bridge interface
    IEditorBridge& bridge = engine->GetEditorBridge();

    // QtEventBridge converts engine events → Qt signals
    QtEventBridge* eventBridge = new QtEventBridge(&splitter);
    eventBridge->ConnectToEngine(bridge);

    // Widgets now depend only on IEditorBridge, not Engine/World/Components
    EntitiesWidget* entityTree = new EntitiesWidget(bridge, *eventBridge);
    ComponentWidget* componentWidget = new ComponentWidget(bridge);

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