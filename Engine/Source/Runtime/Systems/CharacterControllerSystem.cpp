#include "CharacterControllerSystem.h"

#include "Entities/EntityManager.h"
#include "Entities/Entity.h"
#include "Components/InputComponent.h"
#include "Components/CharacterControllerComponent.h"
#include "Components/AnimationStateComponents.h"
#include "Animation/AnimStateMachineBuilder.h"

namespace Zongine {
    void CharacterControllerSystem::Initialize() {
        // Get global input entity ID
        auto& entities = EntityManager::GetInstance().GetEntities();
        for (auto& [id, entity] : entities) {
            if (entity.GetName() == "__GlobalInput__") {
                m_InputEntity = id;
                break;
            }
        }
    }

    void CharacterControllerSystem::Tick(float deltaTime) {
        // Get global input
        if (m_InputEntity == 0) return;

        auto& inputEntity = EntityManager::GetInstance().GetEntity(m_InputEntity);
        if (!inputEntity.HasComponent<InputComponent>()) return;

        const auto& input = inputEntity.GetComponent<InputComponent>();

        // Iterate through all character controllers
        EntityManager::GetInstance().ForEach<CharacterControllerComponent>(
            [this, &input, deltaTime](EntityID entityID, auto& controller) {
                if (!controller.EnableInput) return;

                auto& entity = EntityManager::GetInstance().GetEntity(entityID);
                _UpdateCharacter(entity, input, deltaTime);
            });
    }

    void CharacterControllerSystem::_UpdateCharacter(
        Entity& character, 
        const InputComponent& input, 
        float deltaTime) {

        // Ensure character has animation parameter component
        if (!character.HasComponent<AnimParameterCollectionComponent>()) {
            return;
        }

        // Read input
        bool wPressed = input.IsKeyHeld(Key::W);
        bool aPressed = input.IsKeyHeld(Key::A);
        bool sPressed = input.IsKeyHeld(Key::S);
        bool dPressed = input.IsKeyHeld(Key::D);
        bool spacePressed = input.IsKeyDown(Key::Space);
        bool shiftPressed = input.IsKeyHeld(Key::Shift);

        // Calculate movement speed
        float speed = 0.0f;
        if (wPressed || aPressed || sPressed || dPressed) {
            auto& controller = character.GetComponent<CharacterControllerComponent>();
            
            if (shiftPressed) {
                speed = controller.RunSpeed;  // Hold Shift to run
            } else {
                speed = controller.MoveSpeed;  // Normal movement
            }
        }

        // Update animation parameters
        AnimParameterHelper::SetFloat(character, "Speed", speed);

        // Jump trigger
        if (spacePressed) {
            AnimParameterHelper::SetTrigger(character, "Jump");
        }

        // Can add more control logic...
        // For example: calculate movement direction based on input, rotate character, etc.
    }
}
