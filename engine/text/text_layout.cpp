#include "text_layout.hpp"
#include <cmath>
#include <algorithm>

// ---------------------------------------------------------------
// UTF-8 デコード
// ---------------------------------------------------------------

uint32_t TextLayoutEngine::decode_utf8(const char*& ptr, const char* end) {
    if (ptr >= end) return 0;

    uint8_t c = static_cast<uint8_t>(*ptr);

    // ASCII
    if (c < 0x80) {
        ++ptr;
        return c;
    }
    // 2バイト: 110xxxxx
    if ((c & 0xE0) == 0xC0) {
        if (ptr + 1 >= end) { ++ptr; return 0xFFFD; }
        uint32_t cp = (c & 0x1F) << 6;
        cp |= (static_cast<uint8_t>(ptr[1]) & 0x3F);
        ptr += 2;
        return cp;
    }
    // 3バイト: 1110xxxx
    if ((c & 0xF0) == 0xE0) {
        if (ptr + 2 >= end) { ptr = end; return 0xFFFD; }
        uint32_t cp = (c & 0x0F) << 12;
        cp |= (static_cast<uint8_t>(ptr[1]) & 0x3F) << 6;
        cp |= (static_cast<uint8_t>(ptr[2]) & 0x3F);
        ptr += 3;
        return cp;
    }
    // 4バイト: 11110xxx
    if ((c & 0xF8) == 0xF0) {
        if (ptr + 3 >= end) { ptr = end; return 0xFFFD; }
        uint32_t cp = (c & 0x07) << 18;
        cp |= (static_cast<uint8_t>(ptr[1]) & 0x3F) << 12;
        cp |= (static_cast<uint8_t>(ptr[2]) & 0x3F) << 6;
        cp |= (static_cast<uint8_t>(ptr[3]) & 0x3F);
        ptr += 4;
        return cp;
    }

    ++ptr;
    return 0xFFFD; // 不正なバイト列 → 置換文字
}

// ---------------------------------------------------------------
// 文字分類ヘルパー
// ---------------------------------------------------------------

bool TextLayoutEngine::is_whitespace(uint32_t cp) {
    return cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r'
        || cp == 0x3000;  // 全角スペース
}

bool TextLayoutEngine::is_cjk(uint32_t cp) {
    // CJK統合漢字
    if (cp >= 0x4E00 && cp <= 0x9FFF) return true;
    // CJK統合漢字拡張A
    if (cp >= 0x3400 && cp <= 0x4DBF) return true;
    // CJK統合漢字拡張B
    if (cp >= 0x20000 && cp <= 0x2A6DF) return true;
    // ひらがな
    if (cp >= 0x3040 && cp <= 0x309F) return true;
    // カタカナ
    if (cp >= 0x30A0 && cp <= 0x30FF) return true;
    // CJK記号・句読点
    if (cp >= 0x3000 && cp <= 0x303F) return true;
    // 全角英数・記号
    if (cp >= 0xFF01 && cp <= 0xFF60) return true;
    // 半角カタカナ
    if (cp >= 0xFF65 && cp <= 0xFF9F) return true;
    return false;
}

bool TextLayoutEngine::is_breakable(uint32_t cp) {
    // スペース・タブの後で改行可能
    if (cp == ' ' || cp == '\t' || cp == 0x3000) return true;
    // CJK文字は文字境界で改行可能
    if (is_cjk(cp)) return true;
    // ハイフン後
    if (cp == '-' || cp == 0x2010 || cp == 0x2013 || cp == 0x2014) return true;
    return false;
}

// ---------------------------------------------------------------
// 単純テキストレイアウト
// ---------------------------------------------------------------

