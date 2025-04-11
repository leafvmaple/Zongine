#pragma once

#include <string>
#include <vector>

namespace Zongine {
    struct NVFlexComponent {
        std::string Path{};

        std::vector<float> InvMass{};
    };

    void Initialize(NVFlexComponent& flexComponent, const std::string& path);
}