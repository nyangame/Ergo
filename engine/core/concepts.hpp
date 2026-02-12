#pragma once

#include <concepts>
#include <cstdint>
#include <string_view>

// Forward declarations
struct Transform2D;
struct Vec2f;
struct RenderContext;

// --- Lifecycle ---
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

// --- Task (CppSampleGame TaskBase equivalent) ---
template<typename T>
concept TaskLike = Startable<T> && Updatable<T> && Releasable<T>;

// --- GameObject (CppSampleGame IGameObject equivalent) ---
template<typename T>
concept GameObjectLike = requires(T t) {
    { t.transform() } -> std::same_as<Transform2D&>;
    { t.name() } -> std::convertible_to<std::string_view>;
    { t.object_type() } -> std::same_as<uint32_t>;
};

// --- Collider (CppSampleGame Collider2D equivalent) ---
template<typename T>
concept ColliderLike = requires(T a, T b) {
    { a.is_hit(b) } -> std::same_as<bool>;
    { a.tag() } -> std::same_as<uint32_t>;
    { a.owner_transform() } -> std::same_as<const Transform2D&>;
};

// --- Renderer Backend ---
template<typename T>
concept RendererBackend = requires(T t) {
    { t.initialize() } -> std::same_as<bool>;
    { t.begin_frame() } -> std::same_as<void>;
    { t.end_frame() } -> std::same_as<void>;
    { t.shutdown() } -> std::same_as<void>;
};

// --- Input System ---
template<typename T>
concept InputBackend = requires(T t, uint32_t key) {
    { t.is_key_down(key) } -> std::same_as<bool>;
    { t.is_key_pressed(key) } -> std::same_as<bool>;
    { t.mouse_position() } -> std::same_as<Vec2f>;
    { t.poll_events() } -> std::same_as<void>;
};

// --- Window ---
template<typename T>
concept WindowBackend = requires(T t, uint32_t w, uint32_t h, std::string_view title) {
    { t.create(w, h, title) } -> std::same_as<bool>;
    { t.should_close() } -> std::same_as<bool>;
    { t.poll_events() } -> std::same_as<void>;
    { t.width() } -> std::same_as<uint32_t>;
    { t.height() } -> std::same_as<uint32_t>;
};
