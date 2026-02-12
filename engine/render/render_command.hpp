#pragma once
#include "../math/vec3.hpp"
#include "../math/mat4.hpp"
#include "../math/color.hpp"
#include "../resource/texture_handle.hpp"
#include <cstdint>
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

using RenderCommand = std::variant<
    RenderCmd_Clear,
    RenderCmd_SetViewProjection,
    RenderCmd_DrawMesh,
    RenderCmd_DrawRect,
    RenderCmd_DrawCircle,
    RenderCmd_DrawSprite,
    RenderCmd_DrawText,
    RenderCmd_DrawDebugLine
>;
