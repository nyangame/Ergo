#pragma once
#include <string_view>
#include <unordered_map>
#include <cstdint>
#include "texture_handle.hpp"

struct GlyphInfo {
    float u0 = 0, v0 = 0, u1 = 0, v1 = 0;  // texture coordinates
    float width = 0, height = 0;              // pixel size
    float bearing_x = 0, bearing_y = 0;
    float advance = 0;
};

struct FontAtlas {
    TextureHandle texture;
    float font_size = 0.0f;
    float line_height = 0.0f;
    float ascent = 0.0f;
    float descent = 0.0f;
    std::unordered_map<uint32_t, GlyphInfo> glyphs;

    const GlyphInfo* get_glyph(uint32_t codepoint) const {
        auto it = glyphs.find(codepoint);
        return (it != glyphs.end()) ? &it->second : nullptr;
    }
};
