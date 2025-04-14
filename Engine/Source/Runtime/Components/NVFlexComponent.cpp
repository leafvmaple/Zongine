#include "NVFlexComponent.h"

#include "../Managers/AssetManager.h"
#include "../Managers/DeviceManager.h"

#include "NVFlex/include/NvFlexExt.h"

namespace Zongine {
    void NvFlexComponent::Initialize() {
        auto flex = AssetManager::GetInstance().GetNvFlexAsset(Path);
        auto mesh = AssetManager::GetInstance().GetMeshAsset(MeshPath);

        for (int i = 0; i < mesh->Vertices.size(); i++) {
            const auto& vertex = mesh->Vertices[i];
            auto invMass = flex->InvMass[i];

            FlexVertices.emplace_back(FLEX_VERTEX_EXT{ {
                    vertex.Position.x,
                    vertex.Position.y,
                    vertex.Position.z,
                    1.f}, invMass });

            if (invMass != 0) {
                ParticleVertices.push_back(i);
                Phases.emplace_back(NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseSelfCollideFilter));
            }
        }
    }
}