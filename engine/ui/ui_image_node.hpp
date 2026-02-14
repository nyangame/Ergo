#pragma once
#include "ui_node.hpp"
#include "../resource/texture_handle.hpp"

struct RenderContext;

// ---------------------------------------------------------------------------
// UIImageNode
//
// A UINode that renders a loaded texture.  Supports tint colour, UV rect
// for sprite-sheet regions, and optional aspect-ratio preservation.
// ---------------------------------------------------------------------------
class UIImageNode : public UINode {
public:
    explicit UIImageNode(std::string name = "Image");
    ~UIImageNode() override;

    // Texture
    TextureHandle texture() const { return texture_; }
    void set_texture(TextureHandle tex) { texture_ = tex; }

    // UV rect (normalised)
    Rect uv() const { return uv_; }
    void set_uv(Rect uv) { uv_ = uv; }

    // Tint
    Color tint() const { return tint_; }
    void set_tint(Color c) { tint_ = c; }

    // When true, the node adjusts its rendered size to keep the image's
    // native aspect ratio within the rect transform bounds.
    bool preserve_aspect() const { return preserve_aspect_; }
    void set_preserve_aspect(bool v) { preserve_aspect_ = v; }

    // Source image dimensions (set after loading)
    void set_native_size(uint32_t w, uint32_t h);
    uint32_t native_width() const { return native_w_; }
    uint32_t native_height() const { return native_h_; }

    // Set the rect transform size to the image's native pixel size
    void set_size_to_native();

    void draw(RenderContext& ctx, const WorldRect& parent_rect) override;

private:
    TextureHandle texture_;
    Rect uv_{0.0f, 0.0f, 1.0f, 1.0f};
    Color tint_{255, 255, 255, 255};
    bool preserve_aspect_ = false;
    uint32_t native_w_ = 0;
    uint32_t native_h_ = 0;
};
