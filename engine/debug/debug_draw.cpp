#include "debug_draw.hpp"
#include "system/renderer/vulkan/vk_renderer.hpp"
#include <cmath>
#include <string>

namespace {

struct DrawCmd {
    enum Type { Line2D, Rect2D, Circle2D, Point2D, Text2D, Grid2D };
    Type type;
    Vec2f pos1, pos2;
    Size2f size;
    float radius = 0.0f;
    float spacing = 0.0f;
    int count = 0;
    int segments = 32;
    Color color;
    std::string text;
};

std::vector<DrawCmd> g_commands;

} // anonymous namespace

namespace debug_draw {

void line(Vec2f from, Vec2f to, Color color) {
    DrawCmd cmd;
    cmd.type = DrawCmd::Line2D;
    cmd.pos1 = from;
    cmd.pos2 = to;
    cmd.color = color;
    g_commands.push_back(cmd);
}

void rect_wireframe(Vec2f pos, Size2f size, Color color) {
    DrawCmd cmd;
    cmd.type = DrawCmd::Rect2D;
    cmd.pos1 = pos;
    cmd.size = size;
    cmd.color = color;
    g_commands.push_back(cmd);
}

void circle_wireframe(Vec2f center, float radius, Color color, int segments) {
    DrawCmd cmd;
    cmd.type = DrawCmd::Circle2D;
    cmd.pos1 = center;
    cmd.radius = radius;
    cmd.segments = segments;
    cmd.color = color;
    g_commands.push_back(cmd);
}

void point(Vec2f pos, float size, Color color) {
    DrawCmd cmd;
    cmd.type = DrawCmd::Point2D;
    cmd.pos1 = pos;
    cmd.radius = size;
    cmd.color = color;
    g_commands.push_back(cmd);
}

void line_3d(Vec3f /*from*/, Vec3f /*to*/, Color /*color*/) {
    // Requires camera projection - deferred to renderer integration
}

void aabb_3d(Vec3f /*min*/, Vec3f /*max*/, Color /*color*/) {
    // Requires camera projection
}

void sphere_wireframe(Vec3f /*center*/, float /*radius*/, Color /*color*/, int /*segments*/) {
    // Requires camera projection
}

void text_screen(Vec2f pos, const char* text, Color color) {
    DrawCmd cmd;
    cmd.type = DrawCmd::Text2D;
    cmd.pos1 = pos;
    cmd.color = color;
    cmd.text = text;
    g_commands.push_back(cmd);
}

void grid(Vec2f origin, float spacing, int count, Color color) {
    DrawCmd cmd;
    cmd.type = DrawCmd::Grid2D;
    cmd.pos1 = origin;
    cmd.spacing = spacing;
    cmd.count = count;
    cmd.color = color;
    g_commands.push_back(cmd);
}

void flush(RenderContext& ctx) {
    for (const auto& cmd : g_commands) {
        switch (cmd.type) {
            case DrawCmd::Line2D: {
                // Draw a thin rectangle as a line
                Vec2f d = cmd.pos2 - cmd.pos1;
                float len = d.length();
                if (len > 0.0f) {
                    ctx.draw_rect(cmd.pos1, {len, 1.0f}, cmd.color, true);
                }
                break;
            }
            case DrawCmd::Rect2D:
                ctx.draw_rect(cmd.pos1, cmd.size, cmd.color, false);
                break;
            case DrawCmd::Circle2D:
                ctx.draw_circle(cmd.pos1, cmd.radius, cmd.color, false);
                break;
            case DrawCmd::Point2D:
                ctx.draw_circle(cmd.pos1, cmd.radius, cmd.color, true);
                break;
            case DrawCmd::Text2D:
                ctx.draw_text(cmd.pos1, cmd.text.c_str(), cmd.color, 1.0f);
                break;
            case DrawCmd::Grid2D: {
                float half = cmd.spacing * cmd.count * 0.5f;
                for (int i = -cmd.count; i <= cmd.count; ++i) {
                    float offset = i * cmd.spacing;
                    // Horizontal line
                    ctx.draw_rect(
                        {cmd.pos1.x - half, cmd.pos1.y + offset},
                        {half * 2.0f, 1.0f}, cmd.color, true);
                    // Vertical line
                    ctx.draw_rect(
                        {cmd.pos1.x + offset, cmd.pos1.y - half},
                        {1.0f, half * 2.0f}, cmd.color, true);
                }
                break;
            }
        }
    }
    g_commands.clear();
}

void clear() {
    g_commands.clear();
}

bool has_pending() {
    return !g_commands.empty();
}

} // namespace debug_draw
