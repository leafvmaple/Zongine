#include "EngineThread.h"

#include "Runtime/Engine.h"

#include <chrono>
#include <thread>

namespace Zongine {
    EngineThread::EngineThread(std::shared_ptr<Engine> engine, QObject* parent)
        : QThread(parent), m_Engine(std::move(engine))
    {
    }

    EngineThread::~EngineThread() {
        Stop();
    }

    void EngineThread::Stop() {
        m_Running = false;
        if (isRunning()) {
            quit();
            wait(5000);
        }
    }

    void EngineThread::run() {
        while (m_Running && m_Engine->IsRunning()) {
            // Engine::Tick() internally locks GetTickMutex()
            m_Engine->Tick();

            // Yield briefly to let the UI thread acquire the mutex
            // for property reads/writes. Actual frame pacing is handled
            // inside Engine::Tick() (standalone mode) or here (editor mode).
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
