#pragma once

#include "../Include/ICommand.h"

#include <vector>
#include <memory>

namespace Zongine {
    // =========================================================================
    // CommandHistory -- Manages undo/redo stacks for editor operations
    //
    // Executing a command pushes it onto the undo stack and clears
    // the redo stack. Undo pops from undo and pushes to redo.
    // Redo pops from redo and pushes back to undo.
    // =========================================================================
    class CommandHistory {
    public:
        CommandHistory() = default;
        ~CommandHistory() = default;

        void Execute(std::unique_ptr<ICommand> command);
        bool Undo();
        bool Redo();

        bool CanUndo() const { return !m_UndoStack.empty(); }
        bool CanRedo() const { return !m_RedoStack.empty(); }

        void Clear();

    private:
        static constexpr size_t kMaxHistorySize = 256;

        std::vector<std::unique_ptr<ICommand>> m_UndoStack;
        std::vector<std::unique_ptr<ICommand>> m_RedoStack;
    };
}
