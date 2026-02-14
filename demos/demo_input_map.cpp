#include "demo_framework.hpp"
#include "engine/core/input_map.hpp"
#include <cstdio>

DEMO(InputMap_Actions) {
    InputMap imap;

    InputAction jump;
    jump.name = "jump";
    jump.keys = {32};  // Space
    imap.register_action(jump);

    InputAction fire;
    fire.name = "fire";
    fire.keys = {90};  // Z key
    imap.register_action(fire);

    InputAction move_right;
    move_right.name = "move_right";
    move_right.keys = {262};  // Right arrow
    move_right.gamepad_axis = 0;
    move_right.dead_zone = 0.15f;
    imap.register_action(move_right);

    std::printf("  Registered actions: jump, fire, move_right\n");

    // Simulate key press
    imap.set_key_state(32, true);   // Space down
    imap.set_key_state(90, false);  // Z not pressed
    std::printf("  Space down -> jump=%s, fire=%s\n",
                imap.is_action_down("jump") ? "DOWN" : "up",
                imap.is_action_down("fire") ? "DOWN" : "up");

    // Simulate pressed (transition)
    imap.set_previous_key_state(32, false);  // Was not down last frame
    std::printf("  jump pressed (new press)=%s\n",
                imap.is_action_pressed("jump") ? "PRESSED" : "held");

    // Gamepad axis
    imap.set_gamepad_axis(0, 0.8f);
    std::printf("  Gamepad axis 0=0.8 -> move_right axis=%.2f\n",
                imap.get_axis("move_right"));

    imap.set_gamepad_axis(0, 0.1f);
    std::printf("  Gamepad axis 0=0.1 (below deadzone) -> move_right axis=%.2f\n",
                imap.get_axis("move_right"));
}
