#include "TransformSystem.h"

#include "Entities/World.h"
#include "Managers/AssetManager.h"

#include "Components/TransformComponent.h"
#include "Components/MeshComponent.h"

#include "Maths/Matrix.h"

namespace Zongine {
    using namespace DirectX;
    using Zongine::XMConvertToRadians;

    void TransformSystem::Tick(float fDeltaTime) {
        DirectX::XMFLOAT4X4 identity{};
        XMStoreFloat4x4(&identity, XMMatrixIdentity());
        _UpdateWorldTransformRecursive(World::GetInstance().GetRootEntity(), identity);
    }

    void TransformSystem::_UpdateWorldTransformRecursive(EntityID entityID, const DirectX::XMFLOAT4X4& parentMatrix) {
        auto& world = World::GetInstance();
        auto& transformComponent = world.Get<TransformComponent>(entityID);
        auto& rotation = transformComponent.Rotation;

        auto localMatrix = XMMatrixTransformation(
            g_XMZero,
            g_XMIdentityR3,
            XMLoadFloat3(&transformComponent.Scale),
            g_XMZero,
            XMQuaternionRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z)),
            XMLoadFloat3(&transformComponent.Position)
        );

        XMStoreFloat4x4(&transformComponent.World, XMLoadFloat4x4(&parentMatrix) * localMatrix);

        for (auto& childID : world.GetChildren(entityID)) {
            if (!world.Has<TransformComponent>(childID)) continue;
            auto& component = world.Get<TransformComponent>(childID);

            auto targetMatrix = transformComponent.World;

            if (component.BindType == BIND_TYPE::Socket) {
                auto& meshComponent = world.Get<MeshComponent>(entityID);
                auto mesh = AssetManager::GetInstance().GetMeshAsset(meshComponent.Path);
                if (component.SocketIndex == -1) {
                    auto it = std::lower_bound(mesh->Sockets.begin(), mesh->Sockets.end(), component.TargetName,
                        [](const SOCKET& socket, const std::string& value) {
                            return socket.Name < value;
                        });
                    if (it != mesh->Sockets.end() && it->Name == component.TargetName) {
                        component.SocketIndex = it - mesh->Sockets.begin();
                    }
                }
                auto& socket = mesh->Sockets[component.SocketIndex];
                auto& modelTransform = meshComponent.BoneModelTransforms[socket.nParentBoneIndex];

                XMStoreFloat4x4(&targetMatrix,
                    XMLoadFloat4x4(&socket.Offset) * XMLoadFloat4x4(&modelTransform) * XMLoadFloat4x4(&transformComponent.World));
            }

            _UpdateWorldTransformRecursive(childID, targetMatrix);
        }
    }
}