#pragma once

#include <string>
#include <memory>

namespace Zongine {
    // =========================================================================
    // ICommand -- Command pattern interface for undo/redo support
    //
    // All editor operations that modify ECS data should be wrapped in a
    // command object. This allows the editor to maintain a history stack
    // and support Ctrl+Z / Ctrl+Y undo/redo.
    // =========================================================================
    class ICommand {
    public:
        virtual ~ICommand() = default;

        virtual void Execute() = 0;
        virtual void Undo() = 0;
        virtual std::string GetDescription() const = 0;
    };
}
