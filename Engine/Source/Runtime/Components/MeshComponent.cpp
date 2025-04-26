#include "MeshComponent.h"

#include "../Managers/AssetManager.h"

namespace Zongine {

    void MeshComponent::Initialize(const Entity& entity) {
        auto mesh = AssetManager::GetInstance().GetMeshAsset(Path);

        BoneModelTransforms.resize(mesh->Bones.size());
        SkinningTransforms.resize(mesh->Bones.size());

        for (int i = 0; i < mesh->Bones.size(); i++) {
            XMStoreFloat4x4(&BoneModelTransforms[i], XMMatrixInverse(nullptr, XMLoadFloat4x4(&mesh->Bones[i].InversePoseTransform)));
            XMStoreFloat4x4(&SkinningTransforms[i],
                XMLoadFloat4x4(&mesh->Bones[i].InversePoseTransform) * XMLoadFloat4x4(&BoneModelTransforms[i]));
        }
    }
}