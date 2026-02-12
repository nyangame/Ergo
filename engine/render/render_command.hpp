#pragma once
#include "../math/vec3.hpp"
#include "../math/mat4.hpp"
#include "../math/color.hpp"
#include "../resource/texture_handle.hpp"
#include <cstdint>
#include <vector>
#include <variant>

// Render commands: recorded by game threads, consumed by render thread

struct RenderCmd_Clear {
    Color color{0, 0, 0, 255};
    float depth = 1.0f;
};

struct RenderCmd_SetViewProjection {
    Mat4 view;
    Mat4 projection;
};

struct RenderCmd_DrawMesh {
    uint64_t mesh_id = 0;
    Mat4 world_transform;
    uint64_t material_id = 0;
};

struct RenderCmd_DrawRect {
    Vec3f position;
    float width = 0.0f;
    float height = 0.0f;
    Color color;
    bool filled = true;
};

struct RenderCmd_DrawCircle {
    Vec3f center;
    float radius = 0.0f;
    Color color;
    bool filled = true;
};

struct RenderCmd_DrawSprite {
    Vec3f position;
    float width = 0.0f;
    float height = 0.0f;
    TextureHandle texture;
    Rect uv;
};

struct RenderCmd_DrawText {
    Vec3f position;
    char text[256] = {};
    Color color;
    float scale = 1.0f;
};

struct RenderCmd_DrawDebugLine {
    Vec3f from;
    Vec3f to;
    Color color;
};

// SDF/MSDFテキスト描画コマンド (バッチ描画)
// TextRenderer::build_batches() の出力をレンダーパイプラインに投入する
struct TextBatchVertex {
    float pos_x, pos_y, pos_z;
    float uv_x, uv_y;
    uint8_t r, g, b, a;
};

struct RenderCmd_DrawTextBatch {
    Vec3f origin;
    uint64_t font_atlas_texture = 0;  // アトラスページのテクスチャID
    uint32_t render_mode = 0;         // FontRenderMode (SDF/MSDF等)
    float sdf_pixel_range = 4.0f;     // SDF距離レンジ
    // スタイルパラメータ (SDF/MSDFシェーダー用)
    float outline_width = 0.0f;
    Color outline_color{0, 0, 0, 255};
    float shadow_offset_x = 0.0f;
    float shadow_offset_y = 0.0f;
    float shadow_softness = 0.0f;
    Color shadow_color{0, 0, 0, 128};
    float face_dilate = 0.0f;
    float face_softness = 0.0f;
    // 頂点データ
    std::vector<TextBatchVertex> vertices;
    std::vector<uint32_t> indices;
};

using RenderCommand = std::variant<
    RenderCmd_Clear,
    RenderCmd_SetViewProjection,
    RenderCmd_DrawMesh,
    RenderCmd_DrawRect,
    RenderCmd_DrawCircle,
    RenderCmd_DrawSprite,
    RenderCmd_DrawText,
    RenderCmd_DrawDebugLine,
    RenderCmd_DrawTextBatch
>;
