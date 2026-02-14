#pragma once
#include "ui_node.hpp"

// ---------------------------------------------------------------------------
// CanvasScaleMode
//
// Controls how the UI canvas maps to the screen.
//
//   DotByDot          : 1 UI pixel == 1 screen pixel.  No scaling applied.
//   ScaleWithScreen   : The canvas is uniformly scaled so that the
//                       reference resolution maps to the current screen.
//                       ScreenMatchMode selects the axis to match.
// ---------------------------------------------------------------------------
enum class CanvasScaleMode : uint8_t {
    DotByDot,
    ScaleWithScreen,
};

enum class ScreenMatchMode : uint8_t {
    MatchWidth,
    MatchHeight,
    MatchMinAxis,
    MatchMaxAxis,
};

// ---------------------------------------------------------------------------
// UICanvas
//
// Root of a UI hierarchy.  Every UI tree begins with exactly one UICanvas.
// The canvas is responsible for:
//   - Determining the effective scale factor.
//   - Providing the root WorldRect for child layout.
//   - Driving update / draw recursion.
// ---------------------------------------------------------------------------
class UICanvas : public UINode {
public:
    explicit UICanvas(std::string name = "Canvas");
    ~UICanvas() override;

    // Scale mode
    CanvasScaleMode scale_mode() const { return scale_mode_; }
    void set_scale_mode(CanvasScaleMode mode) { scale_mode_ = mode; }

    // Reference resolution (used when ScaleWithScreen)
    Size2f reference_resolution() const { return reference_resolution_; }
    void set_reference_resolution(Size2f res) { reference_resolution_ = res; }

    // Screen match
    ScreenMatchMode screen_match_mode() const { return screen_match_mode_; }
    void set_screen_match_mode(ScreenMatchMode mode) { screen_match_mode_ = mode; }

    // Current screen size (set each frame before update/draw)
    void set_screen_size(float w, float h);
    Size2f screen_size() const { return screen_size_; }

    // Computed scale factor (valid after set_screen_size)
    float scale_factor() const { return scale_factor_; }

    // The logical canvas size after scaling (screen_size / scale_factor)
    Size2f canvas_size() const;

    // Root WorldRect for child layout
    WorldRect root_rect() const;

    // Update & draw the entire canvas tree
    void update_all(float dt);
    void draw_all(RenderContext& ctx);

    // Hit test from screen coordinates
    UINode* hit_test_screen(Vec2f screen_pos);

private:
    CanvasScaleMode scale_mode_ = CanvasScaleMode::DotByDot;
    ScreenMatchMode screen_match_mode_ = ScreenMatchMode::MatchMinAxis;
    Size2f reference_resolution_{1920.0f, 1080.0f};
    Size2f screen_size_{1920.0f, 1080.0f};
    float scale_factor_ = 1.0f;

    void recalculate_scale();
};
