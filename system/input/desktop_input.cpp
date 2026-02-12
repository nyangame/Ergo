#include "desktop_input.hpp"
#include <array>

static constexpr size_t MAX_KEYS = 256;

struct DesktopInput::Impl {
    std::array<bool, MAX_KEYS> key_current{};
    std::array<bool, MAX_KEYS> key_previous{};
    std::array<bool, 8> mouse_buttons{};
    Vec2f mouse_pos;
};

DesktopInput::DesktopInput() : impl_(new Impl{}) {}

DesktopInput::~DesktopInput() {
    delete impl_;
}

void DesktopInput::poll_events() {
    if (!impl_) return;
    // Save previous state for edge detection
    impl_->key_previous = impl_->key_current;
    // TODO: Read actual keyboard/mouse state from platform (GLFW, Win32, X11, etc.)
}

bool DesktopInput::is_key_down(uint32_t key) const {
    if (!impl_ || key >= MAX_KEYS) return false;
    return impl_->key_current[key];
}

bool DesktopInput::is_key_pressed(uint32_t key) const {
    if (!impl_ || key >= MAX_KEYS) return false;
    return impl_->key_current[key] && !impl_->key_previous[key];
}

Vec2f DesktopInput::mouse_position() const {
    if (!impl_) return {};
    return impl_->mouse_pos;
}

bool DesktopInput::is_mouse_button_down(uint32_t button) const {
    if (!impl_ || button >= 8) return false;
    return impl_->mouse_buttons[button];
}