TextLayoutResult TextLayoutEngine::layout(
    std::string_view text,
    const FontAsset& font,
    const TextLayoutConfig& config)
{
    TextLayoutResult result;
    if (text.empty()) return result;

    const float scale = config.font_size / font.base_size;
    const float line_height = font.face.line_height * scale * config.line_spacing;
    const float ascender = font.face.ascender * scale;
    const float descender = font.face.descender * scale;
    const float space_advance = config.font_size * 0.25f; // スペース幅のフォールバック

    float cursor_x = 0.0f;
    float cursor_y = ascender; // 最初の行のベースライン

    TextLine current_line;
    current_line.first_glyph = 0;
    current_line.ascent = ascender;
    current_line.descent = descender;
    current_line.baseline_y = cursor_y;

    // ワードラップ用: 最後の分割可能位置
    uint32_t last_break_glyph = 0;
    float last_break_x = 0.0f;
    bool has_break_point = false;

    uint32_t prev_codepoint = 0;
    const char* ptr = text.data();
    const char* end = text.data() + text.size();
    uint32_t byte_offset = 0;

    while (ptr < end) {
        const char* char_start = ptr;
        uint32_t cp = decode_utf8(ptr, end);
        uint32_t current_offset = static_cast<uint32_t>(char_start - text.data());

        // 改行文字
        if (cp == '\n') {
            current_line.width = cursor_x;
            current_line.glyph_count = static_cast<uint32_t>(result.glyphs.size()) - current_line.first_glyph;
            result.lines.push_back(current_line);

            cursor_x = 0.0f;
            cursor_y += line_height;

            current_line = {};
            current_line.first_glyph = static_cast<uint32_t>(result.glyphs.size());
            current_line.ascent = ascender;
            current_line.descent = descender;
            current_line.baseline_y = cursor_y;

            has_break_point = false;
            prev_codepoint = 0;
            continue;
        }

        // キャリッジリターンをスキップ
        if (cp == '\r') continue;

        // タブ
        if (cp == '\t') {
            float tab_stop = space_advance * config.tab_width;
            cursor_x = std::ceil(cursor_x / tab_stop) * tab_stop;
            if (cursor_x < 0.001f) cursor_x = tab_stop;
            prev_codepoint = cp;
            continue;
        }

        // グリフ検索
        const Glyph* glyph = font.find_glyph(cp);
        float advance;
        if (glyph) {
            advance = glyph->metrics.advance * scale;
        } else {
            // グリフが見つからない場合はスペース幅で代用
            advance = space_advance;
        }

        // カーニング
        if (prev_codepoint != 0) {
            float kern = font.get_kerning(prev_codepoint, cp);
            cursor_x += kern * scale;
        }

        // 文字間スペース
        cursor_x += config.letter_spacing;

        // 単語間スペース
        if (cp == ' ' && config.word_spacing != 0.0f) {
            cursor_x += config.word_spacing;
        }

        // ワードラップ判定
        if (config.overflow == TextOverflow::WordWrap && config.max_width > 0.0f) {
            if (cursor_x + advance > config.max_width && current_line.glyph_count > 0) {
                if (has_break_point) {
                    // 最後の分割位置で改行
                    uint32_t break_count = last_break_glyph - current_line.first_glyph;
                    current_line.width = last_break_x;
                    current_line.glyph_count = break_count;
                    result.lines.push_back(current_line);

                    // 分割位置以降のグリフを次行に移動
                    cursor_y += line_height;
                    float offset_x = last_break_x;

                    current_line = {};
                    current_line.first_glyph = last_break_glyph;
                    current_line.ascent = ascender;
                    current_line.descent = descender;
                    current_line.baseline_y = cursor_y;

                    // 残りグリフの位置を再計算
                    for (uint32_t i = last_break_glyph; i < result.glyphs.size(); ++i) {
                        result.glyphs[i].position.x -= offset_x;
                    }
                    cursor_x -= offset_x;
                } else {
                    // 分割位置がない場合は強制改行
                    current_line.width = cursor_x;
                    current_line.glyph_count = static_cast<uint32_t>(result.glyphs.size()) - current_line.first_glyph;
                    result.lines.push_back(current_line);

                    cursor_x = 0.0f;
                    cursor_y += line_height;

                    current_line = {};
                    current_line.first_glyph = static_cast<uint32_t>(result.glyphs.size());
                    current_line.ascent = ascender;
                    current_line.descent = descender;
                    current_line.baseline_y = cursor_y;
                }
                has_break_point = false;
            }
        }

        // 最大高さチェック
        if (config.max_height > 0.0f && cursor_y + std::abs(descender) > config.max_height) {
            if (config.overflow == TextOverflow::Truncate || config.overflow == TextOverflow::Ellipsis) {
                result.truncated = true;
                break;
            }
        }

        // 分割可能位置の記録
        if (is_breakable(cp)) {
            last_break_glyph = static_cast<uint32_t>(result.glyphs.size()) + 1;
            last_break_x = cursor_x + advance;
            has_break_point = true;
        }

        // グリフ配置
        PlacedGlyph pg;
        pg.codepoint = cp;
        pg.source_index = current_offset;
        pg.glyph = glyph;
        pg.source_font = FontHandle{font.id};
        pg.position = Vec2f{cursor_x, cursor_y};
        pg.scale = scale;
        result.glyphs.push_back(pg);

        current_line.glyph_count = static_cast<uint32_t>(result.glyphs.size()) - current_line.first_glyph;

        cursor_x += advance;
        prev_codepoint = cp;
    }

    // 最後の行を追加
    if (current_line.glyph_count > 0 || result.lines.empty()) {
        current_line.width = cursor_x;
        current_line.glyph_count = static_cast<uint32_t>(result.glyphs.size()) - current_line.first_glyph;
        result.lines.push_back(current_line);
    }

    // 省略記号の追加
    if (result.truncated && config.overflow == TextOverflow::Ellipsis) {
        const Glyph* dot_glyph = font.find_glyph(U'.');
        if (dot_glyph && !result.glyphs.empty()) {
            // 最後の3文字を省略記号に置換
            uint32_t remove_count = std::min<uint32_t>(3, static_cast<uint32_t>(result.glyphs.size()));
            for (uint32_t i = 0; i < remove_count; ++i) {
                if (!result.glyphs.empty()) result.glyphs.pop_back();
            }
            float x = result.glyphs.empty() ? 0.0f : result.glyphs.back().position.x
                       + (result.glyphs.back().glyph ? result.glyphs.back().glyph->metrics.advance * scale : space_advance);
            for (int i = 0; i < 3; ++i) {
                PlacedGlyph pg;
                pg.codepoint = U'.';
                pg.glyph = dot_glyph;
                pg.source_font = FontHandle{font.id};
                pg.position = Vec2f{x, result.lines.back().baseline_y};
                pg.scale = scale;
                result.glyphs.push_back(pg);
                x += dot_glyph->metrics.advance * scale;
            }
            // 行情報の更新
            if (!result.lines.empty()) {
                auto& last = result.lines.back();
                last.glyph_count = static_cast<uint32_t>(result.glyphs.size()) - last.first_glyph;
                last.width = x;
            }
        }
    }

    // 行揃え適用
    apply_alignment(result, config);

    // トータルサイズ算出
    result.total_width = 0.0f;
    for (const auto& line : result.lines) {
        result.total_width = std::max(result.total_width, line.width);
    }
    if (!result.lines.empty()) {
        const auto& last = result.lines.back();
        result.total_height = last.baseline_y - last.descent;
    }

    return result;
}

