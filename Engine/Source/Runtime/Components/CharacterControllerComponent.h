#pragma once

namespace Zongine {
    /**
     * Character Controller Component
     * 
     * Marks entity to be processed by CharacterControllerSystem
     * Can store character-related configuration
     */
    struct CharacterControllerComponent {
        float MoveSpeed = 5.0f;           // Movement speed
        float RunSpeed = 10.0f;           // Running speed
        bool EnableInput = true;          // Whether input control is enabled
    };
}
