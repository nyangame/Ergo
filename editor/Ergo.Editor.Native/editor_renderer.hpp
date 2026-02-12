#pragma once
#include <cstdint>
#include <memory>

// Forward-declared opaque handle for render targets
struct EditorRenderTarget;

/// Camera parameters for a single render target.
struct EditorCamera {
    float eye[3]    = {0.0f, 5.0f, -10.0f};
    float target[3] = {0.0f, 0.0f, 0.0f};
    float up[3]     = {0.0f, 1.0f, 0.0f};
    float fov_degrees  = 60.0f;
    float near_plane   = 0.1f;
    float far_plane    = 1000.0f;
};

/// Determines which overlays are drawn.
enum class RenderMode : uint32_t {
    Scene = 0, // Grid, gizmos, selection outlines
    Game  = 1, // Clean game view
};

// ===================================================================
// EditorRenderer
//
// Manages the graphics device and zero-or-more render targets.
// Each render target is bound to one native window handle (HWND on
// Windows, X11 Window on Linux) and owns its own surface / swapchain.
//
// Thread safety: all public methods must be called from the same
// thread (the UI / render-pump thread).
// ===================================================================
class EditorRenderer {
public:
    EditorRenderer();
    ~EditorRenderer();

    // Non-copyable
    EditorRenderer(const EditorRenderer&) = delete;
    EditorRenderer& operator=(const EditorRenderer&) = delete;

    /// Initialize the graphics device.
    /// Returns true on success.
    bool initialize();

    /// Tear down everything (all render targets + device).
    void shutdown();

    bool is_initialized() const;

    // ---------------------------------------------------------------
    // Render target management
    // ---------------------------------------------------------------

    /// Create a render target for the given native surface handle.
    /// Returns a non-zero handle on success, 0 on failure.
    uint64_t create_render_target(void* native_window_handle,
                                  uint32_t width, uint32_t height,
                                  RenderMode mode);

    void destroy_render_target(uint64_t id);

    void resize_render_target(uint64_t id,
                              uint32_t width, uint32_t height);

    // ---------------------------------------------------------------
    // Per-target camera
    // ---------------------------------------------------------------

    void set_camera(uint64_t id, const EditorCamera& camera);

    // ---------------------------------------------------------------
    // Rendering
    // ---------------------------------------------------------------

    /// Render one frame to the given target (acquire, record, submit,
    /// present). Returns true if the frame was presented.
    bool render_frame(uint64_t id);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
