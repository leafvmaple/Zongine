#include "CameraSystem.h"

#include "Entities/EntityManager.h"

#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"

#include <DirectXMath.h>
using namespace DirectX;

namespace Zongine {
    void CameraSystem::Tick(float fDeltaTime) {
        auto entities = m_EntityManager->GetEntities<CameraComponent>();
        for (auto& [entityID, cameraComponent] : entities) {
            auto& entity = m_EntityManager->GetEntity(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();

            auto cameraInfo = cameraComponent.CameraInfo;
            auto& persective = cameraInfo.Property.Persective;

            XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
                DirectX::XMConvertToRadians(transformComponent.Rotation.x), // Pitch
                DirectX::XMConvertToRadians(transformComponent.Rotation.y), // Yaw
                DirectX::XMConvertToRadians(transformComponent.Rotation.z)  // Roll
            );

            cameraInfo.mView = XMMatrixLookAtLH(
                XMLoadFloat3(&transformComponent.Position),
                XMLoadFloat3(&transformComponent.Position) + XMVector3Normalize(rotationMatrix.r[2]),
                XMVector3Normalize(rotationMatrix.r[1])
            );
            cameraInfo.mProject = DirectX::XMMatrixPerspectiveFovLH(persective.fFovAngleY, persective.fAspectRatio, 1.0f, 1000.0f);
        }
    }
}