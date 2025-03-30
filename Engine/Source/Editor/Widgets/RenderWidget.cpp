#include "RenderWidget.h"

#include "Runtime/Engine.h"

#include <QTimer>

namespace Zongine {
    RenderWidget::RenderWidget(QWidget* parent /*= nullptr*/) {
        setAttribute(Qt::WA_NativeWindow);
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_NoSystemBackground);

        hwnd = reinterpret_cast<HWND>(winId());
        auto nWidth = width();
        auto nHeight = height();

        engine = std::make_unique<Engine>();
        engine->Initialize(hwnd);

        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &RenderWidget::Tick);
        timer->start(16);
    }

    void RenderWidget::resizeEvent(QResizeEvent* event) {
        // engine->Resize(width(), height());
        QWidget::resizeEvent(event);
    }

    void RenderWidget::paintEvent(QPaintEvent* event) {
        // engine->Run();
    }

    void RenderWidget::Tick() {
        if (engine->IsRunning())
            engine->Tick();
    }
}
