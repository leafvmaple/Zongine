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
        D3D11_BUFFER_DESC bufferDesc{};
        ComPtr<ID3D11Buffer> buffer{};

        auto device = DeviceManager::GetInstance().GetDevice();

        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.ByteWidth = sizeof(CAMERA);
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        device->CreateBuffer(&bufferDesc, nullptr, buffer.GetAddressOf());

        auto& entities = EntityManager::GetInstance().GetEntities<CameraComponent>();
        for (auto& [entityID, cameraComponent] : entities) {
            // cameraComponent.Perspective.fFovAngleY = XM_PIDIV2;
            cameraComponent.Perspective.fFovAngleY = XMConvertToRadians(30);
            cameraComponent.Perspective.fAspectRatio = (float)WindowManager::GetInstance().GetWidth() / WindowManager::GetInstance().GetHeight();
            cameraComponent.Buffer = buffer;
        }

        return true;
    }

    void CameraSystem::Tick(float fDeltaTime) {
        auto& entities = EntityManager::GetInstance().GetEntities<CameraComponent>();
        for (auto& [entityID, cameraComponent] : entities) {
            auto& entity = EntityManager::GetInstance().GetEntity(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();

            auto& cameraInfo = cameraComponent.Camera;
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