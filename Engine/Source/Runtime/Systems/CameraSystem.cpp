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

        auto& entities = m_EntityManager->GetEntities<CameraComponent>();
        for (auto& [entityID, cameraComponent] : entities) {
            cameraComponent.Perspective.fFovAngleY = XM_PIDIV2;
            cameraComponent.Perspective.fAspectRatio = info.windowManager->GetWidth() / info.windowManager->GetHeight();
        }

        return true;
    }

    void CameraSystem::Tick(float fDeltaTime) {
        auto& entities = m_EntityManager->GetEntities<CameraComponent>();
        for (auto& [entityID, cameraComponent] : entities) {
            auto& entity = m_EntityManager->GetEntity(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();

            auto cameraInfo = cameraComponent.Camera;
            auto& perspective = cameraComponent.Perspective;

            XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
                XMConvertToRadians(transformComponent.Rotation.x), // Pitch
                XMConvertToRadians(transformComponent.Rotation.y), // Yaw
                XMConvertToRadians(transformComponent.Rotation.z)  // Roll
            );

            cameraInfo.CameraView = XMMatrixLookAtLH(
                XMLoadFloat3(&transformComponent.Position),
                XMLoadFloat3(&transformComponent.Position) + XMVector3Normalize(rotationMatrix.r[2]),
                XMVector3Normalize(rotationMatrix.r[1])
            );
            cameraInfo.CameraProject = DirectX::XMMatrixPerspectiveFovLH(perspective.fFovAngleY, perspective.fAspectRatio, 1.0f, 1000.0f);
        }
    }
}