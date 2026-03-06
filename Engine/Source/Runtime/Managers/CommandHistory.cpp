#include "CommandHistory.h"

namespace Zongine {
    void CommandHistory::Execute(std::unique_ptr<ICommand> command) {
        command->Execute();
        m_UndoStack.push_back(std::move(command));

        // Clear redo stack -- new action invalidates the redo branch
        m_RedoStack.clear();

        // Cap history size to prevent unbounded memory growth
        if (m_UndoStack.size() > kMaxHistorySize) {
            m_UndoStack.erase(m_UndoStack.begin());
        }
    }

    bool CommandHistory::Undo() {
        if (m_UndoStack.empty()) return false;

        auto cmd = std::move(m_UndoStack.back());
        m_UndoStack.pop_back();
        cmd->Undo();
        m_RedoStack.push_back(std::move(cmd));
        return true;
    }

    bool CommandHistory::Redo() {
        if (m_RedoStack.empty()) return false;

        auto cmd = std::move(m_RedoStack.back());
        m_RedoStack.pop_back();
        cmd->Execute();
        m_UndoStack.push_back(std::move(cmd));
        return true;
    }

    void CommandHistory::Clear() {
        m_UndoStack.clear();
        m_RedoStack.clear();
    }
}
