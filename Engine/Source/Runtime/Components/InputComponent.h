#pragma once

#include <unordered_set>
#include <unordered_map>
#include <Windows.h>

namespace Zongine {
    /**
     * InputComponent - Stores input state for the current frame
     * 
     * This is a global singleton component, usually only one instance
     * Attached to a dedicated input entity
     */
    struct InputComponent {
    // Keys pressed in the current frame
        std::unordered_set<int> KeysDown;
        
    // Keys released in the current frame
        std::unordered_set<int> KeysUp;
        
    // Keys currently being held down
        std::unordered_set<int> KeysHeld;
        
    // Mouse position
        int MouseX = 0;
        int MouseY = 0;
        
    // Mouse movement delta
        int MouseDeltaX = 0;
        int MouseDeltaY = 0;
        
    // Mouse button states
        bool LeftMouseDown = false;
        bool LeftMouseHeld = false;
        bool RightMouseDown = false;
        bool MiddleMouseDown = false;
        
    // Mouse wheel value
        int MouseWheel = 0;

    /**
     * Check if a key was pressed in this frame
     */
        bool IsKeyDown(int keyCode) const {
            return KeysDown.find(keyCode) != KeysDown.end();
        }

    /**
     * Check if a key was released in this frame
     */
        bool IsKeyUp(int keyCode) const {
            return KeysUp.find(keyCode) != KeysUp.end();
        }

    /**
     * Check if a key is currently held down
     */
        bool IsKeyHeld(int keyCode) const {
            return KeysHeld.find(keyCode) != KeysHeld.end();
        }

    /**
     * Clear transient events for this frame (call at end of frame)
     */
        void ClearFrameEvents() {
            KeysDown.clear();
            KeysUp.clear();
            MouseDeltaX = 0;
            MouseDeltaY = 0;
            MouseWheel = 0;
        }
    };

    // Common key constants (Windows Virtual Key Codes)
    namespace Key {
        constexpr int W = 0x57;
        constexpr int A = 0x41;
        constexpr int S = 0x53;
        constexpr int D = 0x44;
        constexpr int Space = VK_SPACE;
        constexpr int Shift = VK_SHIFT;
        constexpr int Ctrl = VK_CONTROL;
        constexpr int Escape = VK_ESCAPE;
        constexpr int Up = VK_UP;
        constexpr int Down = VK_DOWN;
        constexpr int Left = VK_LEFT;
        constexpr int Right = VK_RIGHT;
    }
}
