#include "QtEventBridge.h"

#include "Runtime/Include/IEditorBridge.h"

namespace Zongine {
    void QtEventBridge::ConnectToEngine(IEditorBridge& bridge) {
        bridge.Subscribe("ENTITY_UPDATE", [this]() {
            emit entityTreeChanged();
        });
    }
}
