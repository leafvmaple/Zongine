#pragma once

#include <DirectXMath.h>

namespace Zongine {
    enum RASTERIZER_STATE_ID {
        RASTERIZER_STATE_CULL_NONE,
        RASTERIZER_STATE_CULL_CLOCKWISE,

        RASTERIZER_STATE_COUNT,
    };

    enum SAMPLER_STATE_ID {
        SAMPLER_STATE_DEFAULT,
        SAMPLER_STATE_POINT_CLAMP,

        SAMPLER_STATE_COUNT
    };

    enum DEPTHSTENCIL_STATE_ID {
        ZWRITE_ENABLE,

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
    };

    namespace Colors {
        XMGLOBALCONST DirectX::XMFLOAT4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
    }

    enum BONE_FLAG {
        BONE_FLAG_NONE = 0,
        BONE_FLAG_AFFLINE = 1 << 0,
        BONE_FLAG_NO_UPDATE = 1 << 1,
    };

    enum BlendMode {
        BLEND_OPAQUE,
        BLEND_MASKED,
        BLEND_SOFTMASKED = 5,
    };

    enum FLEX_OBJECT_TYPE
    {
        FLEX_OBJECT_NONE,
        FLEX_OBJECT_CLOTH,
        FLEX_OBJECT_SOFTBODY,
        FLEX_OBJECT_DYNAMIC_MESH
    };
}
