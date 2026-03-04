#include "CharacterControllerSystem.h"

#include "Entities/World.h"
#include "Components/NameComponent.h"
#include "Components/InputComponent.h"
#include "Components/CharacterControllerComponent.h"
#include "Components/AnimationStateComponents.h"
#include "Animation/AnimStateMachineBuilder.h"

namespace Zongine {
    void CharacterControllerSystem::Initialize() {
        // Get global input entity ID
        auto& world = World::GetInstance();
        world.ForEach<NameComponent>([this](EntityID id, NameComponent& name) {
            if (name.Name == "__GlobalInput__") {
                m_InputEntity = id;
            }
        });
    }

    void CharacterControllerSystem::Tick(float deltaTime) {
        auto& world = World::GetInstance();
        // Get global input
        if (m_InputEntity == 0) return;

        if (!world.Has<InputComponent>(m_InputEntity)) return;

        const auto& input = world.Get<InputComponent>(m_InputEntity);

        // Iterate through all character controllers
        world.ForEach<CharacterControllerComponent>(
            [this, &input, deltaTime](EntityID entityID, auto& controller) {
                if (!controller.EnableInput) return;

                _UpdateCharacter(entityID, input, deltaTime);
            });
    }

    void CharacterControllerSystem::_UpdateCharacter(
        EntityID characterID, 
        const InputComponent& input, 
        float deltaTime) {

        auto& world = World::GetInstance();

        // Ensure character has animation parameter component
        if (!world.Has<AnimParameterCollectionComponent>(characterID)) {
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
            auto& controller = world.Get<CharacterControllerComponent>(characterID);
            
            if (shiftPressed) {
                speed = controller.RunSpeed;  // Hold Shift to run
            } else {
                speed = controller.MoveSpeed;  // Normal movement
            }
        }

        // Update animation parameters
        AnimParameterHelper::SetFloat(characterID, "Speed", speed);

        // Jump trigger
        if (spacePressed) {
            AnimParameterHelper::SetTrigger(characterID, "Jump");
        }

        // Can add more control logic...
        // For example: calculate movement direction based on input, rotate character, etc.
    }
}
