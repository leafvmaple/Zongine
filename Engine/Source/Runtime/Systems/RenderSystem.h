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
        std::vector<Entity> m_ActorRenderQueue{};
        std::vector<Entity> m_TerrainRenderQueue{};

        ComPtr<ID3D11Buffer> m_CameraBuffer{};

        void TickEntity(ComPtr<ID3D11DeviceContext> context, const Entity& entity);
        void TickTerrain(ComPtr<ID3D11DeviceContext> context, const Entity& entity);

        void _UpdateRenderQueue(Entity& entity);
    };
}
