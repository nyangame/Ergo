#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "../math/vec2.hpp"
#include "../math/color.hpp"
#include "font_asset.hpp"
#include "text_style.hpp"
#include "text_layout.hpp"

// リッチテキストタグの種類
enum class RichTagType : uint32_t {
    Color,           // <color=#RRGGBB> or <color=#RRGGBBAA>
    Size,            // <size=24>
    Bold,            // <b>
    Italic,          // <i>
    Underline,       // <u>
    Strikethrough,   // <s>
    Font,            // <font="FontName">
    Outline,         // <outline=#RRGGBB width=0.1>
    Superscript,     // <sup>
    Subscript,       // <sub>
};

// パース済みリッチテキストセグメント
struct RichTextSegment {
    std::string text;
    Color color{255, 255, 255, 255};
    float font_size = 0.0f;             // 0 = デフォルト使用
    FontHandle font{0};                 // 0 = デフォルト使用
    TextDecoration decoration = TextDecoration::None;
    float italic_slant = 0.2f;
};

// RichText: タグ付きマークアップによる複数スタイルテキスト描画コンポーネント
// Siv3D のリッチテキスト / TextMeshPro の <tag> ベースリッチテキストに相当
//
// サポートするタグ:
//   <color=#FF0000>赤いテキスト</color>
//   <size=32>大きいテキスト</size>
//   <b>太字</b>
//   <i>斜体</i>
//   <u>下線</u>
//   <s>取り消し線</s>
//   <font="NotoSans">フォント変更</font>
//   <outline=#000000 width=0.1>アウトライン付き</outline>
//   <sup>上付き</sup>
//   <sub>下付き</sub>
//
// ネスト対応: <color=#FF0000><b>赤太字</b></color>
//
struct RichText {
    // 設定
    FontHandle default_font{0};
    TextMaterialHandle material{0};
    std::string source_text;             // タグ付き元テキスト
    Vec2f position;
    TextLayoutConfig layout_config;
    TextStyle base_style;

    // パース結果
    std::vector<RichTextSegment> segments;
    TextLayoutResult layout_result;
    bool dirty = true;

    // テキストの設定 (タグ付きテキスト)
    void set_text(std::string_view markup) {
        if (source_text != markup) {
            source_text = std::string(markup);
            dirty = true;
        }
    }

    void set_font(FontHandle font) {
        if (default_font.id != font.id) {
            default_font = font;
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
        base_style.face_color = color;
        dirty = true;
    }

    void set_max_width(float width) {
        if (layout_config.max_width != width) {
            layout_config.max_width = width;
            dirty = true;
        }
    }

    // パースとレイアウトの再計算
    void update_layout(const FontAsset& font_asset) {
        if (!dirty) return;
        segments = parse_markup(source_text, base_style.face_color, layout_config.font_size);
        layout_result = build_layout(font_asset);
        dirty = false;
    }

    // 描画領域の取得
    Vec2f measure(const FontAsset& font_asset) {
        update_layout(font_asset);
        return Vec2f{layout_result.total_width, layout_result.total_height};
    }

    uint32_t line_count() const {
        return static_cast<uint32_t>(layout_result.lines.size());
    }

    const TextLayoutResult& get_layout() const { return layout_result; }

    // --- マークアップパーサー ---

    // タグ付きテキストをセグメントに分解
    static std::vector<RichTextSegment> parse_markup(
        std::string_view markup,
        Color default_color,
        float default_size);

private:
    TextLayoutResult build_layout(const FontAsset& font_asset) const;

    // パーサー内部: 16進カラーコードをパース
    static Color parse_hex_color(std::string_view hex);

    // パーサー内部: 数値をパース
    static float parse_float(std::string_view str);
};
