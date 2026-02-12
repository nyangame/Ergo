#pragma once
#include <cstdint>
#include <array>

struct GamepadState {
    float left_stick_x = 0.0f;
    float left_stick_y = 0.0f;
    float right_stick_x = 0.0f;
    float right_stick_y = 0.0f;
    float left_trigger = 0.0f;
    float right_trigger = 0.0f;
    std::array<bool, 16> buttons{};
    bool connected = false;
};

class GamepadInput {
public:
    static constexpr int MAX_GAMEPADS = 4;

    void poll();
    bool is_connected(int index) const;
    GamepadState state(int index) const;

private:
    std::array<GamepadState, MAX_GAMEPADS> states_{};
};
