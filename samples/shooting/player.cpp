#include "player.hpp"
#include "engine/physics/physics_system.hpp"
#include "system/renderer/vulkan/vk_renderer.hpp"

void Player::start() {
    object.object_type_ = static_cast<uint32_t>(GameObjectType::Player);
    object.name_ = "Player";
    hp_ = 100;
    ground_y_ = object.transform_.position.y;
    object.transform_.size = {50.0f, 80.0f};

    // Setup AABB collider
    collider.shape = AABBData{{25.0f, 40.0f}};
    collider.tag = ColliderTag::Player;
    collider.transform = &object.transform_;
    collider.on_hit = [this](const Collider& target) {
        return hit_callback(target);
    };

    g_physics.register_collider(collider);
}

void Player::update(float dt) {
    // Movement is handled through the engine API (is_key_down)
    // This is a skeleton - actual input handling uses the EngineContext

    // Jump physics
    if (jump_y_ != 0.0f || grav_ != 0.0f) {
        object.transform_.position.y += jump_y_;
        jump_y_ -= grav_;
        grav_ += 0.5f;

        // Landing check
        if (object.transform_.position.y >= ground_y_) {
            object.transform_.position.y = ground_y_;
            jump_y_ = 0.0f;
            grav_ = 0.0f;
        }
    }

    // Shooting interval
    if (interval_ > 0) {
        --interval_;
    }

    // Mark as moved for physics
    g_physics.mark_moved(collider);
}

void Player::draw(RenderContext& ctx) {
    auto& t = object.transform_;
    // Draw player as a blue rectangle
    ctx.draw_rect(t.position, t.size, Color{64, 64, 255, 255}, true);
}

void Player::release() {
    g_physics.remove_collider(collider);
}

bool Player::hit_callback(const Collider& target) {
    if (target.tag == ColliderTag::Enemy) {
        --hp_;
        if (hp_ <= 0) {
            // Player defeated
        }
        return true;
    }
    return false;
}
