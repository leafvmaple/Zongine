#include "MeshComponent.h"

#include "../Managers/AssetManager.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace Zongine {
    void MeshComponent::Initialize() {
        auto mesh = AssetManager::GetInstance().GetMeshAsset(Path);

        BoneModelTransforms.resize(mesh->Bones.size());
        SkinningTransforms.resize(mesh->Bones.size());

        for (int i = 0; i < mesh->Bones.size(); i++) {
            XMVECTOR det;
            XMMATRIX invPose = XMLoadFloat4x4(&mesh->Bones[i].InversePoseTransform);
            XMMATRIX pose = XMMatrixInverse(&det, invPose);
            
            XMStoreFloat4x4(&BoneModelTransforms[i], pose);
            XMStoreFloat4x4(&SkinningTransforms[i], invPose * pose);
        }
    }
}