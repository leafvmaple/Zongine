#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace Zongine {
    inline void XMFloat3Slerp(XMFLOAT3* result, const XMFLOAT3* p0, const XMFLOAT3* p1, float fInterpolation) {
        XMStoreFloat3(result, XMVectorLerp(XMLoadFloat3(p0), XMLoadFloat3(p1), fInterpolation));
    }

    inline void XMFloat4Slerp(XMFLOAT4* result, const XMFLOAT4* p0, const XMFLOAT4* p1, float fInterpolation) {
        XMStoreFloat4(result, XMQuaternionSlerp(XMLoadFloat4(p0), XMLoadFloat4(p1), fInterpolation));
    }
}