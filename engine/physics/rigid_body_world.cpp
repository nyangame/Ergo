#include "rigid_body_world.hpp"
#include "collision3d.hpp"
#include <algorithm>
#include <cmath>

uint64_t RigidBodyWorld::add_body(PhysicsBody body) {
    body.id = next_id_++;
    body.body.transform = &body.transform;
    body.body.set_mass(body.body.mass); // Ensure inv_mass is computed
    bodies_.push_back(std::move(body));
    // Fix transform pointer after move
    bodies_.back().body.transform = &bodies_.back().transform;
    return bodies_.back().id;
}

void RigidBodyWorld::remove_body(uint64_t id) {
    bodies_.erase(
        std::remove_if(bodies_.begin(), bodies_.end(),
            [id](const PhysicsBody& b) { return b.id == id; }),
        bodies_.end()
    );
}

PhysicsBody* RigidBodyWorld::get_body(uint64_t id) {
    for (auto& b : bodies_) {
        if (b.id == id) return &b;
    }
    return nullptr;
}

const PhysicsBody* RigidBodyWorld::get_body(uint64_t id) const {
    for (auto& b : bodies_) {
        if (b.id == id) return &b;
    }
    return nullptr;
}

void RigidBodyWorld::integrate(float dt) {
    for (auto& pb : bodies_) {
        auto& body = pb.body;
        if (body.type == RigidBodyType::Static) continue;
        if (body.is_sleeping) continue;

        // Apply gravity
        Vec3f gravity_force = gravity_ * body.mass * body.gravity_scale;
        body.force_accumulator += gravity_force;

        // Semi-implicit Euler integration (linear)
        Vec3f accel = body.force_accumulator * body.inv_mass;
        body.velocity += accel * dt;

        // Linear damping
        body.velocity *= (1.0f - body.linear_damping);

        // Update position
        pb.transform.position += body.velocity * dt;

        // Angular integration
        body.angular_velocity += body.torque_accumulator * body.inv_mass * dt;
        body.angular_velocity *= (1.0f - body.angular_damping);

        // Update rotation from angular velocity
        float angle = body.angular_velocity.length();
        if (angle > 0.0001f) {
            Vec3f axis = body.angular_velocity.normalized();
            Quat delta = Quat::from_axis_angle(axis, angle * dt);
            pb.transform.rotation = (delta * pb.transform.rotation).normalized();
        }

        body.clear_forces();
    }
}

void RigidBodyWorld::detect_and_resolve() {
    for (size_t i = 0; i < bodies_.size(); ++i) {
        for (size_t j = i + 1; j < bodies_.size(); ++j) {
            auto& a = bodies_[i];
            auto& b = bodies_[j];

            // Skip if both static
            if (a.body.type == RigidBodyType::Static &&
                b.body.type == RigidBodyType::Static) continue;

            // Skip if both sleeping
            if (a.body.is_sleeping && b.body.is_sleeping) continue;

            auto contact = check_collision3d(a.shape, a.transform, b.shape, b.transform);
            if (!contact) continue;

            // Wake up sleeping bodies on collision
            a.body.wake();
            b.body.wake();

            // Resolve collision
            resolve_contact(a, b, *contact);

            // Fire callbacks
            if (a.on_collision) a.on_collision(b, *contact);
            if (b.on_collision) {
                ContactPoint reversed = *contact;
                reversed.normal = reversed.normal * -1.0f;
                b.on_collision(a, reversed);
            }
        }
    }
}

void RigidBodyWorld::resolve_contact(PhysicsBody& a, PhysicsBody& b,
                                      const ContactPoint& contact) {
    float inv_mass_sum = a.body.inv_mass + b.body.inv_mass;
    if (inv_mass_sum <= 0.0f) return;

    // Positional correction (prevent sinking)
    constexpr float correction_percent = 0.8f;
    constexpr float slop = 0.01f;
    float correction_mag = std::max(contact.penetration - slop, 0.0f) /
                           inv_mass_sum * correction_percent;
    Vec3f correction = contact.normal * correction_mag;
    a.transform.position -= correction * a.body.inv_mass;
    b.transform.position += correction * b.body.inv_mass;

    // Relative velocity
    Vec3f rel_vel = b.body.velocity - a.body.velocity;
    float vel_along_normal = rel_vel.dot(contact.normal);

    // Don't resolve if velocities are separating
    if (vel_along_normal > 0.0f) return;

    // Restitution (use minimum)
    float e = std::min(a.body.restitution, b.body.restitution);

    // Impulse magnitude
    float j = -(1.0f + e) * vel_along_normal / inv_mass_sum;

    // Apply impulse
    Vec3f impulse = contact.normal * j;
    a.body.velocity -= impulse * a.body.inv_mass;
    b.body.velocity += impulse * b.body.inv_mass;

    // Friction impulse (tangent)
    Vec3f tangent = rel_vel - contact.normal * vel_along_normal;
    float tangent_len = tangent.length();
    if (tangent_len > 0.0001f) {
        tangent = tangent * (1.0f / tangent_len);
        float jt = -rel_vel.dot(tangent) / inv_mass_sum;
        float mu = (a.body.friction + b.body.friction) * 0.5f;

        Vec3f friction_impulse;
        if (std::abs(jt) < j * mu) {
            friction_impulse = tangent * jt;
        } else {
            friction_impulse = tangent * (-j * mu);
        }
        a.body.velocity -= friction_impulse * a.body.inv_mass;
        b.body.velocity += friction_impulse * b.body.inv_mass;
    }
}

void RigidBodyWorld::update_sleep(float dt) {
    for (auto& pb : bodies_) {
        auto& body = pb.body;
        if (body.type == RigidBodyType::Static) continue;

        float speed = body.velocity.length_sq() + body.angular_velocity.length_sq();
        if (speed < sleep_velocity_threshold_ * sleep_velocity_threshold_) {
            body.sleep_timer += dt;
            if (body.sleep_timer >= sleep_time_threshold_) {
                body.is_sleeping = true;
                body.velocity = Vec3f::zero();
                body.angular_velocity = Vec3f::zero();
            }
        } else {
            body.sleep_timer = 0.0f;
            body.is_sleeping = false;
        }
    }
}

void RigidBodyWorld::step(float dt) {
    accumulator_ += dt;

    int steps = 0;
    while (accumulator_ >= fixed_dt_ && steps < max_substeps_) {
        integrate(fixed_dt_);
        detect_and_resolve();
        update_sleep(fixed_dt_);
        accumulator_ -= fixed_dt_;
        ++steps;
    }

    // Clamp accumulator to prevent spiral of death
    if (accumulator_ > fixed_dt_ * 2.0f) {
        accumulator_ = 0.0f;
    }
}
