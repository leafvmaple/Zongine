#include "Editor.h"
#include <QtWidgets/QApplication>
#include <QVBoxLayout>

#include "Widgets/RenderWidget.h"

using namespace Zongine;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWidget mainWindow;

    app.setApplicationName("Zongine Editor");

    QVBoxLayout* layout = new QVBoxLayout(&mainWindow);

    RenderWidget* renderWidget = new RenderWidget();
    layout->addWidget(renderWidget);

    mainWindow.resize(800, 600);
    mainWindow.show();

    return app.exec();
}
