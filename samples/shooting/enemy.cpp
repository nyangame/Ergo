#include "enemy.hpp"
#include "engine/physics/physics_system.hpp"
#include "system/renderer/vulkan/vk_renderer.hpp"

void Enemy::start() {
    object.object_type_ = static_cast<uint32_t>(GameObjectType::Enemy);
    object.name_ = "Enemy";
    hp_ = 3;
    alive_ = true;
    object.transform_.size = {40.0f, 40.0f};

    // Setup circle collider
    collider.shape = CircleData{20.0f};
    collider.tag = ColliderTag::Enemy;
    collider.transform = &object.transform_;
    collider.on_hit = [this](const Collider& target) {
        return hit_callback(target);
    };

    g_physics.register_collider(collider);
}

void Enemy::update(float dt) {
    if (!alive_) return;

    // Move left
    object.transform_.position.x -= speed_;

    // Mark as moved for physics
    g_physics.mark_moved(collider);
}

void Enemy::draw(RenderContext& ctx) {
    if (!alive_) return;
    auto& t = object.transform_;
    // Draw enemy as a red circle
    ctx.draw_circle(t.position, 20.0f, Color{255, 64, 64, 255}, true);
}

void Enemy::release() {
    g_physics.remove_collider(collider);
}

bool Enemy::hit_callback(const Collider& target) {
    if (target.tag == ColliderTag::Bullet) {
        --hp_;
        if (hp_ <= 0) {
            alive_ = false;
        }
        return true;
    }
    return false;
}
