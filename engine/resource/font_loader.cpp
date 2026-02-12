#include "font_loader.hpp"
#include <fstream>
#include <vector>
#include <algorithm>

// stb_truetype integration:
// To enable, place stb_truetype.h in engine/third_party/ and define ERGO_HAS_STB_TRUETYPE=1
#ifdef ERGO_HAS_STB_TRUETYPE
#define STB_TRUETYPE_IMPLEMENTATION
#include "../third_party/stb_truetype.h"
#endif

FontLoader::LoadResult FontLoader::load(
    std::string_view ttf_path,
    float font_size,
    const std::vector<std::pair<uint32_t, uint32_t>>& char_ranges)
{
    LoadResult result;

#ifdef ERGO_HAS_STB_TRUETYPE
    // Read TTF file
    std::ifstream file(std::string(ttf_path), std::ios::binary | std::ios::ate);
    if (!file.is_open()) return result;

    size_t file_size = static_cast<size_t>(file.tellg());
    file.seekg(0);
    std::vector<uint8_t> ttf_data(file_size);
    file.read(reinterpret_cast<char*>(ttf_data.data()), file_size);

    stbtt_fontinfo font_info;
    if (!stbtt_InitFont(&font_info, ttf_data.data(), 0)) return result;

    float scale = stbtt_ScaleForPixelHeight(&font_info, font_size);

    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);

    result.atlas.font_size = font_size;
    result.atlas.ascent = ascent * scale;
    result.atlas.descent = descent * scale;
    result.atlas.line_height = (ascent - descent + line_gap) * scale;

    // Count total glyphs for atlas sizing
    size_t total_glyphs = 0;
    for (auto& [first, last] : char_ranges) {
        total_glyphs += (last - first + 1);
    }

    // Estimate atlas size
    int glyph_dim = static_cast<int>(font_size) + 2;
    int cols = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(total_glyphs))));
    uint32_t atlas_w = static_cast<uint32_t>(cols * glyph_dim);
    uint32_t atlas_h = atlas_w;
    // Round up to power of 2
    atlas_w = std::max(atlas_w, 256u);
    atlas_h = std::max(atlas_h, 256u);

    result.image.width = atlas_w;
    result.image.height = atlas_h;
    result.image.pixels.resize(atlas_w * atlas_h, 0);

    int pen_x = 0, pen_y = 0;
    int row_height = glyph_dim;

    for (auto& [first, last] : char_ranges) {
        for (uint32_t cp = first; cp <= last; ++cp) {
            int glyph_index = stbtt_FindGlyphIndex(&font_info, cp);
            if (glyph_index == 0 && cp != ' ') continue;

            int x0, y0, x1, y1;
            stbtt_GetGlyphBitmapBox(&font_info, glyph_index, scale, scale,
                                     &x0, &y0, &x1, &y1);

            int gw = x1 - x0;
            int gh = y1 - y0;

            if (pen_x + gw + 1 >= static_cast<int>(atlas_w)) {
                pen_x = 0;
                pen_y += row_height;
            }

            if (pen_y + gh >= static_cast<int>(atlas_h)) break;

            stbtt_MakeGlyphBitmap(&font_info,
                result.image.pixels.data() + pen_x + pen_y * atlas_w,
                gw, gh, atlas_w, scale, scale, glyph_index);

            int advance, lsb;
            stbtt_GetGlyphHMetrics(&font_info, glyph_index, &advance, &lsb);

            GlyphInfo gi;
            gi.u0 = static_cast<float>(pen_x) / atlas_w;
            gi.v0 = static_cast<float>(pen_y) / atlas_h;
            gi.u1 = static_cast<float>(pen_x + gw) / atlas_w;
            gi.v1 = static_cast<float>(pen_y + gh) / atlas_h;
            gi.width = static_cast<float>(gw);
            gi.height = static_cast<float>(gh);
            gi.bearing_x = static_cast<float>(x0);
            gi.bearing_y = static_cast<float>(-y0);
            gi.advance = advance * scale;

            result.atlas.glyphs[cp] = gi;

            pen_x += gw + 1;
        }
    }

    result.valid = true;
#else
    (void)ttf_path;
    (void)font_size;
    (void)char_ranges;
#endif

    return result;
}
