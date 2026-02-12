#include "input_map.hpp"
#include <cmath>

void InputMap::register_action(InputAction action) {
    actions_[action.name] = std::move(action);
}

void InputMap::unregister_action(std::string_view name) {
    actions_.erase(std::string(name));
}

const InputAction* InputMap::get_action(std::string_view name) const {
    auto it = actions_.find(std::string(name));
    return (it != actions_.end()) ? &it->second : nullptr;
}

bool InputMap::is_action_down(std::string_view name) const {
    auto* action = get_action(name);
    if (!action) return false;

    for (uint32_t key : action->keys) {
        if (key < MAX_KEYS && key_current_[key]) return true;
    }
    return false;
}

bool InputMap::is_action_pressed(std::string_view name) const {
    auto* action = get_action(name);
    if (!action) return false;

    for (uint32_t key : action->keys) {
        if (key < MAX_KEYS && key_current_[key] && !key_previous_[key]) return true;
    }
    return false;
}

float InputMap::get_axis(std::string_view name) const {
    auto* action = get_action(name);
    if (!action) return 0.0f;

    if (action->gamepad_axis >= 0 && action->gamepad_axis < 8) {
        float v = gamepad_axes_[action->gamepad_axis];
        if (std::abs(v) < action->dead_zone) return 0.0f;
        return v;
    }
    return 0.0f;
}

void InputMap::set_key_state(uint32_t key, bool down) {
    if (key < MAX_KEYS) key_current_[key] = down;
}

void InputMap::set_previous_key_state(uint32_t key, bool down) {
    if (key < MAX_KEYS) key_previous_[key] = down;
}

void InputMap::set_gamepad_axis(int axis_id, float value) {
    if (axis_id >= 0 && axis_id < 8) gamepad_axes_[axis_id] = value;
}
