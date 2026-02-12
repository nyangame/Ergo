#include "text_renderer.hpp"
#include "simple_text.hpp"
#include "rich_text.hpp"
#include <cmath>

// ---------------------------------------------------------------
// グリフクワッド生成
// ---------------------------------------------------------------

void TextRenderer::emit_glyph_quad(
    TextDrawBatch& batch,
    const PlacedGlyph& pg,
    const FontAsset& font,
    const TextStyle& style,
    Vec3f origin)
{
    if (!pg.glyph) return;

    const auto& g = *pg.glyph;
    const auto& m = g.metrics;
    const float scale = pg.scale;

    // グリフの四角形の位置を計算
    // ベアリングを考慮してベースラインからの位置を決定
    float x0 = origin.x + pg.position.x + m.bearing_x * scale;
    float y0 = origin.y + pg.position.y - m.bearing_y * scale;
    float x1 = x0 + m.width * scale;
    float y1 = y0 + m.height * scale;
    float z = origin.z;

    // 斜体: 上部を水平にシアー変換
    float shear = 0.0f;
    if (has_flag(pg.decoration, TextDecoration::Italic)) {
        shear = pg.italic_slant * m.height * scale;
    }

    // 頂点カラー
    Color c = pg.color;
    // RichText以外は TextStyle の face_color を使用
    if (c.r == 255 && c.g == 255 && c.b == 255 && c.a == 255) {
        c = style.face_color;
    }

    // UV座標
    float u0 = g.atlas.u0;
    float v0 = g.atlas.v0;
    float u1 = g.atlas.u1;
    float v1 = g.atlas.v1;

    // 頂点インデックスのベース
    uint32_t base = static_cast<uint32_t>(batch.vertices.size());

    // 4頂点 (左上、右上、右下、左下)
    batch.vertices.push_back({x0 + shear, y0, z, u0, v0, c.r, c.g, c.b, c.a}); // 左上
    batch.vertices.push_back({x1 + shear, y0, z, u1, v0, c.r, c.g, c.b, c.a}); // 右上
    batch.vertices.push_back({x1,         y1, z, u1, v1, c.r, c.g, c.b, c.a}); // 右下
    batch.vertices.push_back({x0,         y1, z, u0, v1, c.r, c.g, c.b, c.a}); // 左下

    // 6インデックス (2三角形)
    batch.indices.push_back(base + 0);
    batch.indices.push_back(base + 1);
    batch.indices.push_back(base + 2);
    batch.indices.push_back(base + 0);
    batch.indices.push_back(base + 2);
    batch.indices.push_back(base + 3);
}

// ---------------------------------------------------------------
// バッチ構築
// ---------------------------------------------------------------

std::vector<TextDrawBatch> TextRenderer::build_batches(
    const TextLayoutResult& layout,
    const FontAsset& font,
    const TextStyle& style,
    Vec3f origin)
{
    // アトラスページごとにバッチを分割
    std::vector<TextDrawBatch> batches;

    for (const auto& pg : layout.glyphs) {
        if (!pg.glyph) continue;
        if (pg.codepoint == ' ' || pg.codepoint == '\t') continue;

        uint32_t page = pg.glyph->atlas.atlas_index;

        // 対応するバッチを検索 (または作成)
        TextDrawBatch* target = nullptr;
        for (auto& b : batches) {
            if (b.atlas_page == page && b.font.id == font.id) {
                target = &b;
                break;
            }
        }
        if (!target) {
            batches.push_back({});
            target = &batches.back();
            target->atlas_page = page;
            target->font = FontHandle{font.id};
        }

        emit_glyph_quad(*target, pg, font, style, origin);
    }

    // 下線・取り消し線の生成
    for (const auto& line : layout.lines) {
        for (uint32_t i = 0; i < line.glyph_count; ++i) {
            const auto& pg = layout.glyphs[line.first_glyph + i];

            // 下線
            if (has_flag(pg.decoration, TextDecoration::Underline)) {
                // 下線の区間を検出して直線として描画
                // (バッチに薄い矩形を追加)
                float underline_y = origin.y + line.baseline_y - font.face.underline_offset * pg.scale;
                float thickness = font.face.underline_thickness * pg.scale;
                if (thickness < 1.0f) thickness = 1.0f;

                float x0 = origin.x + pg.position.x;
                float advance = pg.glyph ? pg.glyph->metrics.advance * pg.scale : 0.0f;
                float x1 = x0 + advance;

                // 最初のバッチに追加 (アトラスに依存しない矩形)
                if (!batches.empty()) {
                    auto& b = batches[0];
                    uint32_t base = static_cast<uint32_t>(b.vertices.size());
                    Color c = pg.color;

                    // 白テクセル (UV=0,0) で矩形を描画
                    b.vertices.push_back({x0, underline_y, origin.z, 0, 0, c.r, c.g, c.b, c.a});
                    b.vertices.push_back({x1, underline_y, origin.z, 0, 0, c.r, c.g, c.b, c.a});
                    b.vertices.push_back({x1, underline_y + thickness, origin.z, 0, 0, c.r, c.g, c.b, c.a});
                    b.vertices.push_back({x0, underline_y + thickness, origin.z, 0, 0, c.r, c.g, c.b, c.a});

                    b.indices.push_back(base + 0);
                    b.indices.push_back(base + 1);
                    b.indices.push_back(base + 2);
                    b.indices.push_back(base + 0);
                    b.indices.push_back(base + 2);
                    b.indices.push_back(base + 3);
                }
            }

            // 取り消し線 (同様のロジック)
            if (has_flag(pg.decoration, TextDecoration::Strikethrough)) {
                float strike_y = origin.y + line.baseline_y - font.face.strikethrough_offset * pg.scale;
                float thickness = font.face.strikethrough_thickness * pg.scale;
                if (thickness < 1.0f) thickness = 1.0f;

                float x0 = origin.x + pg.position.x;
                float advance = pg.glyph ? pg.glyph->metrics.advance * pg.scale : 0.0f;
                float x1 = x0 + advance;

                if (!batches.empty()) {
                    auto& b = batches[0];
                    uint32_t base = static_cast<uint32_t>(b.vertices.size());
                    Color c = pg.color;

                    b.vertices.push_back({x0, strike_y, origin.z, 0, 0, c.r, c.g, c.b, c.a});
                    b.vertices.push_back({x1, strike_y, origin.z, 0, 0, c.r, c.g, c.b, c.a});
                    b.vertices.push_back({x1, strike_y + thickness, origin.z, 0, 0, c.r, c.g, c.b, c.a});
                    b.vertices.push_back({x0, strike_y + thickness, origin.z, 0, 0, c.r, c.g, c.b, c.a});

                    b.indices.push_back(base + 0);
                    b.indices.push_back(base + 1);
                    b.indices.push_back(base + 2);
                    b.indices.push_back(base + 0);
                    b.indices.push_back(base + 2);
                    b.indices.push_back(base + 3);
                }
            }
        }
    }

    return batches;
}

// ---------------------------------------------------------------
// ヘルパー
// ---------------------------------------------------------------

std::vector<TextDrawBatch> TextRenderer::build_simple(
    const SimpleText& text,
    const FontAsset& font)
{
    Vec3f origin{text.position.x, text.position.y, 0.0f};
    return build_batches(text.layout_result, font, text.style, origin);
}

std::vector<TextDrawBatch> TextRenderer::build_rich(
    const RichText& text,
    const FontAsset& font)
{
    Vec3f origin{text.position.x, text.position.y, 0.0f};
    return build_batches(text.layout_result, font, text.base_style, origin);
}
