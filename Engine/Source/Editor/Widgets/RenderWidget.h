#pragma once

#include <QWidget>
#include <Windows.h>
#include <memory>

namespace Zongine {
    class Engine;

    class RenderWidget : public QWidget {
        Q_OBJECT
    public:
        explicit RenderWidget(QWidget* parent = nullptr);
        ~RenderWidget() = default;

    protected:
        void resizeEvent(QResizeEvent* event) override;
        void paintEvent(QPaintEvent* event) override;

    private slots:
        void Tick();

    private:
        HWND hwnd{};
        std::unique_ptr<Engine> engine{};
    };
}