#include "demo_framework.hpp"
#include "engine/text/text_layout.hpp"
#include "engine/text/rich_text.hpp"
#include "engine/text/font_atlas.hpp"
#include "engine/text/glyph.hpp"
#include "engine/text/text_style.hpp"
#include <cstdio>

DEMO(Text_TextLayoutConfig) {
    TextLayoutConfig config;
    config.max_width = 400.0f;
    config.line_spacing = 1.5f;
    config.align = TextAlign::Center;
    config.vertical_align = TextVerticalAlign::Middle;
    config.direction = TextDirection::LeftToRight;
    config.overflow = TextOverflow::Ellipsis;

    std::printf("  Layout config:\n");
    std::printf("    max_width: %.1f\n", config.max_width);
    std::printf("    line_spacing: %.1f\n", config.line_spacing);
    std::printf("    align: %d (Center)\n", static_cast<int>(config.align));
    std::printf("    overflow: %d (Ellipsis)\n", static_cast<int>(config.overflow));
}

DEMO(Text_GlyphMetrics) {
    GlyphMetrics metrics;
    metrics.advance = 12.0f;
    metrics.bearing_x = 1.0f;
    metrics.bearing_y = 10.0f;
    metrics.width = 10.0f;
    metrics.height = 12.0f;

    GlyphAtlasRegion region;
    region.atlas_index = 0;
    region.u0 = 0.0f; region.v0 = 0.0f;
    region.u1 = 0.1f; region.v1 = 0.12f;

    Glyph glyph;
    glyph.codepoint = U'A';
    glyph.metrics = metrics;
    glyph.atlas = region;

    std::printf("  Glyph 'A': advance=%.1f bearing=(%.1f,%.1f) size=(%.1f,%.1f)\n",
                glyph.metrics.advance, glyph.metrics.bearing_x, glyph.metrics.bearing_y,
                glyph.metrics.width, glyph.metrics.height);
    std::printf("  Atlas region: atlas_index=%u uv=(%.2f,%.2f)-(%.2f,%.2f)\n",
                glyph.atlas.atlas_index, glyph.atlas.u0, glyph.atlas.v0,
                glyph.atlas.u1, glyph.atlas.v1);
}

DEMO(Text_FontAtlasConfig) {
    FontAtlas atlas;
    atlas.atlas_width = 1024;
    atlas.atlas_height = 1024;
    atlas.render_mode = FontRenderMode::MSDF;
    atlas.sdf_pixel_range = 4.0f;
    atlas.population_mode = AtlasPopulationMode::Dynamic;

    std::printf("  Font atlas: %ux%u\n", atlas.atlas_width, atlas.atlas_height);
    std::printf("  Render mode: %d (MSDF)\n", static_cast<int>(atlas.render_mode));
    std::printf("  SDF range: %.1f\n", atlas.sdf_pixel_range);
    std::printf("  Population: %d (Dynamic)\n", static_cast<int>(atlas.population_mode));
    std::printf("  Pages: %zu\n", atlas.pages.size());
}

DEMO(Text_RichTextParsing) {
    RichText rt;
    rt.set_text("Hello <color=#FF0000>red</color> world <size=24>big</size> text");

    // Force parse by marking dirty and calling parse_markup directly
    auto parsed = RichText::parse_markup(rt.source_text, {255,255,255,255}, 16.0f);
    rt.segments = parsed;

    std::printf("  Rich text segments: %zu\n", rt.segments.size());
    for (const auto& seg : rt.segments) {
        std::printf("    '%s' color=(%d,%d,%d,%d) size=%.0f\n",
                    seg.text.c_str(),
                    seg.color.r, seg.color.g, seg.color.b, seg.color.a,
                    seg.font_size);
    }
}

DEMO(Text_Style) {
    TextStyle style;
    style.face_color = {255, 255, 255, 255};
    style.outline_width = 2.0f;
    style.outline_color = {0, 0, 0, 255};
    style.shadow_offset_x = 1.5f;
    style.shadow_offset_y = 1.5f;
    style.shadow_softness = 2.0f;
    style.shadow_color = {0, 0, 0, 128};
    style.face_dilate = 0.0f;
    style.face_softness = 0.0f;
    style.decoration = TextDecoration::Underline;

    std::printf("  Text style:\n");
    std::printf("    Face: (%d,%d,%d,%d)\n",
                style.face_color.r, style.face_color.g,
                style.face_color.b, style.face_color.a);
    std::printf("    Outline: width=%.1f color=(%d,%d,%d)\n",
                style.outline_width,
                style.outline_color.r, style.outline_color.g, style.outline_color.b);
    std::printf("    Shadow: offset=(%.1f,%.1f) softness=%.1f\n",
                style.shadow_offset_x, style.shadow_offset_y, style.shadow_softness);
    std::printf("    Decoration: %d (Underline)\n", static_cast<int>(style.decoration));
}
