#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

namespace Zongine {
    class EntityManager;
    class WindowManager;

    struct CameraSystemInfo {
        std::shared_ptr<EntityManager> entityManager{};
        std::shared_ptr<WindowManager> windowManager{};
    };

    class CameraSystem {
    public:
        bool Initialize(const CameraSystemInfo& info);
        void Tick(float fDeltaTime);
    private:
        std::shared_ptr<EntityManager> m_EntityManager{};
    };
}