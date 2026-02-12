#pragma once
#include "../math/vec2.hpp"
#include "../math/mat4.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

struct Camera2D {
    Vec2f position;
    float zoom = 1.0f;
    float rotation = 0.0f;
    float viewport_width = 800.0f;
    float viewport_height = 600.0f;

    // Orthographic projection (Vulkan: Y down)
    Mat4 view_projection() const {
        float hw = viewport_width * 0.5f / zoom;
        float hh = viewport_height * 0.5f / zoom;

        float cx = position.x + shake_offset_.x;
        float cy = position.y + shake_offset_.y;

        Mat4 proj;
        for (int i = 0; i < 16; ++i) proj.m[i] = 0.0f;
        proj.m[0]  = 1.0f / hw;
        proj.m[5]  = 1.0f / hh;     // Y down for Vulkan
        proj.m[10] = 1.0f;
        proj.m[15] = 1.0f;

        Mat4 view;
        view.m[12] = -cx;
        view.m[13] = -cy;

        if (rotation != 0.0f) {
            float c = std::cos(-rotation);
            float s = std::sin(-rotation);
            Mat4 rot;
            rot.m[0] = c;  rot.m[4] = -s;
            rot.m[1] = s;  rot.m[5] = c;
            return proj * rot * view;
        }

        return proj * view;
    }

    Vec2f world_to_screen(Vec2f world) const {
        float dx = (world.x - position.x) * zoom + viewport_width * 0.5f;
        float dy = (world.y - position.y) * zoom + viewport_height * 0.5f;
        return {dx, dy};
    }

    Vec2f screen_to_world(Vec2f screen) const {
        float wx = (screen.x - viewport_width * 0.5f) / zoom + position.x;
        float wy = (screen.y - viewport_height * 0.5f) / zoom + position.y;
        return {wx, wy};
    }

    // Camera shake
    void shake(float intensity, float duration) {
        shake_intensity_ = intensity;
        shake_duration_ = duration;
        shake_timer_ = duration;
    }

    void update_shake(float dt) {
        if (shake_timer_ <= 0.0f) {
            shake_offset_ = Vec2f::zero();
            return;
        }
        shake_timer_ -= dt;
        float factor = shake_timer_ / shake_duration_;
        float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
        float mag = shake_intensity_ * factor;
        shake_offset_ = {std::cos(angle) * mag, std::sin(angle) * mag};
    }

private:
    Vec2f shake_offset_;
    float shake_intensity_ = 0.0f;
    float shake_duration_ = 0.0f;
    float shake_timer_ = 0.0f;
};
