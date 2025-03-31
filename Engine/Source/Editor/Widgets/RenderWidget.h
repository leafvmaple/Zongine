#pragma once

#include <QWidget>
#include <Windows.h>
#include <memory>

#include "Runtime/Engine.h"

namespace Zongine {
    class Engine;

    class RenderWidget : public QWidget {
        Q_OBJECT
    public:
        explicit RenderWidget(std::shared_ptr<Engine> engine, QWidget* parent = nullptr);
        ~RenderWidget() = default;

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void paintEvent(QPaintEvent* event) override;

    private slots:
        void Tick();

    private:
        HWND hwnd{};
        std::shared_ptr<Engine> m_Engine{};
    };
}