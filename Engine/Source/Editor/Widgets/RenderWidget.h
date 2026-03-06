#pragma once

#include <QWidget>
#include <Windows.h>
#include <memory>

namespace Zongine {
    class Engine;
    class EngineThread;

    // =========================================================================
    // RenderWidget -- Hosts the D3D11 render surface and manages EngineThread
    //
    // The engine tick loop now runs on a dedicated thread (EngineThread)
    // instead of QTimer, keeping the Qt UI thread responsive during heavy
    // rendering. The thread is started after Initialize and stopped on
    // destruction.
    // =========================================================================
    class RenderWidget : public QWidget {
        Q_OBJECT
    public:
        explicit RenderWidget(std::shared_ptr<Engine> engine, QWidget* parent = nullptr);
        ~RenderWidget();

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void paintEvent(QPaintEvent* event) override;

    private:
        HWND hwnd{};
        std::shared_ptr<Engine> m_Engine;
        EngineThread* m_EngineThread{ nullptr };
    };
}