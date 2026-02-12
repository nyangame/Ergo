#include "gpu_physics.hpp"
#include "collision3d.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

GpuPhysicsComponent::~GpuPhysicsComponent() {
    release();
}

void GpuPhysicsComponent::start() {
    // Check compute shader availability
    // In a real implementation, this would query VkPhysicalDeviceProperties
    // or check for VK_KHR_compute_shader support
    compute_available_ = false;  // Default to CPU fallback until Vulkan compute is set up
    initialized_ = true;
}

void GpuPhysicsComponent::update(float dt) {
    if (!initialized_) return;

    accumulator_ += dt;
    while (accumulator_ >= fixed_dt_) {
        if (compute_available_) {
            upload_to_gpu();
            dispatch_compute();
            readback_from_gpu();
        } else {
            step_cpu(fixed_dt_);
        }
        accumulator_ -= fixed_dt_;
    }
}

void GpuPhysicsComponent::release() {
    // Release GPU buffers
    body_buffer_ = {};
    shape_buffer_ = {};
    contact_buffer_ = {};
    dispatch_params_ = {};

    body_data_.clear();
    shape_data_.clear();
    body_ids_.clear();
    callbacks_.clear();
    initialized_ = false;
}

uint64_t GpuPhysicsComponent::add_body(Vec3f position, float mass,
                                         CollisionShape3D shape) {
    uint64_t id = next_id_++;

    GpuBodyData bd{};
    bd.pos_x = position.x;
    bd.pos_y = position.y;
    bd.pos_z = position.z;
    bd.inv_mass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
    bd.vel_x = bd.vel_y = bd.vel_z = 0.0f;
    bd.restitution = 0.3f;
    bd.force_x = bd.force_y = bd.force_z = 0.0f;

    GpuShapeData sd{};
    std::visit([&](const auto& s) {
        using S = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<S, SphereShape>) {
            sd.type = 0;
            sd.param0 = s.radius;
        } else if constexpr (std::is_same_v<S, BoxShape>) {
            sd.type = 1;
            sd.param0 = s.half_extent.x;
            sd.param1 = s.half_extent.y;
            sd.param2 = s.half_extent.z;
        } else if constexpr (std::is_same_v<S, PlaneShape>) {
            sd.type = 2;
            sd.param0 = s.normal.x;
            sd.param1 = s.normal.y;
            sd.param2 = s.normal.z;
        }
    }, shape);

    body_data_.push_back(bd);
    shape_data_.push_back(sd);
    body_ids_.push_back(id);

    return id;
}

void GpuPhysicsComponent::remove_body(uint64_t id) {
    for (size_t i = 0; i < body_ids_.size(); ++i) {
        if (body_ids_[i] == id) {
            body_data_.erase(body_data_.begin() + static_cast<ptrdiff_t>(i));
            shape_data_.erase(shape_data_.begin() + static_cast<ptrdiff_t>(i));
            body_ids_.erase(body_ids_.begin() + static_cast<ptrdiff_t>(i));

            // Remove associated callbacks
            callbacks_.erase(
                std::remove_if(callbacks_.begin(), callbacks_.end(),
                    [id](const GpuCollisionCallback& c) { return c.body_id == id; }),
                callbacks_.end()
            );
            return;
        }
    }
}

void GpuPhysicsComponent::set_collision_callback(uint64_t body_id,
    std::function<void(uint64_t other_id, const ContactPoint& contact)> cb) {
    callbacks_.push_back({body_id, std::move(cb)});
}

void GpuPhysicsComponent::apply_force(uint64_t id, Vec3f force) {
    for (size_t i = 0; i < body_ids_.size(); ++i) {
        if (body_ids_[i] == id) {
            body_data_[i].force_x += force.x;
            body_data_[i].force_y += force.y;
            body_data_[i].force_z += force.z;
            return;
        }
    }
}

void GpuPhysicsComponent::apply_impulse(uint64_t id, Vec3f impulse) {
    for (size_t i = 0; i < body_ids_.size(); ++i) {
        if (body_ids_[i] == id) {
            body_data_[i].vel_x += impulse.x * body_data_[i].inv_mass;
            body_data_[i].vel_y += impulse.y * body_data_[i].inv_mass;
            body_data_[i].vel_z += impulse.z * body_data_[i].inv_mass;
            return;
        }
    }
}

Vec3f GpuPhysicsComponent::get_position(uint64_t id) const {
    for (size_t i = 0; i < body_ids_.size(); ++i) {
        if (body_ids_[i] == id) {
            return {body_data_[i].pos_x, body_data_[i].pos_y, body_data_[i].pos_z};
        }
    }
    return Vec3f::zero();
}

Vec3f GpuPhysicsComponent::get_velocity(uint64_t id) const {
    for (size_t i = 0; i < body_ids_.size(); ++i) {
        if (body_ids_[i] == id) {
            return {body_data_[i].vel_x, body_data_[i].vel_y, body_data_[i].vel_z};
        }
    }
    return Vec3f::zero();
}

