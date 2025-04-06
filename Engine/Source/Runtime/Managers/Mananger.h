#pragma once

namespace Zongine {
    template<typename T>
    class SingleManager {
    public:
        static constexpr T& GetInstance() {
            static T instance;
            return instance;
        }

        SingleManager(const SingleManager&) = delete;
        SingleManager& operator=(const SingleManager&) = delete;
    protected:
        SingleManager() = default;
        ~SingleManager() = default;
    };
}