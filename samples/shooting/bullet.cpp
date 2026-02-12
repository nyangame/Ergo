#include "bullet.hpp"
#include "engine/physics/physics_system.hpp"
#include "system/renderer/vulkan/vk_renderer.hpp"

void Bullet::start() {
    object.object_type_ = static_cast<uint32_t>(GameObjectType::Bullet);
    object.name_ = "Bullet";
    alive_ = true;
    object.transform_.size = {8.0f, 8.0f};

    // Setup circle collider
    collider.shape = CircleData{4.0f};
    collider.tag = ColliderTag::Bullet;
    collider.transform = &object.transform_;
    collider.on_hit = [this](const Collider& target) {
        return hit_callback(target);
    };

    g_physics.register_collider(collider);
}

void Bullet::update(float dt) {
    if (!alive_) return;

    object.transform_.position += velocity_;

    // Off-screen check
    if (object.transform_.position.x < -100.0f || object.transform_.position.x > 900.0f ||
        object.transform_.position.y < -100.0f || object.transform_.position.y > 700.0f) {
        alive_ = false;
    }

    g_physics.mark_moved(collider);
}

void Bullet::draw(RenderContext& ctx) {
    if (!alive_) return;
    auto& t = object.transform_;
    // Draw bullet as a yellow circle
    ctx.draw_circle(t.position, 4.0f, Color{255, 255, 64, 255}, true);
}

void Bullet::release() {
    g_physics.remove_collider(collider);
}

bool Bullet::hit_callback(const Collider& target) {
    if (target.tag == ColliderTag::Enemy) {
        alive_ = false;
        return true;
    }
    return false;
}
