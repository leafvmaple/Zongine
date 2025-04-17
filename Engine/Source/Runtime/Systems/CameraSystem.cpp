#include "CameraSystem.h"

#include "Entities/EntityManager.h"
#include "Managers/DeviceManager.h"
#include "Managers/WindowManager.h"

#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"

#include <DirectXMath.h>
using namespace DirectX;

namespace Zongine {
    bool CameraSystem::Initialize() {
        auto& entities = EntityManager::GetInstance().GetEntities<CameraComponent>();
        for (auto& [entityID, cameraComponent] : entities) {
            // cameraComponent.Perspective.fFovAngleY = XM_PIDIV2;
            cameraComponent.Perspective.fFovAngleY = XMConvertToRadians(30);
            cameraComponent.Perspective.fAspectRatio = (float)WindowManager::GetInstance().GetWidth() / WindowManager::GetInstance().GetHeight();
        }

        return true;
    }

    void CameraSystem::Tick(float fDeltaTime) {
        auto& entities = EntityManager::GetInstance().GetEntities<CameraComponent>();
        for (auto& [entityID, cameraComponent] : entities) {
            auto& entity = EntityManager::GetInstance().GetEntity(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();

            auto& perspective = cameraComponent.Perspective;
            auto& matrix = cameraComponent.Matrix;

            XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
                XMConvertToRadians(transformComponent.Rotation.x), // Pitch
                XMConvertToRadians(transformComponent.Rotation.y), // Yaw
                XMConvertToRadians(transformComponent.Rotation.z)  // Roll
            );

            XMStoreFloat4x4(&matrix.CameraView, XMMatrixLookAtLH(
                XMLoadFloat3(&transformComponent.Position),
                XMLoadFloat3(&transformComponent.Position) + XMVector3Normalize(rotationMatrix.r[2]),
                XMVector3Normalize(rotationMatrix.r[1]))
            );

            XMStoreFloat4x4(&matrix.CameraProject, DirectX::XMMatrixPerspectiveFovLH(perspective.fFovAngleY, perspective.fAspectRatio, 1.0f, 1000.0f));
        }
    }
}