#include "NVFlexComponent.h"

#include "../Managers/AssetManager.h"

#include "NVFlex/include/NvFlexExt.h"

namespace Zongine {
    void Initialize(NvFlexComponent& flexComponent, const std::string& path) {
        auto mesh = AssetManager::GetInstance().GetMeshAsset(path);
        flexComponent.asset = std::make_shared<NvFlexExtAsset>();

        for (const auto& vertex : mesh->Vertices) {
            const auto& diffuse = vertex.Color;
            float invMass = 0.f;
            if (diffuse.a >= 255)
                invMass = 0.f;
            else if (diffuse.a == 0)
                invMass = 1.f;
            else if (diffuse.a >= 100)
                invMass = std::pow(1.10f, 100.f - diffuse.a);
            else
                invMass = std::pow(1.025f, 100.f - diffuse.a);

            // flexComponent.InvMass.push_back(invMass);
            if (invMass != 0) {
                flexComponent.Particles.emplace_back(DirectX::XMFLOAT4(vertex.Position.x, vertex.Position.y, vertex.Position.z, invMass));
                flexComponent.Phases.emplace_back(NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseSelfCollideFilter));
            }
        }
    }
}