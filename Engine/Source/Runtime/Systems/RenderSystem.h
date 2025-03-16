#pragma once

#include <d3d11.h>
#include <memory>
#include <vector>

#include "Entities/Entity.h"

namespace Zongine {
    class EntityManager;
    class DeviceManager;
    class ShaderManager;
    class StateManager;

    struct RenderSystemInfo {
        std::shared_ptr<EntityManager> entityManager;
        std::shared_ptr<DeviceManager> deviceManager;
        std::shared_ptr<ShaderManager> shaderManager;
        std::shared_ptr<StateManager> stateManager;
    };

    class RenderEntity {

    };

    class RenderSystem {
    public:
        void Initialize(RenderSystemInfo info) {
			m_EntityManager = info.entityManager;
            m_WindowManager = info.deviceManager;
            m_ShaderManager = info.shaderManager;
            m_StateManager = info.stateManager;
        }
        void Tick(float fDeltaTime);

    private:
        std::shared_ptr<EntityManager> m_EntityManager{};
        std::shared_ptr<DeviceManager> m_WindowManager{};
        std::shared_ptr<ShaderManager> m_ShaderManager{};
        std::shared_ptr<StateManager> m_StateManager{};
        std::vector<RenderEntity> m_RenderEntities{};
    };
}
