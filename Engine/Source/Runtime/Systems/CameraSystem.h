#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

namespace Zongine {
    class EntityManager;
    class WindowManager;
    class DeviceManager;

    struct CameraSystemInfo {
        std::shared_ptr<EntityManager> entityManager{};
        std::shared_ptr<WindowManager> windowManager{};
        std::shared_ptr<DeviceManager> deviceManager{};
    };

    class CameraSystem {
    public:
        bool Initialize(const CameraSystemInfo& info);
        void Tick(float fDeltaTime);
    private:
        std::shared_ptr<EntityManager> m_EntityManager{};
    };
}