// --- CPU fallback implementation ---

void GpuPhysicsComponent::step_cpu(float dt) {
    integrate_cpu(dt);
    detect_collisions_cpu();
}

void GpuPhysicsComponent::integrate_cpu(float dt) {
    for (auto& bd : body_data_) {
        if (bd.inv_mass <= 0.0f) continue;

        // Apply gravity
        bd.force_y += gravity_.y / bd.inv_mass;

        // Integration
        bd.vel_x += bd.force_x * bd.inv_mass * dt;
        bd.vel_y += bd.force_y * bd.inv_mass * dt;
        bd.vel_z += bd.force_z * bd.inv_mass * dt;

        bd.pos_x += bd.vel_x * dt;
        bd.pos_y += bd.vel_y * dt;
        bd.pos_z += bd.vel_z * dt;

        // Clear forces
        bd.force_x = bd.force_y = bd.force_z = 0.0f;

        // Simple damping
        bd.vel_x *= 0.99f;
        bd.vel_y *= 0.99f;
        bd.vel_z *= 0.99f;
    }
}

void GpuPhysicsComponent::detect_collisions_cpu() {
    size_t n = body_data_.size();
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            auto& a = body_data_[i];
            auto& b = body_data_[j];

            // Skip if both static
            if (a.inv_mass <= 0.0f && b.inv_mass <= 0.0f) continue;

            // Build transform/shape for collision check
            Transform3D ta, tb;
            ta.position = {a.pos_x, a.pos_y, a.pos_z};
            tb.position = {b.pos_x, b.pos_y, b.pos_z};

            // Reconstruct shapes from GpuShapeData
            auto make_shape = [](const GpuShapeData& sd) -> CollisionShape3D {
                switch (sd.type) {
                    case 0: return SphereShape{sd.param0};
                    case 1: return BoxShape{{sd.param0, sd.param1, sd.param2}};
                    case 2: return PlaneShape{{sd.param0, sd.param1, sd.param2}, 0.0f};
                    default: return SphereShape{0.0f};
                }
            };

            auto shape_a = make_shape(shape_data_[i]);
            auto shape_b = make_shape(shape_data_[j]);

            auto contact = check_collision3d(shape_a, ta, shape_b, tb);
            if (!contact) continue;

            // Resolve collision (impulse-based)
            float inv_mass_sum = a.inv_mass + b.inv_mass;
            if (inv_mass_sum <= 0.0f) continue;

            Vec3f rel_vel = {b.vel_x - a.vel_x, b.vel_y - a.vel_y, b.vel_z - a.vel_z};
            float vel_along_n = rel_vel.dot(contact->normal);
            if (vel_along_n > 0.0f) continue;

            float e = std::min(a.restitution, b.restitution);
            float jn = -(1.0f + e) * vel_along_n / inv_mass_sum;

            Vec3f impulse = contact->normal * jn;
            a.vel_x -= impulse.x * a.inv_mass;
            a.vel_y -= impulse.y * a.inv_mass;
            a.vel_z -= impulse.z * a.inv_mass;
            b.vel_x += impulse.x * b.inv_mass;
            b.vel_y += impulse.y * b.inv_mass;
            b.vel_z += impulse.z * b.inv_mass;

            // Positional correction
            float correction = std::max(contact->penetration - 0.01f, 0.0f) / inv_mass_sum * 0.8f;
            a.pos_x -= contact->normal.x * correction * a.inv_mass;
            a.pos_y -= contact->normal.y * correction * a.inv_mass;
            a.pos_z -= contact->normal.z * correction * a.inv_mass;
            b.pos_x += contact->normal.x * correction * b.inv_mass;
            b.pos_y += contact->normal.y * correction * b.inv_mass;
            b.pos_z += contact->normal.z * correction * b.inv_mass;

            // Fire callbacks
            for (auto& cb : callbacks_) {
                if (cb.body_id == body_ids_[i]) {
                    cb.callback(body_ids_[j], *contact);
                } else if (cb.body_id == body_ids_[j]) {
                    ContactPoint reversed = *contact;
                    reversed.normal = reversed.normal * -1.0f;
                    cb.callback(body_ids_[i], reversed);
                }
            }
        }
    }
}

// --- GPU execution stubs ---

void GpuPhysicsComponent::upload_to_gpu() {
    // TODO: Map GPU buffers and upload body_data_ and shape_data_
    // vkMapMemory -> memcpy -> vkUnmapMemory
}

void GpuPhysicsComponent::dispatch_compute() {
    // TODO: Bind compute pipeline, set descriptor sets, dispatch
    // Stage 1: Integration compute shader
    // Stage 2: Broadphase compute shader (spatial hash / grid)
    // Stage 3: Narrowphase compute shader (contact generation)
    // Stage 4: Constraint solver compute shader
}

void GpuPhysicsComponent::readback_from_gpu() {
    // TODO: Read back resolved positions/velocities from GPU
    // vkMapMemory -> memcpy -> vkUnmapMemory
    // Then fire collision callbacks based on contact buffer
}
