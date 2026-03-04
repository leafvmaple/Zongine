#pragma once

#include <vector>

#include "../Include/Types.h"

namespace Zongine {
    struct HierarchyComponent {
        EntityID Parent = 0;
        std::vector<EntityID> Children{};
    };
}
