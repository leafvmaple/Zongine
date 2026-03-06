#pragma once

#include <QThread>
#include <memory>
#include <atomic>

namespace Zongine {
    class Engine;

    // =========================================================================
    // EngineThread -- Runs Engine::Tick() on a dedicated thread
    //
    // Separates the render/simulation loop from the Qt UI thread so that
    // heavy engine work does not block widget interaction. ECS data access
    // is protected by Engine::GetTickMutex() -- the engine thread locks it
    // during Tick(), and the UI thread (via EditorBridgeImpl) locks it for
    // property reads and writes.
    // =========================================================================
    class EngineThread : public QThread {
        Q_OBJECT

    public:
        explicit EngineThread(std::shared_ptr<Engine> engine, QObject* parent = nullptr);
        ~EngineThread() override;

        void Stop();

    protected:
        void run() override;

    private:
        std::shared_ptr<Engine> m_Engine;
        std::atomic<bool> m_Running{ true };
    };
}
