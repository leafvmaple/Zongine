#include "TransformSystem.h"

#include "Managers/Mananger.h"

#include "Components/TransformComponent.h"
#include "Components/MeshComponent.h"

#include "Maths/Matrix.h"

namespace Zongine {
    using namespace DirectX;
    using Zongine::XMConvertToRadians;

    bool TransformSystem::Initialize(const ManagerList& info) {
        m_EntityManager = info.entityManager;
        m_ResourceManger = info.assetManager;

        return true;
    }

    void TransformSystem::Tick(float fDeltaTime) {
        _UpdateWorldTransformRecursive(m_EntityManager->GetRootEntity(), XMMatrixIdentity());
    }

    void TransformSystem::_UpdateWorldTransformRecursive(Entity entity, const XMMATRIX& parentMatrix) {
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

        transformComponent.World = parentMatrix * localMatrix;

        for (auto& id : entity.GetChildren()) {
            auto& child = m_EntityManager->GetEntity(id);
            auto& component = child.GetComponent<TransformComponent>();

            auto targetMatrix = transformComponent.World;

            if (component.BindType == BIND_TYPE::Socket) {
                auto& meshComponent = entity.GetComponent<MeshComponent>();
                auto mesh = m_ResourceManger->GetMeshAsset(meshComponent.Path);
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
                auto& boneMatrix = meshComponent.BoneTransforms[socket.nParentBoneIndex];

                targetMatrix = XMLoadFloat4x4(&socket.Offset) * XMLoadFloat4x4(&boneMatrix) *  transformComponent.World;
            }

            _UpdateWorldTransformRecursive(child, targetMatrix);
        }
    }
}