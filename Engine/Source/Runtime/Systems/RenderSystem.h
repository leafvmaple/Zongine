#pragma once

#include <d3d11.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

#include "Entities/Entity.h"

namespace Zongine {
    class EntityManager;
    class DeviceManager;
    class ShaderManager;
    class StateManager;
    class EffectManager;

    using Microsoft::WRL::ComPtr;

    struct RenderSystemInfo {
        std::shared_ptr<EntityManager> entityManager;
        std::shared_ptr<DeviceManager> deviceManager;
        std::shared_ptr<ShaderManager> shaderManager;
        std::shared_ptr<StateManager> stateManager;
        std::shared_ptr<EffectManager> effectManager;
    };

    class RenderEntity {

    };

    class RenderSystem {
    public:
        bool Initialize(const RenderSystemInfo& info);
        void Tick(float fDeltaTime);

    private:
        std::shared_ptr<EntityManager> m_EntityManager{};
        std::shared_ptr<DeviceManager> m_WindowManager{};
        std::shared_ptr<ShaderManager> m_ShaderManager{};
        std::shared_ptr<StateManager> m_StateManager{};
        std::shared_ptr<EffectManager> m_EffectManager{};

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_SharedShaderCommonBuffer{};

        std::vector<RenderEntity> m_RenderEntities{};

        void _InitializeConstantBuffer();

        void _UpdateConstantBuffer();
        void _UpdateEffect();
    };
}
