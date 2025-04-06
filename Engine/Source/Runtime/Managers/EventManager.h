#pragma once

#include "Mananger.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <functional>

namespace Zongine {
    class EventManager : public SingleManager<EventManager> {
    public:
        EventManager() = default;
        ~EventManager() = default;

        void Subscribe(const std::string& event, std::function<void()> listener) {
            m_EventListeners[event].push_back(listener);
        }

        void Emit(const std::string& event) {
            for (auto& listener : m_EventListeners[event]) {
                listener();
            }
        }

    private:
        std::unordered_map<std::string, std::vector<std::function<void()>>> m_EventListeners;
    };
}