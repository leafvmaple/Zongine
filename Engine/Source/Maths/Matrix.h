#pragma once

namespace Zongine {
    constexpr float XM_PI = 3.141592654f;

    constexpr float XMConvertToRadians(float fDegrees) noexcept { return fDegrees * (XM_PI / 180.0f); }
}