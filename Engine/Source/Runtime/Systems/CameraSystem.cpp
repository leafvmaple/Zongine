#include "CameraSystem.h"

#include "Entities/EntityManager.h"
#include "Managers/DeviceManager.h"
#include "Managers/WindowManager.h"

#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"
#include "Components/InputComponent.h"
#include "Components/NameComponent.h"

#include <DirectXMath.h>
using namespace DirectX;

namespace Zongine {
    bool CameraSystem::Initialize() {
        auto& entities = EntityManager::GetInstance().GetEntities<CameraComponent>();
        for (auto& [entityID, cameraComponent] : entities) {
            cameraComponent.Perspective.fFovAngleY = XMConvertToRadians(30);
            cameraComponent.Perspective.fAspectRatio = (float)WindowManager::GetInstance().GetWidth() / WindowManager::GetInstance().GetHeight();
        }

        return true;
    }

    void CameraSystem::Tick(float fDeltaTime) {
        // Get input component for camera control
        auto& inputEntities = EntityManager::GetInstance().GetEntities<InputComponent>();
        InputComponent* pInputComponent = nullptr;
        for (auto& [entityID, inputComponent] : inputEntities) {
            pInputComponent = &inputComponent;
            break; // Use first input component (should be global input)
        }

        // Find Player entity
        Entity* pPlayerEntity = nullptr;
        XMFLOAT3 playerPosition = { 0.0f, 0.0f, 0.0f };
        auto& nameEntities = EntityManager::GetInstance().GetEntities<NameComponent>();
        for (auto& [entityID, nameComponent] : nameEntities) {
            if (nameComponent.Name == "Player") {
                auto& entity = EntityManager::GetInstance().GetEntity(entityID);
                if (entity.HasComponent<TransformComponent>()) {
                    pPlayerEntity = &entity;
                    playerPosition = entity.GetComponent<TransformComponent>().Position;
                    break;
                }
            }
        }

        auto& entities = EntityManager::GetInstance().GetEntities<CameraComponent>();
        for (auto& [entityID, cameraComponent] : entities) {
            auto& entity = EntityManager::GetInstance().GetEntity(entityID);
            auto& transformComponent = entity.GetComponent<TransformComponent>();

            // Handle camera dragging with left mouse button
            if (pInputComponent && pInputComponent->LeftMouseHeld) {
                // Mouse sensitivity (degrees per pixel)
                const float mouseSensitivity = 0.2f;
                
                // Update rotation based on mouse delta
                transformComponent.Rotation.y += pInputComponent->MouseDeltaX * mouseSensitivity; // Yaw (left/right)
                transformComponent.Rotation.x += pInputComponent->MouseDeltaY * mouseSensitivity; // Pitch (up/down) - Fixed direction
                
                // Clamp pitch to prevent camera flipping
                if (transformComponent.Rotation.x > 89.0f) transformComponent.Rotation.x = 89.0f;
                if (transformComponent.Rotation.x < -89.0f) transformComponent.Rotation.x = -89.0f;
            }

            // Handle mouse wheel zoom
            if (pInputComponent && pInputComponent->MouseWheel != 0) {
                const float zoomSpeed = 20.0f;
                cameraComponent.Distance -= pInputComponent->MouseWheel * zoomSpeed;
                
                // Clamp distance within min/max range
                if (cameraComponent.Distance < cameraComponent.MinDistance) {
                    cameraComponent.Distance = cameraComponent.MinDistance;
                }
                if (cameraComponent.Distance > cameraComponent.MaxDistance) {
                    cameraComponent.Distance = cameraComponent.MaxDistance;
                }
            }

            // Camera parameters
            const float playerHeightOffset = 50.0f; // Height to look at on the player
            
            // Calculate camera position orbiting around the player
            if (pPlayerEntity) {
                // Convert rotation to radians
                float pitch = XMConvertToRadians(transformComponent.Rotation.x);
                float yaw = XMConvertToRadians(transformComponent.Rotation.y);
                
                // Calculate camera position relative to player
                XMFLOAT3 offset;
                offset.x = cameraComponent.Distance * cosf(pitch) * sinf(yaw);
                offset.y = cameraComponent.Distance * sinf(pitch) + playerHeightOffset;
                offset.z = cameraComponent.Distance * cosf(pitch) * cosf(yaw);
                
                // Position camera relative to player
                transformComponent.Position.x = playerPosition.x - offset.x;
                transformComponent.Position.y = playerPosition.y + offset.y;
                transformComponent.Position.z = playerPosition.z - offset.z;
            }

            auto& perspective = cameraComponent.Perspective;
            auto& matrix = cameraComponent.Matrix;

            // Calculate look-at target (player position with height offset)
            XMFLOAT3 targetPosition = playerPosition;
            targetPosition.y += playerHeightOffset; // Look at player's center/head height

            // Create view matrix looking at the player
            XMStoreFloat4x4(&matrix.CameraView, XMMatrixLookAtLH(
                XMLoadFloat3(&transformComponent.Position),
                XMLoadFloat3(&targetPosition),
                XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) // Up vector
            ));

            XMStoreFloat4x4(&matrix.CameraProject, DirectX::XMMatrixPerspectiveFovLH(perspective.fFovAngleY, perspective.fAspectRatio, 1.0f, 1000.0f));
        }
    }
}