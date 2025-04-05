#pragma once

#include <vector>

#include "../Include/Types.h"

namespace Zongine {
    struct DriverInfo {
        uint32_t index{};
        std::vector<uint32_t> driven{};
    };

    struct FlexibleComponent {
        std::vector<DriverInfo> Drivers{};
    };
}