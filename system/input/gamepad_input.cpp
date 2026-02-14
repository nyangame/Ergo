#include "gamepad_input.hpp"

// GLFW gamepad integration:
// When GLFW is available, poll joystick state from GLFW.
// Currently a stub that can be connected to GLFW's glfwGetGamepadState.

void GamepadInput::poll() {
#ifdef ERGO_HAS_GLFW
    for (int i = 0; i < MAX_GAMEPADS; ++i) {
        // Would call glfwGetGamepadState(GLFW_JOYSTICK_1 + i, &glfw_state) here
        states_[i].connected = false;
    }
#else
    for (int i = 0; i < MAX_GAMEPADS; ++i) {
        states_[i].connected = false;
    }
#endif
}

bool GamepadInput::is_connected(int index) const {
    if (index < 0 || index >= MAX_GAMEPADS) return false;
    return states_[index].connected;
}

GamepadState GamepadInput::state(int index) const {
    if (index < 0 || index >= MAX_GAMEPADS) return {};
    return states_[index];
}
