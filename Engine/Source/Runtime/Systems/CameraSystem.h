#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

namespace Zongine {
    class EntityManager;

    struct CameraSystemInfo {
		std::shared_ptr<EntityManager> entityManager{};
    };

    class CameraSystem {
    public:
        void Initialize(const CameraSystemInfo& info) {
			m_EntityManager = info.entityManager;
        }
        void Tick(float fDeltaTime);
    private:
		std::shared_ptr<EntityManager> m_EntityManager{};
    };
}