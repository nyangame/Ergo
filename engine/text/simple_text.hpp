#pragma once
#include <string>
#include <string_view>
#include "../math/vec2.hpp"
#include "../math/color.hpp"
#include "font_asset.hpp"
#include "text_style.hpp"
#include "text_layout.hpp"

// SimpleText: 単一スタイルのテキスト描画コンポーネント
// Siv3D の Font::operator() / TextMeshPro の TextMeshPro (非リッチ) に相当
//
// 特徴:
// - テキスト全体に1つのスタイルを適用
// - レイアウト結果をキャッシュし、テキストや設定変更時のみ再計算
// - RenderContext への描画メソッドを提供
//
// 使い方:
//   SimpleText text;
//   text.set_font(font_handle);
//   text.set_text("Hello, World!");
//   text.set_position({100.0f, 200.0f});
//   text.draw(ctx);
//
struct SimpleText {
    // 設定
    FontHandle font{0};
    TextMaterialHandle material{0};  // 0 = デフォルトスタイル使用
    std::string text;
    Vec2f position;
    TextLayoutConfig layout_config;
    TextStyle style;

    // キャッシュ
    TextLayoutResult layout_result;
    bool dirty = true;               // レイアウト再計算が必要か

    // テキストの設定
    void set_text(std::string_view new_text) {
        if (text != new_text) {
            text = std::string(new_text);
            dirty = true;
        }
    }

    void set_font(FontHandle new_font) {
        if (font.id != new_font.id) {
            font = new_font;
            dirty = true;
        }
    }

    void set_font_size(float size) {
        if (layout_config.font_size != size) {
            layout_config.font_size = size;
            dirty = true;
        }
    }

    void set_position(Vec2f pos) {
        position = pos;
    }

    void set_color(Color color) {
        style.face_color = color;
    }

    void set_align(TextAlign align) {
        if (layout_config.align != align) {
            layout_config.align = align;
            dirty = true;
        }
    }

    void set_max_width(float width) {
        if (layout_config.max_width != width) {
            layout_config.max_width = width;
            dirty = true;
        }
    }

    void set_line_spacing(float spacing) {
        if (layout_config.line_spacing != spacing) {
            layout_config.line_spacing = spacing;
            dirty = true;
        }
    }

    void set_overflow(TextOverflow overflow) {
        if (layout_config.overflow != overflow) {
            layout_config.overflow = overflow;
            dirty = true;
        }
    }

    // レイアウトの再計算 (FontAssetへの参照が必要)
    void update_layout(const FontAsset& font_asset) {
        if (!dirty) return;
        layout_result = TextLayoutEngine::layout(text, font_asset, layout_config);
        dirty = false;
    }

    // 描画領域の取得
    Vec2f measure(const FontAsset& font_asset) {
        update_layout(font_asset);
        return Vec2f{layout_result.total_width, layout_result.total_height};
    }

    // 行数の取得
    uint32_t line_count() const {
        return static_cast<uint32_t>(layout_result.lines.size());
    }

    // レイアウト結果へのアクセス
    const TextLayoutResult& get_layout() const { return layout_result; }
};
