#pragma once

#include <QObject>

namespace Zongine {
    class IEditorBridge;

    // =========================================================================
    // QtEventBridge -- Converts engine events into Qt signals
    //
    // Engine events (string-keyed callbacks) are not Qt-aware. This bridge
    // subscribes to engine events and re-emits them as Qt signals, ensuring
    // safe cross-thread communication via Qt's event loop.
    // =========================================================================
    class QtEventBridge : public QObject {
        Q_OBJECT

    public:
        explicit QtEventBridge(QObject* parent = nullptr) : QObject(parent) {}

        // Call after Engine is initialized to wire up event subscriptions
        void ConnectToEngine(IEditorBridge& bridge);

    signals:
        void entityTreeChanged();
        void componentChanged();
    };
}
