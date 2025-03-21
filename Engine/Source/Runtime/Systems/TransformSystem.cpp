#include "TransformSystem.h"

#include "Components/TransformComponent.h"

namespace Zongine {
    using namespace DirectX;

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

            _UpdateWorldTransformRecursive(child, transformComponent.World);
        }
    }
}