#include "touch_input.hpp"

struct TouchInput::Impl {
    std::vector<TouchPoint> touch_points;
    Vec2f primary_pos;
};

TouchInput::TouchInput() : impl_(new Impl{}) {}

TouchInput::~TouchInput() {
    delete impl_;
}

void TouchInput::poll_events() {
    if (!impl_) return;
    // TODO: Read touch events from platform (Android NDK / iOS)
}

bool TouchInput::is_key_down(uint32_t /*key*/) const {
    // Virtual key mapping for touch
    return false;
}

bool TouchInput::is_key_pressed(uint32_t /*key*/) const {
    return false;
}

Vec2f TouchInput::mouse_position() const {
    if (!impl_) return {};
    return impl_->primary_pos;
}

const std::vector<TouchPoint>& TouchInput::touches() const {
    static const std::vector<TouchPoint> empty;
    if (!impl_) return empty;
    return impl_->touch_points;
}
