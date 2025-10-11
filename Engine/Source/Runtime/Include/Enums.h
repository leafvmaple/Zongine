#pragma once

#include <DirectXMath.h>

namespace Zongine {
    enum RASTERIZER_STATE_ID {
        RASTERIZER_SOLID_CULL_NONE,
        RASTERIZER_SOLID_CULL_CLOCKWISE,

        RASTERIZER_STATE_COUNT,
    };

    enum BLEND_STATE {
        BLEND_STATE_OPAQUE,
        BLEND_STATE_MASKED,
        BLEND_STATE_SOFTMASKED,
        BLEND_STATE_OIT,
        BLEND_STATE_COPY,

        BLEND_STATE_COUNT,
    };

    enum SAMPLER_STATE_ID {
        SAMPLER_STATE_DEFAULT,
        SAMPLER_STATE_POINT_CLAMP,
        SAMPLER_STATE_LINEAR3_CLAMP,

        SAMPLER_STATE_COUNT
    };

    enum DEPTHSTENCIL_STATE_ID {
        ZWRITE_ENABLE,
        ZWRITE_DISABLE,

        DEPTH_STENCIL_STATE_COUNT
    };

    enum RUNTIME_MACRO {
        RUNTIME_MACRO_MESH,
        RUNTIME_MACRO_SKIN_MESH,
        RUNTIME_MACRO_TERRAIN,
        RUNTIME_MACRO_FLEX_MESH,

        RUNTIME_MACRO_COUNT,
    };

    enum class RENDER_PASS {
        COLOR,
        COLORSOFTMASK,
        OIT,
    };

    // Color constants (compatible with DirectX Colors naming style)
    namespace Colors {
        constexpr DirectX::XMFLOAT4 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
        constexpr DirectX::XMFLOAT4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
        constexpr DirectX::XMFLOAT4 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
        constexpr DirectX::XMFLOAT4 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
        constexpr DirectX::XMFLOAT4 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
        constexpr DirectX::XMFLOAT4 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
        constexpr DirectX::XMFLOAT4 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
        constexpr DirectX::XMFLOAT4 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
        constexpr DirectX::XMFLOAT4 Gray = { 0.5f, 0.5f, 0.5f, 1.0f };
        constexpr DirectX::XMFLOAT4 DarkGray = { 0.25f, 0.25f, 0.25f, 1.0f };
        constexpr DirectX::XMFLOAT4 LightGray = { 0.75f, 0.75f, 0.75f, 1.0f };
        constexpr DirectX::XMFLOAT4 Transparent = { 0.0f, 0.0f, 0.0f, 0.0f };
        constexpr DirectX::XMFLOAT4 CornflowerBlue = { 0.392f, 0.584f, 0.929f, 1.0f };
        constexpr DirectX::XMFLOAT4 Orange = { 1.0f, 0.647f, 0.0f, 1.0f };
        constexpr DirectX::XMFLOAT4 Purple = { 0.5f, 0.0f, 0.5f, 1.0f };
    }

    enum BONE_FLAG {
        BONE_FLAG_NONE = 0,
        BONE_FLAG_AFFLINE = 1 << 0,
        BONE_FLAG_NO_UPDATE = 1 << 1,
    };

    enum FLEX_OBJECT_TYPE {
        FLEX_OBJECT_NONE,
        FLEX_OBJECT_CLOTH,
        FLEX_OBJECT_SOFTBODY,
        FLEX_OBJECT_DYNAMIC_MESH
    };
}
