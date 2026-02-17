#pragma once

#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>

// Forward declarations
struct Transform2D;
struct Transform3D;
struct Vec2f;
struct Vec3f;
struct RenderContext;
struct CommandBuffer;

// ============================================================
// Threading policy: annotates how a type can be executed
// ============================================================

enum class ThreadingPolicy : uint8_t {
    MainThread,    // Must run on main thread (input, UI, render state, etc.)
    AnyThread,     // Safe to call from any single thread (no shared mutable state)
    Parallel,      // Designed for parallel execution across worker threads (data-oriented)
};

// ThreadAware: a type that declares its threading policy
template<typename T>
concept ThreadAware = requires {
    { T::threading_policy() } -> std::same_as<ThreadingPolicy>;
};

// ============================================================
// Lifecycle concepts
// ============================================================

template<typename T>
concept Startable = requires(T t) {
    { t.start() } -> std::same_as<void>;
};

template<typename T>
concept Updatable = requires(T t, float dt) {
    { t.update(dt) } -> std::same_as<void>;
};

template<typename T, typename Ctx = RenderContext>
concept Drawable = requires(T t, Ctx& ctx) {
    { t.draw(ctx) } -> std::same_as<void>;
};

template<typename T>
concept Releasable = requires(T t) {
    { t.release() } -> std::same_as<void>;
};

// ============================================================
// Task concept (enforced at registration)
// ============================================================

template<typename T>
concept TaskLike = Startable<T> && Updatable<T> && Releasable<T>;

// ============================================================
// Physics concepts
// ============================================================

// PhysicsSteppable: a physics component that can be stepped each frame
template<typename T>
concept PhysicsSteppable = requires(T t, float dt) {
    { t.start() } -> std::same_as<void>;
    { t.update(dt) } -> std::same_as<void>;
    { t.release() } -> std::same_as<void>;
};

// PhysicsBodyProvider: provides access to physics bodies by ID
template<typename T>
concept PhysicsBodyProvider = requires(T t, uint64_t id) {
    { t.body_count() } -> std::convertible_to<size_t>;
    { t.remove_body(id) } -> std::same_as<void>;
};

// PhysicsForceApplicable: can apply forces/impulses
template<typename T>
concept PhysicsForceApplicable = requires(T t, uint64_t id, Vec3f v) {
    { t.apply_force(id, v) } -> std::same_as<void>;
    { t.apply_impulse(id, v) } -> std::same_as<void>;
};

// ============================================================
// GameObject concepts
// ============================================================

template<typename T>
concept GameObjectLike = requires(T t) {
    { t.transform() } -> std::same_as<Transform2D&>;
    { t.name() } -> std::convertible_to<std::string_view>;
    { t.object_type() } -> std::same_as<uint32_t>;
};

template<typename T>
concept ColliderLike = requires(T a, T b) {
    { a.is_hit(b) } -> std::same_as<bool>;
    { a.tag() } -> std::same_as<uint32_t>;
    { a.owner_transform() } -> std::same_as<const Transform2D&>;
};

// ============================================================
// Behaviour concepts (composable units of object logic)
// ============================================================

template<typename T>
concept BehaviourLike = Startable<T> && Updatable<T> && Releasable<T> &&
    requires {
        { T::type_name() } -> std::convertible_to<std::string_view>;
    };

// ============================================================
// System backend concepts
// ============================================================

template<typename T>
concept RendererBackend = requires(T t) {
    { t.initialize() } -> std::same_as<bool>;
    { t.begin_frame() } -> std::same_as<void>;
    { t.end_frame() } -> std::same_as<void>;
    { t.shutdown() } -> std::same_as<void>;
};

template<typename T>
concept InputBackend = requires(T t, uint32_t key) {
    { t.is_key_down(key) } -> std::same_as<bool>;
    { t.is_key_pressed(key) } -> std::same_as<bool>;
    { t.mouse_position() } -> std::same_as<Vec2f>;
    { t.poll_events() } -> std::same_as<void>;
};

template<typename T>
concept WindowBackend = requires(T t, uint32_t w, uint32_t h, std::string_view title) {
    { t.create(w, h, title) } -> std::same_as<bool>;
    { t.should_close() } -> std::same_as<bool>;
    { t.poll_events() } -> std::same_as<void>;
    { t.width() } -> std::same_as<uint32_t>;
    { t.height() } -> std::same_as<uint32_t>;
};

// ============================================================
// Render pipeline concepts
// ============================================================

// CommandSubmittable: can submit render commands to a command buffer
template<typename T>
concept CommandSubmittable = requires(T t, CommandBuffer& buf) {
    { t.record_commands(buf) } -> std::same_as<void>;
};

// ============================================================
// Shader composition concepts
// ============================================================

// Forward declarations
class ShaderGraph;
class ShaderMaterial;

// ShaderComposable: a material that can compile from a node graph
template<typename T>
concept ShaderComposable = requires(T t) {
    { t.compile() } -> std::same_as<bool>;
    { t.vertex_source() } -> std::convertible_to<std::string_view>;
    { t.fragment_source() } -> std::convertible_to<std::string_view>;
    { t.is_compiled() } -> std::same_as<bool>;
};

// ShaderOptimizable: can be optimized by the shader optimizer
template<typename T>
concept ShaderOptimizable = requires(T t) {
    { t.optimization_report() } -> std::convertible_to<std::string>;
    { t.is_optimized() } -> std::same_as<bool>;
};

// ============================================================
// Network concepts (see engine/net/net_concepts.hpp for full set)
// ============================================================

// NetworkPollable: can be polled each frame for network events
template<typename T>
concept NetworkPollable = requires(T t) {
    { t.poll() } -> std::same_as<void>;
    { t.shutdown() } -> std::same_as<void>;
    { t.is_active() } -> std::same_as<bool>;
};
