#pragma once
#include "../math/vec2.hpp"
#include "../math/vec3.hpp"
#include "../math/color.hpp"
#include "../math/size2.hpp"
#include <vector>

struct RenderContext;

namespace debug_draw {

// 2D primitives
void line(Vec2f from, Vec2f to, Color color);
void rect_wireframe(Vec2f pos, Size2f size, Color color);
void circle_wireframe(Vec2f center, float radius, Color color, int segments = 32);
void point(Vec2f pos, float size, Color color);

// 3D primitives (projected via current camera)
void line_3d(Vec3f from, Vec3f to, Color color);
void aabb_3d(Vec3f min, Vec3f max, Color color);
void sphere_wireframe(Vec3f center, float radius, Color color, int segments = 16);

// Text
void text_screen(Vec2f pos, const char* text, Color color);

// Grid
void grid(Vec2f origin, float spacing, int count, Color color);

// Flush all accumulated draw commands and clear
void flush(RenderContext& ctx);

// Clear without drawing
void clear();

// Check if there are pending draw commands
bool has_pending();

} // namespace debug_draw
