#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <cstdint>

struct InputAction {
    std::string name;
    std::vector<uint32_t> keys;
    std::vector<uint32_t> gamepad_buttons;
    int gamepad_axis = -1;
    float dead_zone = 0.15f;
};

class InputMap {
public:
    void register_action(InputAction action);
    void unregister_action(std::string_view name);

    bool is_action_down(std::string_view name) const;
    bool is_action_pressed(std::string_view name) const;
    float get_axis(std::string_view name) const;

    const InputAction* get_action(std::string_view name) const;

    // Set key state (called by the input system each frame)
    void set_key_state(uint32_t key, bool down);
    void set_previous_key_state(uint32_t key, bool down);
    void set_gamepad_axis(int axis_id, float value);

    void clear_actions() { actions_.clear(); }

private:
    std::unordered_map<std::string, InputAction> actions_;

    static constexpr uint32_t MAX_KEYS = 512;
    bool key_current_[MAX_KEYS]{};
    bool key_previous_[MAX_KEYS]{};
    float gamepad_axes_[8]{};
};
