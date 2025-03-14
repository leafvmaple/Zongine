#pragma once

#include <d3d11.h>
#include <memory>
#include <vector>

#include "Entities/Entity.h"

namespace Zongine {
    struct RenderSystemInitInfo {
    };
    struct RenderSystemTickInfo {
        std::shared_ptr<ID3D11InputLayout> piInputLayout;
        D3D11_PRIMITIVE_TOPOLOGY ePrimitiveTopology;
    };

    class RenderEntity {

    };

    class RenderSystem {
    public:
        void Initialize(RenderSystemInitInfo initInfo);
        void Tick(float fDeltaTime);

        std::shared_ptr<ID3D11Device> GetDevice() const { return m_piDevice; }

    private:
        std::shared_ptr<ID3D11Device> m_piDevice;
        std::shared_ptr<ID3D11DeviceContext> m_piImmediateContext;

        std::vector<RenderEntity> m_RenderEntities;
    };
}
