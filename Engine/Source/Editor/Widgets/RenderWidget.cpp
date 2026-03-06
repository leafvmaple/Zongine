#include "RenderWidget.h"
#include "EngineThread.h"

#include "Runtime/Engine.h"

namespace Zongine {
    RenderWidget::RenderWidget(std::shared_ptr<Engine> engine, QWidget* parent /*= nullptr*/) {
        setAttribute(Qt::WA_NativeWindow);
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_NoSystemBackground);

        hwnd = reinterpret_cast<HWND>(winId());

        engine->Initialize(hwnd);
        m_Engine = engine;

        // Start the engine tick loop on a dedicated thread
        m_EngineThread = new EngineThread(engine, this);
        m_EngineThread->start();
    }

    RenderWidget::~RenderWidget() {
        if (m_EngineThread) {
            m_EngineThread->Stop();
        }
    }

    void RenderWidget::resizeEvent(QResizeEvent* event) {
        // engine->Resize(width(), height());
        QWidget::resizeEvent(event);
    }

    void RenderWidget::paintEvent(QPaintEvent* event) {
        // Rendering is handled by EngineThread
    }
}
