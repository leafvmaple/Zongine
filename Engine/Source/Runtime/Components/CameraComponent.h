#pragma once

#include "Entities/Entity.h"

#include <DirectXMath.h>
using DirectX::XMMATRIX;
using DirectX::XMVECTOR;

namespace Zongine {
    struct CAMERA_PROPERTY {
        union {
            struct PERSPECTIVE_PARAM
            {
                float fFovAngleY;
                float fAspectRatio;
            } Persective;
        };
    };

    struct __declspec(align(16)) CAMERA_INFO {
		XMMATRIX mView;
		XMMATRIX mProject;

		CAMERA_PROPERTY Property;
    };

    struct __declspec(align(16)) CameraComponent {
        CAMERA_INFO CameraInfo;

        EntityID target{ INVALID_ENTITY };

        XMVECTOR Offset;
    };
}