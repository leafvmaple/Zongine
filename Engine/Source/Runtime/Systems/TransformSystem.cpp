#include "TransformSystem.h"

#include "Entities/EntityManager.h"
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
        _UpdateWorldTransformRecursive(EntityManager::GetInstance().GetRootEntity(), identity);
    }

    void TransformSystem::_UpdateWorldTransformRecursive(Entity entity, const DirectX::XMFLOAT4X4& parentMatrix) {
        auto& transformComponent = entity.GetComponent<TransformComponent>();
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

        for (auto& id : entity.GetChildren()) {
            auto& child = EntityManager::GetInstance().GetEntity(id);
            auto& component = child.GetComponent<TransformComponent>();

            auto targetMatrix = transformComponent.World;

            if (component.BindType == BIND_TYPE::Socket) {
                auto& meshComponent = entity.GetComponent<MeshComponent>();
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

            _UpdateWorldTransformRecursive(child, targetMatrix);
        }
    }
}