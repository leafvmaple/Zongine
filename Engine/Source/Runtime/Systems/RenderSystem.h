#pragma once

#include <d3d11.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

#include "Entities/Entity.h"

#include "components/CameraComponent.h"

namespace Zongine {
    struct ManagerList;

    class EntityManager;
    class DeviceManager;
    class ShaderManager;
    class StateManager;
    class EffectManager;
    class ResourceManager;

    using Microsoft::WRL::ComPtr;

    class RenderEntity {

    };

    class RenderSystem {
    public:
        bool Initialize(const ManagerList& info);
        void Tick(float fDeltaTime);

    private:
        __declspec(align(16)) struct SWITCH
        {
            int bEnableSunLight = 0;
            int bEnableConvertMap = 0;
            int bEnableIBL = 0;
            int bEnableFog = 0;
        };

        __declspec(align(16)) struct SHARED_SHADER_COMMON
        {
            SWITCH Switch;
            XMMATRIX CameraView;
            XMMATRIX CameraProject;
        };

        std::shared_ptr<EntityManager> m_EntityManager{};
        std::shared_ptr<DeviceManager> m_DeviceManager{};
        std::shared_ptr<ShaderManager> m_ShaderManager{};
        std::shared_ptr<StateManager> m_StateManager{};
        std::shared_ptr<EffectManager> m_EffectManager{};
        std::shared_ptr<ResourceManager> m_ResourceManger{};

        std::vector<Entity> m_RenderQueue{};

        ComPtr<ID3D11Buffer> m_CameraBuffer{};

        void TickEntity(ComPtr<ID3D11DeviceContext> context, const Entity& entity);

        void _UpdateRenderQueue(Entity& entity);
    };
}