// ---------------------------------------------------------------
// リッチテキストレイアウト
// ---------------------------------------------------------------

TextLayoutResult TextLayoutEngine::layout_rich(
    const std::vector<StyledSegment>& segments,
    const FontAsset& default_font,
    const TextLayoutConfig& config)
{
    TextLayoutResult result;
    if (segments.empty()) return result;

    const float default_scale = config.font_size / default_font.base_size;
    const float line_height = default_font.face.line_height * default_scale * config.line_spacing;
    const float ascender = default_font.face.ascender * default_scale;
    const float descender = default_font.face.descender * default_scale;
    const float space_advance = config.font_size * 0.25f;

    float cursor_x = 0.0f;
    float cursor_y = ascender;

    TextLine current_line;
    current_line.first_glyph = 0;
    current_line.ascent = ascender;
    current_line.descent = descender;
    current_line.baseline_y = cursor_y;

    uint32_t prev_codepoint = 0;

    for (const auto& seg : segments) {
        const float seg_scale = seg.font_size / default_font.base_size;
        const char* ptr = seg.text.data();
        const char* end = seg.text.data() + seg.text.size();

        while (ptr < end) {
            const char* char_start = ptr;
            uint32_t cp = decode_utf8(ptr, end);

            // 改行
            if (cp == '\n') {
                current_line.width = cursor_x;
                current_line.glyph_count = static_cast<uint32_t>(result.glyphs.size()) - current_line.first_glyph;
                result.lines.push_back(current_line);

                cursor_x = 0.0f;
                cursor_y += line_height;

                current_line = {};
                current_line.first_glyph = static_cast<uint32_t>(result.glyphs.size());
                current_line.ascent = ascender;
                current_line.descent = descender;
                current_line.baseline_y = cursor_y;

                prev_codepoint = 0;
                continue;
            }

            if (cp == '\r') continue;

            // タブ
            if (cp == '\t') {
                float tab_stop = space_advance * config.tab_width;
                cursor_x = std::ceil(cursor_x / tab_stop) * tab_stop;
                if (cursor_x < 0.001f) cursor_x = tab_stop;
                prev_codepoint = cp;
                continue;
            }

            // グリフ検索
            const Glyph* glyph = default_font.find_glyph(cp);
            float advance = glyph ? glyph->metrics.advance * seg_scale : space_advance;

            // カーニング
            if (prev_codepoint != 0) {
                cursor_x += default_font.get_kerning(prev_codepoint, cp) * seg_scale;
            }
            cursor_x += config.letter_spacing;

            // ワードラップ (簡易版)
            if (config.overflow == TextOverflow::WordWrap && config.max_width > 0.0f) {
                if (cursor_x + advance > config.max_width && current_line.glyph_count > 0) {
                    current_line.width = cursor_x;
                    current_line.glyph_count = static_cast<uint32_t>(result.glyphs.size()) - current_line.first_glyph;
                    result.lines.push_back(current_line);

                    cursor_x = 0.0f;
                    cursor_y += line_height;

                    current_line = {};
                    current_line.first_glyph = static_cast<uint32_t>(result.glyphs.size());
                    current_line.ascent = ascender;
                    current_line.descent = descender;
                    current_line.baseline_y = cursor_y;
                }
            }

            // グリフ配置 (セグメントのスタイル情報を付与)
            PlacedGlyph pg;
            pg.codepoint = cp;
            pg.source_index = static_cast<uint32_t>(char_start - seg.text.data());
            pg.glyph = glyph;
            pg.source_font = seg.font;
            pg.position = Vec2f{cursor_x, cursor_y};
            pg.scale = seg_scale;
            pg.color = seg.color;
            pg.decoration = seg.decoration;
            pg.italic_slant = seg.italic_slant;
            result.glyphs.push_back(pg);

            current_line.glyph_count = static_cast<uint32_t>(result.glyphs.size()) - current_line.first_glyph;
            cursor_x += advance;
            prev_codepoint = cp;
        }
    }

    // 最後の行
    if (current_line.glyph_count > 0 || result.lines.empty()) {
        current_line.width = cursor_x;
        current_line.glyph_count = static_cast<uint32_t>(result.glyphs.size()) - current_line.first_glyph;
        result.lines.push_back(current_line);
    }

    apply_alignment(result, config);

    result.total_width = 0.0f;
    for (const auto& line : result.lines) {
        result.total_width = std::max(result.total_width, line.width);
    }
    if (!result.lines.empty()) {
        const auto& last = result.lines.back();
        result.total_height = last.baseline_y - last.descent;
    }

    return result;
}

