#include "CameraSystem.h"

#include "Entities/EntityManager.h"
#include "Utilities/WindowManager.h"

#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"

#include <DirectXMath.h>
using namespace DirectX;

namespace Zongine {
    bool CameraSystem::Initialize(const CameraSystemInfo& info) {
        m_EntityManager = info.entityManager;

        auto entities = m_EntityManager->GetEntities<CameraComponent>();
        for (auto& [entityID, cameraComponent] : entities) {
            cameraComponent.Persective.fFovAngleY = XM_PIDIV2;
            cameraComponent.Persective.fAspectRatio = info.windowManager->GetWidth() / info.windowManager->GetHeight();
        }

        return true;
    }

    void CameraSystem::Tick(float fDeltaTime) {
        auto entities = m_EntityManager->GetEntities<CameraComponent>();
        for (auto& [entityID, cameraComponent] : entities) {
            auto& entity = m_EntityManager->GetEntity(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();

            auto cameraInfo = cameraComponent.Camera;
            auto& persective = cameraComponent.Persective;

            XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
                DirectX::XMConvertToRadians(transformComponent.Rotation.x), // Pitch
                DirectX::XMConvertToRadians(transformComponent.Rotation.y), // Yaw
                DirectX::XMConvertToRadians(transformComponent.Rotation.z)  // Roll
            );

            cameraInfo.CameraView = XMMatrixLookAtLH(
                XMLoadFloat3(&transformComponent.Position),
                XMLoadFloat3(&transformComponent.Position) + XMVector3Normalize(rotationMatrix.r[2]),
                XMVector3Normalize(rotationMatrix.r[1])
            );
            cameraInfo.CameraProject = DirectX::XMMatrixPerspectiveFovLH(persective.fFovAngleY, persective.fAspectRatio, 1.0f, 1000.0f);
        }
    }
}