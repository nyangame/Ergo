#include "test_framework.hpp"
#include "engine/core/input_map.hpp"

TEST_CASE(InputMap_RegisterAction) {
    InputMap imap;
    InputAction jump;
    jump.name = "jump";
    jump.keys = {32};
    imap.register_action(jump);

    auto* action = imap.get_action("jump");
    ASSERT_TRUE(action != nullptr);
    ASSERT_TRUE(action->name == "jump");
}

TEST_CASE(InputMap_IsActionDown) {
    InputMap imap;
    InputAction fire;
    fire.name = "fire";
    fire.keys = {90};
    imap.register_action(fire);

    imap.set_key_state(90, true);
    ASSERT_TRUE(imap.is_action_down("fire"));

    imap.set_key_state(90, false);
    ASSERT_FALSE(imap.is_action_down("fire"));
}

TEST_CASE(InputMap_IsActionPressed) {
    InputMap imap;
    InputAction jump;
    jump.name = "jump";
    jump.keys = {32};
    imap.register_action(jump);

    // Key was not down previous frame, now it is
    imap.set_previous_key_state(32, false);
    imap.set_key_state(32, true);
    ASSERT_TRUE(imap.is_action_pressed("jump"));

    // Key was already down previous frame
    imap.set_previous_key_state(32, true);
    imap.set_key_state(32, true);
    ASSERT_FALSE(imap.is_action_pressed("jump"));
}

TEST_CASE(InputMap_GamepadAxis) {
    InputMap imap;
    InputAction move;
    move.name = "move_x";
    move.gamepad_axis = 0;
    move.dead_zone = 0.15f;
    imap.register_action(move);

    imap.set_gamepad_axis(0, 0.8f);
    ASSERT_NEAR(imap.get_axis("move_x"), 0.8f, 0.001f);

    imap.set_gamepad_axis(0, 0.1f);
    ASSERT_NEAR(imap.get_axis("move_x"), 0.0f, 0.001f);  // Below dead zone
}

TEST_CASE(InputMap_UnregisterAction) {
    InputMap imap;
    InputAction fire;
    fire.name = "fire";
    fire.keys = {90};
    imap.register_action(fire);

    imap.unregister_action("fire");
    auto* action = imap.get_action("fire");
    ASSERT_TRUE(action == nullptr);
}

TEST_CASE(InputMap_NonexistentAction) {
    InputMap imap;
    ASSERT_FALSE(imap.is_action_down("nonexistent"));
    ASSERT_FALSE(imap.is_action_pressed("nonexistent"));
}
