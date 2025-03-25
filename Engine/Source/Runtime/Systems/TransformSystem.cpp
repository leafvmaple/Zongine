#include "TransformSystem.h"

#include "Managers/Mananger.h"

#include "Components/TransformComponent.h"
#include "Components/MeshComponent.h"

namespace Zongine {
    using namespace DirectX;

    bool TransformSystem::Initialize(const ManagerList& info) {
        m_EntityManager = info.entityManager;

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
                auto& mesh = child.GetComponent<MeshComponent>();
                //auto& parentBone = mesh.Bones[mesh.BoneMap[component.TargetName]]; // TODO
                //targetMatrix = transformComponent.World * XMLoadFloat4x4(&parentBone.Offset);
            }

            _UpdateWorldTransformRecursive(child, targetMatrix);
        }
    }
}