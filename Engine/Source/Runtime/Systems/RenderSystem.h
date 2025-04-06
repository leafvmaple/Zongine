#pragma once

#include <d3d11.h>
#include <vector>
#include <wrl/client.h>

#include "Entities/Entity.h"

#include "components/CameraComponent.h"

namespace Zongine {
    using Microsoft::WRL::ComPtr;

    class RenderSystem {
    public:
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

        std::vector<Entity> m_RenderQueue{};

        ComPtr<ID3D11Buffer> m_CameraBuffer{};

        void TickEntity(ComPtr<ID3D11DeviceContext> context, const Entity& entity);

        void _UpdateRenderQueue(Entity& entity);
    };
}