// ---------------------------------------------------------------
// テキスト計測
// ---------------------------------------------------------------

Vec2f TextLayoutEngine::measure(
    std::string_view text,
    const FontAsset& font,
    const TextLayoutConfig& config)
{
    auto result = layout(text, font, config);
    return Vec2f{result.total_width, result.total_height};
}

// ---------------------------------------------------------------
// 行揃え適用
// ---------------------------------------------------------------

void TextLayoutEngine::apply_alignment(
    TextLayoutResult& result,
    const TextLayoutConfig& config)
{
    if (config.align == TextAlign::Left && config.vertical_align == TextVerticalAlign::Top) {
        return; // デフォルトは左上揃え、調整不要
    }

    // 最大幅を算出 (max_width が指定されている場合はそれを使用)
    float container_width = config.max_width;
    if (container_width <= 0.0f) {
        for (const auto& line : result.lines) {
            container_width = std::max(container_width, line.width);
        }
    }

    // 水平揃え
    if (config.align != TextAlign::Left && container_width > 0.0f) {
        for (const auto& line : result.lines) {
            float offset = 0.0f;
            if (config.align == TextAlign::Center) {
                offset = (container_width - line.width) * 0.5f;
            } else if (config.align == TextAlign::Right) {
                offset = container_width - line.width;
            }

            if (offset > 0.0f) {
                for (uint32_t i = 0; i < line.glyph_count; ++i) {
                    result.glyphs[line.first_glyph + i].position.x += offset;
                }
            }
        }
    }

    // 垂直揃え
    if (config.vertical_align != TextVerticalAlign::Top && config.max_height > 0.0f) {
        float offset = 0.0f;
        if (config.vertical_align == TextVerticalAlign::Middle) {
            offset = (config.max_height - result.total_height) * 0.5f;
        } else if (config.vertical_align == TextVerticalAlign::Bottom) {
            offset = config.max_height - result.total_height;
        }

        if (offset > 0.0f) {
            for (auto& pg : result.glyphs) {
                pg.position.y += offset;
            }
            for (auto& line : result.lines) {
                line.baseline_y += offset;
            }
        }
    }
}
