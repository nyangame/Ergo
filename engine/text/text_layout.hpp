#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include "../math/vec2.hpp"
#include "../math/color.hpp"
#include "glyph.hpp"
#include "font_asset.hpp"
#include "text_style.hpp"

// テキスト水平揃え
enum class TextAlign : uint32_t {
    Left,
    Center,
    Right
};

// テキスト垂直揃え
enum class TextVerticalAlign : uint32_t {
    Top,
    Middle,
    Bottom
};

// テキスト方向
enum class TextDirection : uint32_t {
    LeftToRight,     // LTR (欧文、日本語横書き)
    RightToLeft,     // RTL (アラビア語、ヘブライ語)
    TopToBottom      // 縦書き (日本語)
};

// オーバーフロー処理
enum class TextOverflow : uint32_t {
    Overflow,        // はみ出す (デフォルト)
    Truncate,        // 切り捨て
    Ellipsis,        // 省略記号 ("...")
    WordWrap         // ワードラップ
};

// テキストレイアウト設定
struct TextLayoutConfig {
    TextAlign align = TextAlign::Left;
    TextVerticalAlign vertical_align = TextVerticalAlign::Top;
    TextDirection direction = TextDirection::LeftToRight;
    TextOverflow overflow = TextOverflow::WordWrap;
    float font_size = 16.0f;            // 描画サイズ (ピクセル)
    float line_spacing = 1.2f;          // 行間係数 (1.0 = フォント行高と同じ)
    float letter_spacing = 0.0f;        // 文字間の追加スペース
    float word_spacing = 0.0f;          // 単語間の追加スペース
    float max_width = 0.0f;             // 最大幅 (0 = 無制限)
    float max_height = 0.0f;            // 最大高さ (0 = 無制限)
    float tab_width = 4.0f;             // タブ幅 (スペース文字数)
};

// レイアウト済みの1グリフ分の配置情報
struct PlacedGlyph {
    uint32_t codepoint = 0;
    uint32_t source_index = 0;          // 元テキスト中のバイトオフセット
    const Glyph* glyph = nullptr;       // フォントアセットのグリフへの参照
    FontHandle source_font{0};          // グリフの提供元フォント
    Vec2f position;                     // 配置位置 (ベースライン左端)
    float scale = 1.0f;                 // font_size / base_size
    Color color{255, 255, 255, 255};    // 個別色 (RichText用)
    TextDecoration decoration = TextDecoration::None;
    float italic_slant = 0.0f;
};

// レイアウト済みの1行分の情報
struct TextLine {
    uint32_t first_glyph = 0;           // glyphs配列内の開始インデックス
    uint32_t glyph_count = 0;           // この行のグリフ数
    float width = 0.0f;                 // 行の実幅
    float ascent = 0.0f;                // この行のアセンダ
    float descent = 0.0f;               // この行のディセンダ
    float baseline_y = 0.0f;            // この行のベースラインY座標
};

// テキストレイアウト結果: レイアウト計算の出力
struct TextLayoutResult {
    std::vector<PlacedGlyph> glyphs;
    std::vector<TextLine> lines;
    float total_width = 0.0f;           // 全行の最大幅
    float total_height = 0.0f;          // 全行の合計高さ
    bool truncated = false;             // テキストが切り捨てられたか
};

// テキストレイアウトエンジン
// Siv3D のテキスト配置ロジック / TextMeshPro の TMP_Text.GenerateTextMesh に相当
//
// フォントアセットのメトリクスを使ってテキストを行分割し、
// 各グリフの画面上の位置を決定する
class TextLayoutEngine {
public:
    // 単純テキストのレイアウト
    static TextLayoutResult layout(
        std::string_view text,
        const FontAsset& font,
        const TextLayoutConfig& config
    );

    // リッチテキスト用: スタイル付きセグメント単位のレイアウト
    struct StyledSegment {
        std::string_view text;
        FontHandle font{0};
        float font_size = 16.0f;
        Color color{255, 255, 255, 255};
        TextDecoration decoration = TextDecoration::None;
        float italic_slant = 0.0f;
    };

    static TextLayoutResult layout_rich(
        const std::vector<StyledSegment>& segments,
        const FontAsset& default_font,
        const TextLayoutConfig& config
    );

    // テキストの描画領域を計測 (レイアウト結果なし)
    static Vec2f measure(
        std::string_view text,
        const FontAsset& font,
        const TextLayoutConfig& config
    );

private:
    // UTF-8からコードポイントをデコード
    static uint32_t decode_utf8(const char*& ptr, const char* end);

    // 改行判定: ワードラップ時の分割位置を決定
    static bool is_breakable(uint32_t codepoint);

    // CJK文字判定 (日本語/中国語/韓国語は文字単位で改行可能)
    static bool is_cjk(uint32_t codepoint);

    // ホワイトスペース判定
    static bool is_whitespace(uint32_t codepoint);

    // 行揃え適用
    static void apply_alignment(
        TextLayoutResult& result,
        const TextLayoutConfig& config
    );
};
