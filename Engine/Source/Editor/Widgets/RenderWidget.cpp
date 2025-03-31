#include "RenderWidget.h"

#include "Runtime/Engine.h"

#include <QTimer>

namespace Zongine {
    RenderWidget::RenderWidget(std::shared_ptr<Engine> engine, QWidget* parent /*= nullptr*/) {
        setAttribute(Qt::WA_NativeWindow);
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_NoSystemBackground);

        hwnd = reinterpret_cast<HWND>(winId());
        auto nWidth = width();
        auto nHeight = height();

        engine->Initialize(hwnd);

        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &RenderWidget::Tick);
        timer->start(16);

        m_Engine = engine;
    }

    void RenderWidget::resizeEvent(QResizeEvent* event) {
        // engine->Resize(width(), height());
        QWidget::resizeEvent(event);
    }

    void RenderWidget::paintEvent(QPaintEvent* event) {
        // engine->Run();
    }

    void RenderWidget::Tick() {
        if (m_Engine->IsRunning())
            m_Engine->Tick();
    }
}
