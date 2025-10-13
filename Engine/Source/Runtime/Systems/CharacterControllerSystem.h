#pragma once

#include "Include/Types.h"

namespace Zongine {
    struct InputComponent;
    class Entity;

    /**
     * Character Controller System
     * 
     * Responsible for converting input to character behavior and updating animation parameters
     * 
     * Workflow:
     * 1. Read global InputComponent
     * 2. Iterate through all CharacterControllerComponents
     * 3. Update AnimParameterComponent based on input
     */
    class CharacterControllerSystem {
    public:
        void Initialize();
        void Tick(float deltaTime);

    private:
        EntityID m_InputEntity = 0;

        void _UpdateCharacter(Entity& character, const InputComponent& input, float deltaTime);
    };
}
