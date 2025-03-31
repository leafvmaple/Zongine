#include <QtWidgets/QApplication>
#include <QVBoxLayout>
#include <QSplitter>

#include "Widgets/RenderWidget.h"
#include "Widgets/EntitiesWidget.h"

#include "Runtime/Engine.h"

using namespace Zongine;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Zongine Editor");

    auto engine = std::make_shared<Engine>();

    RenderWidget* renderWidget = new RenderWidget(engine);
    EntitiesWidget* entityTree = new EntitiesWidget(engine);

    QSplitter splitter(Qt::Horizontal);
    splitter.setGeometry(QRect(0, 0, 1280, 720));

    splitter.addWidget(entityTree);
    splitter.addWidget(renderWidget);

    splitter.setSizes({ 100, 800 });

    splitter.show();

    return app.exec();
}