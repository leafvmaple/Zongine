#pragma once

#include "BaseComponent.h"

#include <d3d11.h>
#include <memory>

namespace Zongine {
    class MaterialComponent : public BaseComponent {
        std::shared_ptr<ID3D11InputLayout> pInputLayout;
        D3D11_PRIMITIVE_TOPOLOGY topology;
    };
}