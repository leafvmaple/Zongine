#pragma once

#include <cassert>

namespace Zongine {
    /**
     * SingleManager - Singleton base class with explicit ownership
     * 
     * Engine creates and destroys all managers in a well-defined order.
     * GetInstance() provides global access for convenience, but the
     * Engine class is the true owner of each manager's lifetime.
     * 
     * This pattern:
     * - Gives Engine explicit control over initialization/destruction order
     * - Makes dependencies traceable (Engine holds unique_ptrs)
     * - Keeps existing GetInstance() calls working during gradual migration
     * - Allows future replacement with dependency injection
     */
    template<typename T>
    class SingleManager {
    public:
        static T& GetInstance() {
            assert(s_Instance && "Manager not initialized! Ensure Engine creates it first.");
            return *s_Instance;
        }

        static void Register(T* instance) {
            assert(!s_Instance && "Manager already registered!");
            s_Instance = instance;
        }

        static void Unregister() {
            s_Instance = nullptr;
        }

        static bool IsRegistered() {
            return s_Instance != nullptr;
        }

        SingleManager(const SingleManager&) = delete;
        SingleManager& operator=(const SingleManager&) = delete;
    protected:
        SingleManager() = default;
        ~SingleManager() = default;
    private:
        static inline T* s_Instance = nullptr;
    };
}