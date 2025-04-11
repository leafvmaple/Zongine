#include "NVFlexComponent.h"

#include "../Managers/AssetManager.h"

#include "NVFlex/include/NvFlexExt.h"

namespace Zongine {
    void Initialize(NVFlexComponent& flexComponent, const std::string& path) {
        auto mesh = AssetManager::GetInstance().GetMeshAsset(path);
        std::shared_ptr<NvFlexExtAsset> asset = std::make_shared<NvFlexExtAsset>();

        for (const auto& vertex : mesh->Vertices) {
            const auto& diffuse = vertex.Color;
            float invMass = 0.f;
            if (diffuse.a >= 255)
                invMass = 0.f;
            else if (diffuse.a == 0)
                invMass = 1.f;
            else if (diffuse.a >= 100)
                invMass = std::pow(1.10f, 100 - diffuse.a);
            else
                invMass = std::pow(1.025f, 100 - diffuse.a);

            flexComponent.InvMass.push_back(invMass);
        }
    }
}